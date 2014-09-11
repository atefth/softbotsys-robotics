#include <Servo.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <MPU6050.h>
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;
Servo myservo;
int val;
int prevVal;
void setup() 
{
    Wire.begin();
    Serial.begin(9600);
    Serial.println("Initialize MPU");
    mpu.initialize();
    Serial.println(mpu.testConnection() ? "Connected" : "Connection failed");
    myservo.attach(11);
}
void loop() 
{
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    val = map(ay, -17000, 17000, -180, 180);
    if (val != prevVal)
    {
        myservo.write(val);
        prevVal = val;
    }
    delay(50);
}
