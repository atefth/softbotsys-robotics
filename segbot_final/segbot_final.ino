// Test MD03a / Pololu motor with encoder
// speed control (PI), V & I display
// Credits:
//   Dallaby   http://letsmakerobots.com/node/19558#comment-49685
//   Bill Porter  http://www.billporter.info/?p=286
//   bobbyorr (nice connection diagram) http://forum.pololu.com/viewtopic.php?f=15&t=1923

#include "Servo.h"
#include "Encoder.h"
//motors
//motor 1 pins
#define motor_1_dir       4                      // motor direction pin
#define motor_1_in        5                      // motor input pin
//motor 2 pins
#define motor_2_dir       7                      // motor direction pin
#define motor_2_in        6                      // motor input pin

//encoders
//encoder 1 pins
#define encoderPinA1      21                    // encoder pin 1
#define encoderPinA2      19            // encoder pin 2
//encoder 2 pins
#define encoderPinB1      20                    // encoder pin 1
#define encoderPinB2      18            // encoder pin 2
//interrupt pins
#define interruptPIN1    3
#define interruptPIN2    5

//ir sensors
#define ir_sensor_1_PIN    0
#define ir_sensor_2_PIN    1
#define ir_sensor_3_PIN    2
#define ir_sensor_4_PIN    3
#define ir_sensor_5_PIN    4

//ultrasonic sensor
#define trigPin 13
#define echoPin 12

static int velocity = 100;                       // speed (Set Point)

//actiual speed of motors
static int speed_act_1 = 0;                     // speed (actual value) of motor 1
static int speed_act_2 = 0;                     // speed (actual value) of motor 2

//required speed of motors
static int velocity_1 = velocity;
static int velocity_2 = velocity;

//power output to motors
int PWM_1 = 0;                                  // (25% = 64; 50% = 127; 75% = 191; 100% = 255)
int PWM_2 = 0;                                  // (25% = 64; 50% = 127; 75% = 191; 100% = 255)

//revolution counter for motors
volatile long count_1 = 0;                      // revolution counter for motor 1
volatile long count_2 = 0;                      // revolution counter for motor 2

//PID updating constants
float Kp =   .5;                                // PID proportional control Gain
float Kd =    1;                                // PID Derivitave control gain

//time delay after loop
static int timeDelay = 0;

//number of revolution per loop
static int rpl = 1;

//time
int time = 0;

//for rotation
static int encoder_count = 500;

//for SR04
static long duration, distance;
static int safeDist = 10;

//for servo
Servo myservo;                                  // create servo object to control a servo 
static int pos = 0;
 
//for steering
long steeringConst = 1;                  //Constant for turning left or right
int steeringReg = 2;                  //Constant for regulating the steering factor

//for bluetooth
static char determinant;         //Used in the check function, stores the character received from the phone.
static char det;                 //Used in the loop function, stores the character received from the phone.
static char dataIn = 'S';        //Character/Data coming from the phone.

//for ir sensor 1
static int sensor_1 = 0;
static int adj_1 = 0;
static int s1_min = 45;
static int s1_max = 55;

//for ir sensor 2
static int sensor_2 = 0;
static int adj_2 = 0;
static int s2_min = 45;
static int s2_max = 55;

//for ir sensor 3
static int sensor_3 = 0;
static int adj_3 = 0;
static int s3_min = 45;
static int s3_max = 52;

//for ir sensor 4
static int sensor_4 = 0;
static int adj_4 = 0;
static int s4_min = 48;
static int s4_max = 55;

//for ir sensor 5
static int sensor_5 = 0;
static int adj_5 = 0;
static int s5_min = 46;
static int s5_max = 58;

//for ir sensors
static int threshold = 120;
static int speed_value = 255;

void setup() {
  Serial.begin(9600);

  //for SR04
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  //for servo
  myservo.attach(9);    // attaches the servo on pin 9 to the servo object
  
  //for motor 1
  pinMode(motor_1_dir, OUTPUT);
  pinMode(motor_1_in, OUTPUT);
  //for motor 2  
  pinMode(motor_2_dir, OUTPUT);
  pinMode(motor_2_in, OUTPUT);
  //for encoder 1
  encoder_1_begin(encoderPinA1, encoderPinA2);
  //for encoder 2
  encoder_2_begin(encoderPinB1, encoderPinB2);
  initialise();
  
}

//initialise
void initialise(){
  encoder_1_Count();
  encoder_2_Count();
}

//encoder 1 counter
int encoder_1_Count(){
  int dir = encoder_1_data(); // Check for rotation
  
  if(dir == 1)       // If its forward...
  {
    count_1++;       // Increment the counter
  }
  
  else if(dir == -1) // If its backward...
  {
    count_1--;       // Decrement the counter
  }
}

//encoder 2 counter
int encoder_2_Count(){
  int dir = encoder_2_data(); // Check for rotation
  
  if(dir == 1)       // If its forward...
  {
    count_2++;       // Increment the counter
  }
  
  else if(dir == -1) // If its backward...
  {
    count_2--;       // Decrement the counter
  }
}

//for ir sensor
void updateIRSensors(){
  //adjusting sensor 1
  sensor_1 = analogRead(0);
  adj_1 = map(sensor_1, s1_min, s1_max, 0, speed_value);
  adj_1 = constrain(adj_1, 0, 255); 
  
  //adjusting sensor 2
  sensor_2 = analogRead(1);
  adj_2 = map(sensor_2, s2_min, s2_max, 0, speed_value);
  adj_2 = constrain(adj_2, 0, 255); 
  
  //adjusting sensor 3
  sensor_3 = analogRead(2);
  adj_3 = map(sensor_3, s3_min, s3_max, 0, speed_value);
  adj_3 = constrain(adj_3, 0, 255); 
  
  //adjusting sensor 4
  sensor_4 = analogRead(3);
  adj_4 = map(sensor_4, s4_min, s4_max, 0, speed_value);
  adj_4 = constrain(adj_4, 0, 255); 
  
  //adjusting sensor 5
  sensor_5 = analogRead(4);
  adj_5 = map(sensor_5, s5_min, s5_max, 0, speed_value);
  adj_5 = constrain(adj_5, 0, 255); 
}

//check if front has obstacle
int isFrontClear(){
  myservo.writeMicroseconds(1500);              // tell servo to go to 0 position
  return ultrasoundDistance();
}

//check if Left has obstacle
boolean isLeftClear(){
  myservo.writeMicroseconds(1500);              // tell servo to go to 0 position
  int distance = 0;
  for(pos = 90; pos<180; pos+=1)          // goes from 0 degrees to -90 degrees 
  {                                  // in steps of 15 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  }
  for(pos; pos<=90; pos-=1)          // goes from -90 degrees to 0 degrees 
  {                                  // in steps of 15 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  }
  return distance; 
}  

//check if Right has obstacle
boolean isRightClear(){
  myservo.writeMicroseconds(1500);              // tell servo to go to 0 position
  int distance = 0;
  for(pos; pos < 90; pos += 1)  // goes from 0 degrees to 90 degrees 
  {                                  // in steps of 15 degree     
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position     
  }
  distance = ultrasoundDistance(); 
  for(pos; pos >= 1; pos -= 1)  // goes from 90 degrees to 0 degrees 
  {                                  // in steps of 15 degree     
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  }
  return distance; 
}

//distance to obstacle
int ultrasoundDistance(){
 digitalWrite(trigPin, LOW);
 // delayMicroseconds(2);
 digitalWrite(trigPin, HIGH);
 //delayMicroseconds(10);
 digitalWrite(trigPin, LOW);
 duration =pulseIn(echoPin, HIGH);
 distance = (duration/2)/29.1;
 return (int)distance;
}

void loop() {
  //  updateIRSensors();
  moveForward();
  printMotorInfo();
  //  move with bluetooth
  //  throughBluetooth();
  //  Serial.print("adjust_1 : "); Serial.println(adj_1);
  //  Serial.print("adjust_2 : "); Serial.println(adj_2);
  //  Serial.print("adjust_3 : "); Serial.println(adj_3);
  //  Serial.print("adjust_4 : "); Serial.println(adj_4);
  //  Serial.print("adjust_5 : "); Serial.println(adj_5);  
  //  lineFollower();

}

void throughSerial(){
  // check keyboard
}

//move with bluetooth
void throughBluetooth(){
      det = check();
      while (det == 'F')   //if incoming data is a F, move forward
      {     
          moveForward(); 
          det = check();          
      }  
      while (det == 'B')   //if incoming data is a B, move back
      {    
          moveBackward();       
          det = check();          
      } 

      while (det == 'L')   //if incoming data is a L, move wheels left
      {     
          moveLeft();
          det = check();          
      }  
      while (det == 'R')   //if incoming data is a R, move wheels right
      {    
          moveRight();  
          det = check();          
      }
      while (det == 'S')   //if incoming data is a S, stop
      {
          halt();    
          det = check(); 
      }
}

//line follower logic
void lineFollower(){
  switch(whichIRSensor()){
   case 1:
    moveLeft();
    break;
   case 2:
    moveSoftLeft();
    break;
   case 3:
    moveForward();
    break;
   case 4:
    moveSoftRight();
    break;
   case 5:
    moveRight();
    break;
   case 0:
    halt();
    break;  
  }
}

//detect active ir sensor
int whichIRSensor(){
  if(adj_1 > threshold && adj_1 != speed_value){
    return 1;
  }
  else if (adj_2 > threshold && adj_2 != speed_value){
    return 2;
  }
  else if (adj_3 > threshold && adj_3 != speed_value){
    return 3;
  }
  else if (adj_4 > threshold && adj_4 != speed_value){
    return 4;
  }
  else if (adj_5 > threshold && adj_5 != speed_value){
    return 5;
  }else{
    return 0;
  }
}

//stop the motors
void halt(){
  analogWrite(motor_1_in, 0);                    
  analogWrite(motor_2_in, 0);
  delay(timeDelay); 
}

//move forward
void moveForward(){
  velocity_1 = velocity;
  velocity_2 = velocity;
  while(count_1 < (encoder_count * rpl)){
    initialise();
    analogWrite(motor_1_in, PWM_1);          //giving motor required power "PWM_1" which is on pin "motor_1_in"
    digitalWrite(motor_1_dir, LOW);          //motor pin for direction "motor_1_dir" is provided with "LOW" input
    analogWrite(motor_2_in, PWM_2);
    digitalWrite(motor_2_dir, LOW);
  }
  analogWrite(motor_1_in, 0);                    
  analogWrite(motor_2_in, 0);
  delay(timeDelay);
}

//move backward
void moveBackward(){
  velocity_1 = velocity;
  velocity_2 = velocity;
    while(count_1 < (encoder_count * rpl)){
      analogWrite(motor_1_in, PWM_1);          //giving motor required power "PWM_1" which is on pin "motor_1_in"
      digitalWrite(motor_1_dir, HIGH);          //motor pin for direction "motor_1_dir" is provided with "LOW" input
      analogWrite(motor_2_in, PWM_2);
      digitalWrite(motor_2_dir, HIGH);
    }
    analogWrite(motor_1_in, 0);                    
    analogWrite(motor_2_in, 0);
 
}

//move left
boolean moveLeft(){
  velocity_2 = (steeringConst * velocity);
    while(count_1 < encoder_count * rpl){
      analogWrite(motor_1_in, PWM_1);          //giving motor required power "PWM_1" which is on pin "motor_1_in"
      digitalWrite(motor_1_dir, HIGH);          //motor pin for direction "motor_1_dir" is provided with "LOW" input
      analogWrite(motor_2_in, PWM_2);
      digitalWrite(motor_2_dir, LOW);
  
    }
    analogWrite(motor_1_in, 0);                    
    analogWrite(motor_2_in, 0);
}

//move left softly
boolean moveSoftLeft(){
   velocity_2 = (steeringConst * velocity)/steeringReg;
    while(count_1 < encoder_count * rpl){
      analogWrite(motor_1_in, PWM_1);          //giving motor required power "PWM_1" which is on pin "motor_1_in"
      digitalWrite(motor_1_dir, HIGH);          //motor pin for direction "motor_1_dir" is provided with "LOW" input
      analogWrite(motor_2_in, PWM_2);
      digitalWrite(motor_2_dir, LOW);
  
      }
    analogWrite(motor_1_in, 0);                    
    analogWrite(motor_2_in, 0); 
}

//move right
boolean moveRight(){
  velocity_1 = (steeringConst * velocity);
    while(count_1 < encoder_count * rpl){
      analogWrite(motor_1_in, PWM_1);          //giving motor required power "PWM_1" which is on pin "motor_1_in"
      digitalWrite(motor_1_dir, LOW);          //motor pin for direction "motor_1_dir" is provided with "LOW" input
      analogWrite(motor_2_in, PWM_2);
      digitalWrite(motor_2_dir, HIGH);
  
    }
    analogWrite(motor_1_in, 0);                    
    analogWrite(motor_2_in, 0);
}

//move right softly
boolean moveSoftRight(){
  velocity_1 = (steeringConst * velocity)/steeringReg;
    while(count_1 < encoder_count * rpl){
      analogWrite(motor_1_in, PWM_1);          //giving motor required power "PWM_1" which is on pin "motor_1_in"
      digitalWrite(motor_1_dir, LOW);          //motor pin for direction "motor_1_dir" is provided with "LOW" input
      analogWrite(motor_2_in, PWM_2);
      digitalWrite(motor_2_dir, HIGH);
  
    }
    analogWrite(motor_1_in, 0);                    
    analogWrite(motor_2_in, 0);
}

//control feedback signal to motor 1
int updatePid_1(int command, int targetValue, int currentValue)   {             // compute PWM value
  float pidTerm = 0;                                                            // PID correction
  int error=0;                                  
  static int last_error_1=0;                             
  error = abs(targetValue) - abs(currentValue); 
  pidTerm = (Kp * error) + (Kd * (error - last_error_1));                            
  last_error_1 = error;
  return constrain(command + int(pidTerm), 0, 255);
}

//control feedback signal to motor 2
int updatePid_2(int command, int targetValue, int currentValue)   {             // compute PWM value
  float pidTerm = 0;                                                            // PID correction
  int error=0;                                  
  static int last_error_2=0;                             
  error = abs(targetValue) - abs(currentValue); 
  pidTerm = (Kp * error) + (Kd * (error - last_error_2));                            
  last_error_2 = error;
  return constrain(command + int(pidTerm), 0, 255);
}

//print motor variables
void printMotorInfo()  {                                                      // display data
    Serial.print("  COUNT 1:");          Serial.println(count_1);  
    Serial.print("  COUNT 2:");          Serial.println(count_2);           
}

//receive bluetooth signal
int check(){
  if (Serial.available() > 0) {   //Check for data on the serial lines.   
        dataIn = Serial.read();  //Get the character sent by the phone and store it in 'dataIn'.
        if (dataIn == 'F')
        {     
          determinant = 'F';
        }  
        else if (dataIn == 'B')
        { 
          determinant = 'B'; 
        }
        else if (dataIn == 'L')  
        { 
          determinant = 'L';
        }
        else if (dataIn == 'R')  
        { 
          determinant = 'R';
        } 
        else if (dataIn == 'I')  
        { 
          determinant = 'I'; 
        }  
        else if (dataIn == 'J')  
        {  
          determinant = 'J';
        }          
        else if (dataIn == 'G') 
        {
          determinant = 'G'; 
        }    
        else if (dataIn == 'H')  
        {
          determinant = 'H'; 
        }   
        else if (dataIn == 'S') 
        {
          determinant = 'S';
        }
        else if (dataIn == '0') 
        {
          velocity = 0;    //"velocity" does not need to be returned.
        }
        else if (dataIn == '1') 
        {
          velocity = 25;
        }
        else if (dataIn == '2') 
        {
          velocity = 50;
        }
        else if (dataIn == '3') 
        {
          velocity = 75;
        }
        else if (dataIn == '4') 
        {
          velocity = 100;
        }
        else if (dataIn == '5') 
        {
          velocity = 125;
        }
        else if (dataIn == '6') 
        {
          velocity = 150;
        }
        else if (dataIn == '7') 
        {
          velocity = 175;
        }
        else if (dataIn == '8') 
        {
          velocity = 200;
        }
        else if (dataIn == '9') 
        {
          velocity = 225;
        }
        else if (dataIn == 'q') 
        {
          velocity = 255;
        }
  }
  return determinant;
}
