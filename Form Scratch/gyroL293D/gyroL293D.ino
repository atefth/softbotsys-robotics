#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int val;
int prevVal;

int power = 150;
#define FORWARD LOW
#define BACKWARD HIGH

int motorDir_A1 = 2;
int motorDir_A2 = 4;
int motorPWM_1 = 3;

int motorDir_B1 = 7;
int motorDir_B2 = 8;
int motorPWM_2 = 9;

int timePeriod = 50;

void setup() 
{
    Wire.begin();
    Serial.begin(57600);
    Serial.println("Initialize MPU");
    mpu.initialize();
    Serial.println(mpu.testConnection() ? "Connected" : "Connection failed");
    pinMode(motorDir_A1, OUTPUT);
    pinMode(motorDir_A2, OUTPUT);
    pinMode(motorPWM_1, OUTPUT);
    pinMode(motorDir_B1, OUTPUT);
    pinMode(motorDir_B2, OUTPUT);
    pinMode(motorPWM_2, OUTPUT); 
}
void loop() 
{
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    val = map(ay, -17000, 17000, 90, -90);
    Serial.println(val);
    if (val > 0  && val < 15){
      moveBackward();
    }else if (val < 0 && val > -15){
      moveForward(); 
    }else if (val == 0){
     brake(); 
    }else if(val > 15 && val < 35){
      power = 230;
      moveBackward();
    }else if(val < -15 && val > -35){
      power = 230;
      moveForward();
    }

}
