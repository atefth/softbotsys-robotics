// Test MD03a / Pololu motor with encoder
// speed control (PI), V & I display
// Credits:
//   Dallaby   http://letsmakerobots.com/node/19558#comment-49685
//   Bill Porter  http://www.billporter.info/?p=286
//   bobbyorr (nice connection diagram) http://forum.pololu.com/viewtopic.php?f=15&t=1923

#include "Servo.h"
//#include "Encoder_Polling.h" 
#define In1            4                      // INA motor pin
#define In2            7                      // INB motor pin 
#define PWM1            5                       // PWM motor pin
#define PWM2            6
//encoders
#define encoderPinA1      21                       // encoder A pin
#define encoderPinB1      20                       // encoder B pin
#define encoderPinA2      19
#define encoderPinB2      18
#define interruptPIN1    3
#define interruptPIN2    5

//for ir sensors
#define ir_sensor_1_PIN    0
#define ir_sensor_2_PIN    1
#define ir_sensor_3_PIN    2
#define ir_sensor_4_PIN    3
#define ir_sensor_5_PIN    4

//for SR04 ultrasonic sensor
#define trigPin 13
#define echoPin 12

#define CURRENT_LIMIT   1000                    // high current warning
#define LOW_BAT         10000                   // low bat warning
#define LOOPTIME        100                     // PID loop time
#define NUMREADINGS     10                      // samples for Amp average

int readings[NUMREADINGS];
unsigned long lastMilli = 0;                    // loop timing 
unsigned long lastMilliPrint = 0;               // loop timing
static int velocity = 70;                       // speed (Set Point)

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

//time
int time = 0;

//for rotation
static int revolution_count = 34000;

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
  pinMode(In1, OUTPUT);
  pinMode(PWM1, OUTPUT);
  //for motor 2  
  pinMode(In2, OUTPUT);
  pinMode(PWM2, OUTPUT);
  //for encoder 1
  pinMode(encoderPinA1, INPUT);
  digitalWrite(encoderPinA1, HIGH);        // turn on pullup resistor 
  pinMode(encoderPinA2, INPUT);
  digitalWrite(encoderPinA2, HIGH);
  attachInterrupt(interruptPIN1, rencoder_1, FALLING);
  //for encoder 2
  pinMode(encoderPinB1, INPUT);
  digitalWrite(encoderPinB1, HIGH);
  pinMode(encoderPinB2, INPUT);             // turn on pullup resistor
  digitalWrite(encoderPinB2, HIGH);
  attachInterrupt(interruptPIN2, rencoder_2, FALLING);
  
  // initialize readings to 0
  for(int i=0; i<NUMREADINGS; i++){
    readings[i] = 0;  
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
  //  move with bluetooth
      //throughBluetooth();
      initialize();
      moveForward();
      printMotorInfo();
  //  Serial.print("adjust_1 : "); Serial.println(adj_1);
  //  Serial.print("adjust_2 : "); Serial.println(adj_2);
  //  Serial.print("adjust_3 : "); Serial.println(adj_3);
  //  Serial.print("adjust_4 : "); Serial.println(adj_4);
  //  Serial.print("adjust_5 : "); Serial.println(adj_5);  
  //  lineFollower();
  //    printMotorInfo();
  //    if(millis()<100000){
  //      moveForward();
  //      delay(15000);
  //      moveLeft();
  //    }
  // check keyboard
  //moveForward();
  //moveLeft();
  //moveSoftLeft();
  //moveRight();
  //moveSoftRight(); 
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

//initiate motor variables
boolean initialize(){
    count_1 = 0;
    count_2 = 0;
    if((millis()-lastMilli) >= LOOPTIME){                                    // enter tmed loop
      lastMilli = millis();
      getMotorData();     
      PWM_1= updatePid_1(PWM_1, velocity_1, speed_act_1);
      PWM_2= updatePid_2(PWM_2, velocity_2, speed_act_2);
      return true; 
    }else{
      return false; 
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
  analogWrite(PWM1, 0);                    
  analogWrite(PWM2, 0);
  //printMotorInfo();
  delay(timeDelay);
}

//move forward
void moveForward(){
  velocity_1 = velocity;
  velocity_2 = velocity;
  while(count_1 < revolution_count){
    analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
    digitalWrite(In1, LOW);          //motor pin for direction "In1" is provided with "LOW" input
    analogWrite(PWM2, PWM_2);
    digitalWrite(In2, LOW);
  }
  analogWrite(PWM1, 0);                    
  analogWrite(PWM2, 0);
  //printMotorInfo();
  delay(timeDelay);
}

//move backward
void moveBackward(){
  velocity_1 = velocity;
  velocity_2 = velocity;
    while(count_1 < revolution_count){
      analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
      digitalWrite(In1, HIGH);          //motor pin for direction "In1" is provided with "LOW" input
      analogWrite(PWM2, PWM_2);
      digitalWrite(In2, HIGH);
    }
    analogWrite(PWM1, 0);                    
    analogWrite(PWM2, 0);
    //printMotorInfo(); 
}

//move left
void moveLeft(){
  velocity_2 = (steeringConst * velocity);
    while(count_1 < revolution_count){
      analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
      digitalWrite(In1, HIGH);          //motor pin for direction "In1" is provided with "LOW" input
      analogWrite(PWM2, PWM_2);
      digitalWrite(In2, LOW);
      //printMotorInfo();
    }
    analogWrite(PWM1, 0);                    
    analogWrite(PWM2, 0);
}

//move left softly
void moveSoftLeft(){
   velocity_2 = (steeringConst * velocity)/steeringReg;
    while(count_1 < revolution_count){
      analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
      digitalWrite(In1, HIGH);          //motor pin for direction "In1" is provided with "LOW" input
      analogWrite(PWM2, PWM_2);
      digitalWrite(In2, LOW);
      //printMotorInfo();
      }
    analogWrite(PWM1, 0);                    
    analogWrite(PWM2, 0);
}

//move right
void moveRight(){
  velocity_1 = (steeringConst * velocity);
    while(count_1 < revolution_count){
      analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
      digitalWrite(In1, LOW);          //motor pin for direction "In1" is provided with "LOW" input
      analogWrite(PWM2, PWM_2);
      digitalWrite(In2, HIGH);
      //printMotorInfo();
    }
    analogWrite(PWM1, 0);                    
    analogWrite(PWM2, 0);
}

//move right softly
void moveSoftRight(){
  velocity_1 = (steeringConst * velocity)/steeringReg;
    while(count_1 < revolution_count){
      analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
      digitalWrite(In1, LOW);          //motor pin for direction "In1" is provided with "LOW" input
      analogWrite(PWM2, PWM_2);
      digitalWrite(In2, HIGH);
      //printMotorInfo();
    }
    analogWrite(PWM1, 0);                    
    analogWrite(PWM2, 0);
}

//get motor variable readings
void getMotorData()  {                                                        // calculate speed, volts and Amps
  static long countAnt_1 = 0;
  static long countAnt_2 = 0;
  // last count
  speed_act_1 = ((count_1 - countAnt_1)*(60*(100/LOOPTIME)))/(334);          // 16 pulses X 29 gear ratio = 464 counts per output shaft rev
  countAnt_1 = count_1; 
  speed_act_2 = ((count_2 - countAnt_2)*(60*(100/LOOPTIME)))/(334);          // 16 pulses X 29 gear ratio = 464 counts per output shaft rev
  countAnt_2 = count_2;     
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
  if((millis()-lastMilliPrint) >= 500)   {                     
    lastMilliPrint = millis();
    Serial.print("  COUNT 1:");          Serial.println(count_1);  
    Serial.print("  COUNT 2:");          Serial.println(count_2);           

    //if (current > CURRENT_LIMIT)               Serial.println("*** CURRENT_LIMIT ***");                
    //if (voltage > 1000 && voltage < LOW_BAT)   Serial.println("*** LOW_BAT ***");                
  }
}

//count for encoder of motor 1
void rencoder_1()  {                                    // pulse and direction, direct port reading to save cycles
  if (digitalRead(encoderPinA1)==LOW)    count_1++;                // if(digitalRead(encoderPinB1)==HIGH)   count ++;
  else                      count_1--;  // if (digitalRead(encoderPinB1)==LOW)   count --;
}

//count for encoder of motor 2
void rencoder_2()  {                                    // pulse and direction, direct port reading to save cycles
  if (digitalRead(encoderPinB2)==LOW)    count_2++;                // if(digitalRead(encoderPinB1)==HIGH)   count ++;
  else                      count_2--;  // if (digitalRead(encoderPinB1)==LOW)   count --;
}

//remove noise signal
int digital_smooth(int value, int *data_array)  {    // remove signal noise
  static int ndx=0;                                                         
  static int count=0;                          
  static int total=0;                          
  total -= data_array[ndx];               
  data_array[ndx] = value;                
  total += data_array[ndx];               
  ndx = (ndx+1) % NUMREADINGS;                                
  if(count < NUMREADINGS)      count++;
  return total/count;
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
