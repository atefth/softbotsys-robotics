#include <PID_v1.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h" // for i2c
#include <Kalman.h>
Kalman kalman;

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

// motor controller
#define pwm_a 5   //PWM control for motor outputs 1 and 2 is on digital pin 3
#define pwm_b 6  //PWM control for motor outputs 3 and 4 is on digital pin 11
#define dir_a 4  //direction control for motor outputs 1 and 2 is on digital pin 12
#define dir_b 7  //direction control for motor outputs 3 and 4 is on digital pin 13

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

//PID variables
double Setpoint, Input, Output;
//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint,1,1,1, DIRECT);

void setup() {
  Serial.begin(57600);
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

  //PID
  Setpoint = 0;
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255);

  //Set control pins to be outputs
  pinMode(pwm_a, OUTPUT);  
  pinMode(pwm_b, OUTPUT);
  pinMode(dir_a, OUTPUT);
  pinMode(dir_b, OUTPUT);

  int motorPower = 100;
  
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
  //roll -= 90;
  timer = micros();
  time_stamp(); 
}
void getGyrodata(){
    // print values to serial monitor for checking 
	//  Serial.print(accYangle);
	//  Serial.print("\t");
    //Serial.println("\t");
	//  Serial.print(roll);
	//  Serial.print("\t");
	//  Serial.print(gyro_angle);
	//  Serial.print("\t");
	//  Serial.println(" ");
}

void loop(){
	updateIMUSensors();
	//gyro to PID
    Input = roll;
  	myPID.Compute();
	  Serial.print("GyroValue ---> ");
    Serial.print(roll);
    Serial.print("        ");
    Serial.print("PIDValue ---> ");
    Serial.println(Output);
    detMovement();
}

void detMovement(){
  if (Output > 0)
  {
    moveMotorsForward();
  }else if (Output < 0)
  {
    moveMotorsBackward();
  }
}

void moveMotorsForward(){
  digitalWrite(dir_a, HIGH);
  analogWrite(pwm_a, motorPower);
  digitalWrite(dir_b, HIGH);
  analogWrite(pwm_b, motorPower);
}

void moveMotorsBackward(){
  digitalWrite(dir_a, LOW);
  analogWrite(pwm_a, motorPower);
  digitalWrite(dir_b, LOW);
  analogWrite(pwm_b, motorPower);
}
