#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h" // for i2c
#include <Kalman.h>
Kalman kalman;

int setpoint = 0;
#define Kp 5;
#define Kd 1.0005;

//pid
 double currentGyroOffset = 0;
 double lastGyroOffset = 0;
 double currentGyroDeviation = 0;
 double lastGyroDeviation = 0;

double currentPower = 50;
double lastPower = 0;

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
double gyroReading;
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

#define FORWARD LOW
#define BACKWARD HIGH

int motorDir_a1 = 2;
int motorDir_a2 = 4;
int motorPWM_1 = 3;

int motorDir_b1 = 7;
int motorDir_b2 = 8;
int motorPWM_2 = 9;

int steering = 1;

void setup(){  
  Serial.begin(57600);
  Wire.begin();

  initiate();

  // initialize device
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();
   
  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");;
  loopStartTime = micros();
  timer = loopStartTime;
  calibrateSensors();
  
 pinMode(motorDir_a1, OUTPUT);
  pinMode(motorDir_a2, OUTPUT);
 pinMode(motorPWM_1, OUTPUT);
  pinMode(motorDir_b1, OUTPUT);
 pinMode(motorDir_b2, OUTPUT);
 pinMode(motorPWM_2, OUTPUT); 
}

void loop(){
  
  //gyro to PID  
  Serial.print("GyroValue ---> ");
  Serial.print(gyroReading);  
  Serial.print("        ");
  Serial.print("lastOffset ---> ");
  Serial.print(getLastGyroOffset());
  Serial.print("        ");
  Serial.print("currentOffset ---> ");
  Serial.print(getCurrentGyroOffset());
  Serial.print("        ");
  Serial.print("lastDeviation ---> ");
  Serial.print(getLastGyroDeviation());
  Serial.print("        ");
  Serial.print("currentDeviation ---> ");
  Serial.print(getCurrentGyroDeviation());
  Serial.print("        ");
  Serial.print("Power ---> ");
  Serial.println(currentPower);  
  balance();
  //delay(1000);
}

void balance(){
  updateIMUSensors();
  adjustCurrentPower();
  if (gyroReading > 0 && gyroReading < 2){
    moveForward();
  }else if(gyroReading > -2 && gyroReading < 0){
   moveBackward(); 
  }else if (gyroReading > 3){
   moveForward();
  }else{
   moveBackward(); 
  }
}

void brake(){
  analogWrite(motorPWM_1, 0);
  analogWrite(motorPWM_2, 0);
}

void moveForward(){
  digitalWrite(motorDir_a1, FORWARD);
  digitalWrite(motorDir_a2, BACKWARD);  
  analogWrite(motorPWM_1, currentPower);

  digitalWrite(motorDir_b1, FORWARD);  
  digitalWrite(motorDir_b2, BACKWARD);
  analogWrite(motorPWM_2, currentPower);  
}

void moveBackward(){
  digitalWrite(motorDir_a1, BACKWARD);
  digitalWrite(motorDir_a2, FORWARD);  
  analogWrite(motorPWM_1, currentPower);
  
  digitalWrite(motorDir_b1, BACKWARD);  
  digitalWrite(motorDir_b2, FORWARD);
  analogWrite(motorPWM_2, currentPower);  
}

//void moveLeft(){
//  digitalWrite(motorDir_1, FORWARD);
//  analogWrite(motorPWM_1, currentPower);
//  
//  digitalWrite(motorDir_2, BACKWARD);
//  analogWrite(motorPWM_2, currentPower * steering);
//}
//
//void moveRight(){
//  digitalWrite(motorDir_1, BACKWARD);
//  analogWrite(motorPWM_1, currentPower * steering);
//  
//  digitalWrite(motorDir_2, FORWARD);
//  analogWrite(motorPWM_2, currentPower);  
//}

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
  gyroReading = kalman.getAngle(accYangle, gyroYrate, (double)(micros() - timer)/1000000); // Calculate the angle using a Kalman filter
  //gyroReading -= 90;
  timer = micros();
  time_stamp(); 
}

