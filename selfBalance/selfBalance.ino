#include <PID_v1.h> //github.com/mwoodward/Arduino-PID-Library
#include "I2Cdev.h"
#include "MPU6050.h"
#include <Kalman.h>

#include "Wire.h" // for i2c
#include <TimedAction.h> // for updating sensors and debug
#include <Button.h>        //github.com/JChristensen/Button
#include <EEPROM.h> // for storing configuraion
#include <FIR.h>  //github.com/sebnil/FIR-filter-Arduino-Library
#include <MovingAvarageFilter.h> //github.com/sebnil/Moving-Avarage-Filter--Arduino-Library-
Kalman kalman;
boolean debug = true;

#define motor1ForwardDir HIGH
#define motor1BackwardDir LOW
#define motor2ForwardDir HIGH
#define motor2BackwardDir LOW
#define maxSpeed 50
#define speedMultiplier 1

volatile long leftMotorPosition  = 0;
volatile long rightMotorPosition = 0;
long lastLeftMotorPosition  = 0;
long lastRightMotorPosition  = 0;
int leftMotorSpeedEncoder;
int rightMotorSpeedEncoder;

//int leftMotorSpeed;
//int rightMotorSpeed;

int loopCounter = 0;
int lastSpeedUpdate = 0;
int lastDebugEncoders = 0;

// configure struct that is stored in eeprom and updated from the lcd console.
struct Configure {
  word speedPIDKp;
  word speedPIDKi;
  word speedPIDKd;
  word speedPIDOutputLowerLimit;
  word speedPIDOutputHigherLimit;
  word anglePIDAggKp;
  word anglePIDAggKi;
  word anglePIDAggKd;
  word anglePIDConKp;
  word anglePIDConKi;
  word anglePIDConKd;
  word anglePIDLowerLimit;
  uint8_t anglePIDSampling;
  uint8_t speedPIDSampling;
  uint8_t angleSensorSampling;
  uint8_t motorSpeedSensorSampling;
  uint8_t v1;
  uint8_t v2;  
  uint8_t v3;
  uint8_t v4;
} configuration = {
  // The default values
 150, 500, 1, 50, 50, 8000, 2500, 350, 5, 5, 5000, 0, 10, 10, 10, 10
};

boolean started = false; // if the robot is started or not

// these take care of the timing of things
TimedAction debugTimedAction = TimedAction(1000,debugEverything);
TimedAction updateMotorStatusesTimedAction = TimedAction(20, updateMotorStatuses);
TimedAction debugEncodersTimedAction = TimedAction(100,debugEncoders);
TimedAction updateMotorSpeedsTimedAction = TimedAction(20,updateMotorSpeeds);
TimedAction updateIMUSensorsTimedAction = TimedAction(20, updateIMUSensors);

// button declarations
Button startBtn(8, false, false, 20);
Button stopBtn(9, false, false, 20);
Button calibrateBtn(12, false, false, 20);

// motor controller
#define pwm_a 5   //PWM control for motor outputs 1 and 2 is on digital pin 3
#define pwm_b 6  //PWM control for motor outputs 3 and 4 is on digital pin 11
#define dir_a 4  //direction control for motor outputs 1 and 2 is on digital pin 12
#define dir_b 7  //direction control for motor outputs 3 and 4 is on digital pin 13

/* Encoders */
#define leftEncoder1 2
#define leftEncoder2 3
#define rightEncoder1 19
#define rightEncoder2 18

// imu variables
MPU6050 accelgyro;
 
int16_t ax, ay, az;
int16_t gx, gy, gz;

// accelerometer values
int accel_reading;
int accel_corrected;
int accel_offset = 200;
double accel_angle;
float accel_scale = 1; // set to 0.01

// gyro values
int gyro_offset = 151; // 151
int gyro_corrected;
int gyro_reading;
float gyro_rate;
float gyro_scale = 0.02; // 0.02 by default - tweak as required
float gyro_angle;
float loop_time = 0.05; // 50ms loop
float angle = 0.00; // value to hold final calculated gyro angle

//new
double roll;
double accYangle;
double gyroYrate;
double gyroAngle;
/* Used for timing */
unsigned long timer;
#define STD_LOOP_TIME 10000 // Fixed time loop of 10 milliseconds
unsigned long lastLoopUsefulTime = STD_LOOP_TIME;
unsigned long loopStartTime;
double zeroValues[2] = { 0 };

// time stamp variables
int last_update;
int cycle_time;
long last_cycle = 0;

// motor speeds and calibrations
float motorSpeed;
float leftMotorSpeed;
float rightMotorSpeed;
float motor1Calibration = 1;
float motor2Calibration = 1.4;

// PID variables
double anglePIDSetpoint, anglePIDInput, anglePIDOutput;
double speedPIDInput, speedPIDOutput, speedPIDSetpoint;

// The cascading PIDs. The tunings are updated from the code
PID anglePID(&anglePIDInput, &anglePIDOutput, &anglePIDSetpoint, 0, 0, 0, REVERSE);
PID speedPID(&speedPIDInput, &speedPIDOutput, &speedPIDSetpoint, 2, 5, 1, DIRECT);

// filters

FIR speedFIR;
MovingAvarageFilter speedMovingAvarageFilter(40);
MovingAvarageFilter throttleControlAvarageFilter(40);

// Set the FreeSixIMU object. This handles the communcation to the IMU.


 //remote control
int8_t steering = 0; // goes from -128 to +127 but we will only use from -127 to +127 to get symmetry
float motor1SteeringOffset;
float motor2SteeringOffset;

void setup() { 
  Serial.begin(57600);
  saveConfig();
 /* Setup encoders */
  pinMode(leftEncoder1,INPUT);
  pinMode(leftEncoder2,INPUT);
  pinMode(rightEncoder1,INPUT);
  pinMode(rightEncoder2,INPUT); 
  attachInterrupt(1,leftEncoder,RISING); // pin 3
  attachInterrupt(5,rightEncoder,RISING); // pin 18
  
  //Set control pins to be outputs
  pinMode(pwm_a, OUTPUT);  
  pinMode(pwm_b, OUTPUT);
  pinMode(dir_a, OUTPUT);
  pinMode(dir_b, OUTPUT);

  digitalWrite(dir_a, LOW);
  digitalWrite(dir_b, HIGH);

  // stop the motors if they are running
  stopMotors();

  // load config from eeprom
  loadConfig();

  // init i2c and IMU
  Wire.begin();
  // initialize device
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();
   
  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");;
  loopStartTime = micros();
  timer = loopStartTime;
  calibrateSensors();
  
  
  // init PIDs
  initAnglePID();
  initSpeedPID();  
  //initMotorPIDs();

  // init the timers
  initTimedActions();

  
  float speedFIRcoef[FILTERTAPS] = {1.000, 10.0, 20.0, 10.0, 1.00};
  float gain = 0;
  for (int i=0; i<FILTERTAPS; i++) {
    gain += speedFIRcoef[i];
  }
  speedFIR.setCoefficients(speedFIRcoef);
  speedFIR.setGain(gain);
}

void initTimedActions() {
  updateMotorStatusesTimedAction.setInterval(configuration.motorSpeedSensorSampling);
  updateIMUSensorsTimedAction.setInterval(configuration.angleSensorSampling);
}

void initAnglePID() {
  anglePIDSetpoint = 0;
  anglePID.SetOutputLimits(-100, 100);
  anglePID.SetMode(AUTOMATIC);
  anglePID.SetSampleTime(configuration.anglePIDSampling);
}

void initSpeedPID() {
  speedPIDSetpoint = 0;
  speedPID.SetOutputLimits(-(float)configuration.speedPIDOutputLowerLimit/100, (float)configuration.speedPIDOutputLowerLimit/100);
  anglePID.SetMode(AUTOMATIC);
  speedPID.SetSampleTime(configuration.speedPIDSampling);
  speedPID.SetTunings((float)configuration.speedPIDKp / 100, (float)configuration.speedPIDKi / 100, (float)configuration.speedPIDKd / 100);
}


void time_stamp(){
  while ((millis() - last_cycle) < 50){
  delay(1);
  }
  // once loop cycle reaches 50ms, reset timer value and continue
  cycle_time = millis() - last_cycle;
  last_cycle = millis();
}
double getGyroYrate() {
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
   // gyro_Y_Axis angle calc  
  gyro_reading = gy;
  gyro_corrected = (float)((gyro_reading/131) - gyro_offset);  // 131 is sensivity of gyro from data sheet
  gyro_rate = (gyro_corrected * gyro_scale) * -loop_time;      // loop_time = 0.05 ie 50ms        
  gyro_angle = angle + gyro_rate;
  return gyro_rate;
  //return gyro_angle;
}
double getAccY() {
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    // accelerometer_X_Axis angle calc
  accel_reading = ax;
  accel_corrected = accel_reading - accel_offset;
  accel_corrected = map(accel_corrected, -16800, 16800, -90, 90);
  accel_corrected = constrain(accel_corrected, -90, 90);
  accel_angle = (float)(accel_corrected * accel_scale);
  return accel_angle;
}

void calibrateSensors() {
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  for (uint8_t i = 0; i < 100; i++) { // Take the average of 100 readings
    zeroValues[0] += ay;
    zeroValues[1] += ax;
    //zeroValues[2] += analogRead(accZ);
    delay(10);
  }
  zeroValues[0] /= 100; // Gyro X-axis
  zeroValues[1] /= 100; // Accelerometer Y-axis
  //zeroValues[2] /= 100; // Accelerometer Z-axis

  if(zeroValues[1] > 500) { // Check which side is lying down - 1g is equal to 0.33V or 102.3 quids (0.33/3.3*1023=102.3)
    zeroValues[1] -= 102.3; // +1g when lying at one of the sides
    kalman.setAngle(90); // It starts at 90 degress and 270 when facing the other way
    gyroAngle = 90;
  } else {
    zeroValues[1] += 102.3; // -1g when lying at the other side
    kalman.setAngle(270);
    gyroAngle = 270;
  }
}

void updateIMUSensors(){
  accYangle = getAccY();
  gyroYrate = getGyroYrate();
  gyroAngle += gyroYrate*((double)(micros()-timer)/1000000);
  // See my guide for more info about calculation the angles and the Kalman filter: http://arduino.cc/forum/index.php/topic,58048.0.htm
  roll = kalman.getAngle(accYangle, gyroYrate, (double)(micros() - timer)/1000000); // Calculate the angle using a Kalman filter
  timer = micros();
  time_stamp();
  anglePIDInput = roll + 0.5;
}
void getGyrodata(){
    // print values to serial monitor for checking 
//  Serial.print(accYangle);
//  Serial.print("\t");
  Serial.print(anglePIDInput);
  Serial.println("\t");
//  Serial.print(roll);
//  Serial.print("\t");
//  Serial.print(gyro_angle);
//  Serial.print("\t");
//  Serial.println(" ");
}
void loop() {   
  

  updateIMUSensorsTimedAction.check();
  debugEncodersTimedAction.check();
  updateMotorSpeedsTimedAction.check();

  
  debugEncoders();
  updateMotorSpeeds();
  leftMotorSpeed = leftMotorSpeedEncoder;
  rightMotorSpeed = rightMotorSpeedEncoder;
  
  updateMotorStatusesTimedAction.check();
  
  // update sensors and sometimes debug
  if (debug)
    debugTimedAction.check();

  if (started) {
    // speed pid. input is wheel speed. output is angleSetpoint
    speedPID.Compute();
    anglePIDSetpoint = speedPIDOutput;

    if (abs(rightMotorSpeed) < 5) {
      motor1Calibration = 1;
      motor2Calibration = 1.3;  
    }
    else if (abs(rightMotorSpeed < 10)) {
      motor1Calibration = 1;
      motor2Calibration = 1.2;
    }
    else if (abs(rightMotorSpeed < 15)) {
      motor1Calibration = 1;
      motor2Calibration = 1.1;
    }
    else {
      motor1Calibration = 1;
      motor2Calibration = 1;          
    }

    // update angle pid tuning
    if(abs(anglePIDInput) < (float)configuration.anglePIDLowerLimit / 100 && configuration.anglePIDLowerLimit != 0) { 
      //we're close to setpoint, use conservative tuning parameters
      anglePID.SetTunings((float)configuration.anglePIDConKp / 100, (float)configuration.anglePIDConKi / 100, (float)configuration.anglePIDConKd / 100);
    }
    else if (abs(anglePIDInput) < 45) {
      //we're far from setpoint, use aggressive tuning parameters
      anglePID.SetTunings((float)configuration.anglePIDAggKp / 100, (float)configuration.anglePIDAggKi / 100, (float)configuration.anglePIDAggKd / 100);
    }
    else {
      anglePID.SetTunings(0, 0, 0);
      stopMotors();
    }
    anglePID.Compute();


    // set motor pwm
    if (steering == 0) {
      motor1SteeringOffset = 0;
      motor2SteeringOffset = 0;
    }
    // left
    else if (steering < 0) {
      // motor1 should move faster. motor 2 slower.
      motor1SteeringOffset = (float)steering;
      motor2SteeringOffset = -(float)steering;
    }
    else {
      motor1SteeringOffset = (float)steering;
      motor2SteeringOffset = -(float)steering;
    }
    moveMotor(1, anglePIDOutput + motor1SteeringOffset);
    moveMotor(2, anglePIDOutput + motor2SteeringOffset);
    /*moveMotor(1, anglePIDOutput);
    moveMotor(2, anglePIDOutput);*/

  }
  else {
    moveMotor(1, 0);
    moveMotor(2, 0);      
  }


  // check buttons and do actions if released
  startBtn.read();
  stopBtn.read();
  calibrateBtn.read();
  if (startBtn.wasReleased() || stopBtn.wasReleased()) {
    //get_msg_from_console();
    saveConfig();
    debugConfiguration();

    //initMotorPIDs(); // pids on motors are too slow
    initAnglePID();
    initSpeedPID();
    initTimedActions();
  }
  if (startBtn.wasReleased()) {
    Serial.println("startBtn.wasReleased");
    anglePID.SetMode(AUTOMATIC);
    speedPID.SetMode(AUTOMATIC);
    started = true;
  }
  if (stopBtn.wasReleased()) {
    Serial.println("stopBtn.wasReleased");
    stopMotors();
    anglePID.SetMode(MANUAL);
    speedPID.SetMode(MANUAL);
    started = false;
  }
  if (started && calibrateBtn.wasPressed()) {
    Serial.println("calibrateBtn.wasPressed");
    speedPID.SetMode(MANUAL);
    speedPIDOutput = 0;
    anglePIDInput = 0;
  }
  else if (started && calibrateBtn.wasReleased()) {
    Serial.println("calibrateBtn.wasReleased");
    speedPID.SetMode(AUTOMATIC);    
  }
  

}

// go here if remote control connection is lost
void stopRobot() {
  //Serial1.println("stopRobot");
  speedPIDSetpoint = 0;
  steering = 0;
}

/* just debug functions. uncomment the debug information you want in debugEverything */
void debugEverything() {
  //debugImu();
  debugAnglePID();
  debugSpeedPID();  
  //debugMotorSpeeds();
  //debugMotorCalibrations();
  //debugMotorSpeedCalibration();
  //debugChart2();
  //Serial.println();
  //debugSensorValues();
  //getGyrodata();
  }


//void debugConfiguration() {
//
//}

void debugSensorValues() {
 Serial.print("\tfirroll: ");
 Serial.print(roll, 5);
 Serial.println();
 }
// 
 void debugAnglePID() {
 Serial.print("\tanglePID I: ");
 Serial.print(anglePIDInput, 4);
 Serial.print("\tO: ");
 Serial.print(anglePIDOutput, 4);
 Serial.print("\tS: ");
 Serial.println(anglePIDSetpoint, 4);
 }
 
 void debugSpeedPID() {
 Serial.print("\tspeedPID I: ");
 Serial.print(speedPIDInput, 4);
 Serial.print("\tO: ");
 Serial.print(speedPIDOutput, 4);
 Serial.print("\tS: ");
 Serial.println(speedPIDSetpoint, 4);
 }
 /*
 void debugMotorCalibrations() {
 Serial.print("\tm1 C: ");
 printFloat(motor1Calibration, 4);
 Serial.print("\tm2 C: ");
 printFloat(motor2Calibration, 4);
 }
 void debugMotorSpeeds() {
 Serial.print("\tl m S: ");
 printFloat(leftMotorSpeed, 4);
 Serial.print("\tr m S: ");
 printFloat(rightMotorSpeed, 4);
 }
 
 void debugImu() {
 Serial.print("aX: ");
 printInt(imuValues[0], 4);
 Serial.print("\taY: ");
 printInt(imuValues[1], 4);
 Serial.print("\taZ: ");
 printInt(imuValues[2], 4);
 Serial.print("\tgX: ");
 printInt(imuValues[3], 4);
 Serial.print("\tgY: ");
 printInt(imuValues[4], 4);
 Serial.print("\tgZ: ");
 printInt(imuValues[5], 4);
 }
 
 void debugMotorSpeedCalibration() {
 Serial.print("\t");
 Serial.print(anglePIDOutput*motor1Calibration);
 Serial.print("\t");
 Serial.print(anglePIDOutput*motor2Calibration);
 Serial.print("\t");
 Serial.print(rightMotorSpeed);
 Serial.print("\t");
 Serial.print(leftMotorSpeed);
 }
 
 void printInt(int number, byte width) {
 int currentMax = 10;
 if (number < 0) 
 currentMax = 1;
 for (byte i=1; i<width; i++){
 if (fabs(number) < currentMax) {
 Serial.print(" ");
 }
 currentMax *= 10;
 }
 Serial.print(number);
 }
 void printFloat(float number, byte width) {
 int currentMax = 10;
 if (number < 0) 
 currentMax = 1;
 for (byte i=1; i<width; i++){
 if (fabs(number) < currentMax) {
 Serial.print(" ");
 }
 currentMax *= 10;
 }
 Serial.print(number);
 }
 */
void debugConfiguration() {
 Serial.print("speedPIDKp: ");
 Serial.println(configuration.speedPIDKp);
 Serial.print("speedPIDKi: ");
 Serial.println(configuration.speedPIDKi);
 Serial.print("speedPIDKd: ");
 Serial.println(configuration.speedPIDKd);
 
 Serial.print("speedPidOutputLowerLimit: ");
 Serial.println(configuration.speedPIDOutputLowerLimit);
 Serial.print("speedPidOutputHigherLimit: ");
 Serial.println(configuration.speedPIDOutputHigherLimit);
 
 Serial.print("anglePIDAggKp: ");
 Serial.println(configuration.anglePIDAggKp);
 Serial.print("anglePIDAggKi: ");
 Serial.println(configuration.anglePIDAggKi);
 Serial.print("anglePIDAggKd: ");
 Serial.println(configuration.anglePIDAggKd);
 
 Serial.print("anglePIDConKp: ");
 Serial.println(configuration.anglePIDConKp);
 Serial.print("anglePIDConKi: ");
 Serial.println(configuration.anglePIDConKi);
 Serial.print("anglePIDConKd: ");
 Serial.println(configuration.anglePIDConKd);
 
 Serial.print("anglePIDLowerLimit: ");
 Serial.println(configuration.anglePIDLowerLimit);
 
 Serial.print("anglePIDSampling: ");
 Serial.println(configuration.anglePIDSampling);
 Serial.print("motorsPIDSampling: ");
 Serial.println(configuration.speedPIDSampling);
 Serial.print("angleSensorSampling: ");
 Serial.println(configuration.angleSensorSampling);
 Serial.print("motorSpeedSensorSampling: ");
 Serial.println(configuration.motorSpeedSensorSampling);
 }






