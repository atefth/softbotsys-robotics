// Test MD03a / Pololu motor with encoder
// speed control (PI), V & I display
// Credits:
//   Dallaby   http://letsmakerobots.com/node/19558#comment-49685
//   Bill Porter  http://www.billporter.info/?p=286
//   bobbyorr (nice connection diagram) http://forum.pololu.com/viewtopic.php?f=15&t=1923

#include <Servo.h> 
#define In1            4                      // INA motor pin
#define In2            7                      // INB motor pin 
#define PWM1            5                       // PWM motor pin
#define PWM2            6
#define encodPin1      2                       // encoder A pin
#define encodPin2      3                       // encoder B pin
#define encodPinA2      19
#define encodPinB2      18

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
static int velocity = 50;                       // speed (Set Point)

//actiual speed of motors
static int speed_act_1 = 0;                     // speed (actual value) of motor 1
static int speed_act_2 = 0;                     // speed (actual value) of motor 2

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
static int magnitudeTurns = 1;

//time
int time = 0;

//for rotation
static int moveFactor = 1, moveTurns = 8500, moveConst = 0;

//for SR04
static long duration, distance;
static int safeDist = 10;

//for servo
Servo myservo;                                  // create servo object to control a servo 
static int pos = 0;
 
//for steering
int steeringConst = 1;                  //Constant for turning left or right
int steeringReg = 5;                  //Constant for regulating the steering factor

//for bluetooth
static char determinant;         //Used in the check function, stores the character received from the phone.
char det;                 //Used in the loop function, stores the character received from the phone.
char dataIn = 'S';        //Character/Data coming from the phone.

//for ir sensor 1
static int sensor_1 = 0;
static int adj_1 = 0;
static int s1_min = 45;
static int s1_max = 57;

//for ir sensor 2
static int sensor_2 = 0;
static int adj_2 = 0;
static int s2_min = 43;
static int s2_max = 58;

//for ir sensor 3
static int sensor_3 = 0;
static int adj_3 = 0;
static int s3_min = 43;
static int s3_max = 57;

//for ir sensor 4
static int sensor_4 = 0;
static int adj_4 = 0;
static int s4_min = 45;
static int s4_max = 59;

//for ir sensor 5
static int sensor_5 = 0;
static int adj_5 = 0;
static int s5_min = 43;
static int s5_max = 59;

//for ir sensors
static int threshold = 150;

void setup() {
  Serial.begin(115200);

  //for SR04
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  //for servo
  myservo.attach(9);    // attaches the servo on pin 9 to the servo object
  
  pinMode(In1, OUTPUT);
  pinMode(PWM1, OUTPUT);
  
  pinMode(In2, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(encodPin1, INPUT); 
  pinMode(encodPin2, INPUT); 
  digitalWrite(encodPin1, HIGH);                      // turn on pullup resistor
  digitalWrite(encodPin2, HIGH);
  pinMode(encodPinA2, INPUT); 
  pinMode(encodPinB2, INPUT); 
  digitalWrite(encodPinA2, HIGH);                      // turn on pullup resistor
  digitalWrite(encodPinB2, HIGH);
  attachInterrupt(1, rencoder_1, FALLING);
  attachInterrupt(5, rencoder_2, FALLING);
  for(int i=0; i<NUMREADINGS; i++)   readings[i] = 0;  // initialize readings to 0

  
}

//for ir sensor
void updateIRSensors(){
  
  //adjusting sensor 1
  sensor_1 = analogRead(0);
  adj_1 = map(sensor_1, s1_min, s1_max, 0, 255);
  adj_1 = constrain(adj_1, 0, 255); 
  
//adjusting sensor 2
  sensor_2 = analogRead(1);
  adj_2 = map(sensor_2, s2_min, s2_max, 0, 255);
  adj_2 = constrain(adj_2, 0, 255); 
  
//adjusting sensor 3
  sensor_3 = analogRead(2);
  adj_3 = map(sensor_3, s3_min, s3_max, 0, 255);
  adj_3 = constrain(adj_3, 0, 255); 
  
//adjusting sensor 4
  sensor_4 = analogRead(3);
  adj_4 = map(sensor_4, s4_min, s4_max, 0, 255);
  adj_4 = constrain(adj_4, 0, 255); 
  
//adjusting sensor 5
  sensor_5 = analogRead(4);
  adj_5 = map(sensor_5, s5_min, s5_max, 0, 255);
  adj_5 = constrain(adj_5, 0, 255); 
}

int isFrontClear(){
  myservo.writeMicroseconds(1500);              // tell servo to go to 0 position
  return ultrasoundDistance();
}

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
//    while(moveForward()){
//      if(moveForward()){
//      }
//    }
//    if(moveLeft()){
//      //moveForward();
//    }
//    else if(moveRight()){
//      //moveForward();
//    }
//    else{
//      moveBackward();
//      moveBackward();
//      moveBackward();      
//    }
updateIRSensors();
  Serial.print("adjust_1 : "); Serial.println(adj_1);
  Serial.print("adjust_2 : "); Serial.println(adj_2);
  Serial.print("adjust_3 : "); Serial.println(adj_3);
  Serial.print("adjust_4 : "); Serial.println(adj_4);
  Serial.print("adjust_5 : "); Serial.println(adj_5);  

//    printMotorInfo();
//    if(millis()<100000){
//      moveForward();
//      delay(15000);
//      moveLeft();
//    }
  // check keyboard
  //moveForward();
//    det = check();
//    Serial.print(det);
//    while(det == 'F'){
//      Serial.print("Forward"); 
//      moveForward();
//      det = check();
//      //Serial.print(det);
//    }
//    while(det == 'B'){
//      Serial.print("Backward"); 
//      moveBackward();
//      det = check();
//      //Serial.print(det);
//    }  
}

boolean initiate(){
  count_1 = 0;
  count_2 = 0;
    if((millis()-lastMilli) >= LOOPTIME){                                    // enter tmed loop
      lastMilli = millis();
      getMotorData();
      moveConst = (moveFactor * moveTurns);      
      PWM_1= updatePid_1(PWM_1, velocity, speed_act_1);
      PWM_2= updatePid_2(PWM_2, velocity, speed_act_2);
      return true; 
    }else{
      return false; 
    }
}

boolean moveForward(){
  if (initiate() && (isFrontClear() > safeDist)){
    while(count_1 < (moveConst * magnitudeTurns)){
      analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
      digitalWrite(In1, LOW);          //motor pin for direction "In1" is provided with "LOW" input
      analogWrite(PWM2, PWM_2);
      digitalWrite(In2, LOW);
    }
    analogWrite(PWM1, 0);                    
    analogWrite(PWM2, 0);
    printMotorInfo();
    delay(timeDelay);
    return true;
  }else{
    return false;
  }  
}

void moveBackward(){
  if (initiate()){
    while(count_1 < (moveConst * magnitudeTurns)){
      analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
      digitalWrite(In1, HIGH);          //motor pin for direction "In1" is provided with "LOW" input
      analogWrite(PWM2, PWM_2);
      digitalWrite(In2, HIGH);
    }
    analogWrite(PWM1, 0);                    
    analogWrite(PWM2, 0);
    printMotorInfo();
  }   
}

boolean moveLeft(){
  if(initiate() && (isLeftClear() > safeDist)){
    while(count_1 < moveConst){
      analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
      digitalWrite(In1, HIGH);          //motor pin for direction "In1" is provided with "LOW" input
      speed_act_2 = (steeringConst * velocity);
      analogWrite(PWM2, PWM_2);
      digitalWrite(In2, LOW);
      printMotorInfo();
      speed_act_2 = 0;
    }
    analogWrite(PWM1, 0);                    
    analogWrite(PWM2, 0);
    return true;
  }else{
    return false;
  }  
}

void moveSoftLeft(){
  if(initiate() && (isFrontClear() > safeDist)){
    while(count_1 < moveConst){
      analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
      digitalWrite(In1, HIGH);          //motor pin for direction "In1" is provided with "LOW" input
      speed_act_2 = (steeringConst * velocity)/steeringReg;
      analogWrite(PWM2, PWM_2);
      digitalWrite(In2, LOW);
      printMotorInfo();
      speed_act_2 = 0;
    }
    analogWrite(PWM1, 0);                    
    analogWrite(PWM2, 0);
  }  
}

boolean moveRight(){
  if(initiate() && (isRightClear() > safeDist)){
    while(count_1 < moveConst){
      analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
      digitalWrite(In1, LOW);          //motor pin for direction "In1" is provided with "LOW" input
      speed_act_2 = (steeringConst * velocity);
      analogWrite(PWM2, PWM_2);
      digitalWrite(In2, HIGH);
      printMotorInfo();
      speed_act_2 = 0;
    }
    analogWrite(PWM1, 0);                    
    analogWrite(PWM2, 0);
    return true;
  }else{
    return false;
  }  
}

void moveSoftRight(){
  if(initiate() && (isRightClear() > safeDist)){
    while(count_1 < moveConst){
      analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
      digitalWrite(In1, LOW);          //motor pin for direction "In1" is provided with "LOW" input
      speed_act_2 = (steeringConst * velocity)/steeringReg;
      analogWrite(PWM2, PWM_2);
      digitalWrite(In2, HIGH);
      printMotorInfo();
      speed_act_2 = 0;
    }
    analogWrite(PWM1, 0);                    
    analogWrite(PWM2, 0);
  }  
}

void getMotorData()  {                                                        // calculate speed, volts and Amps
static long countAnt_1 = 0;
static long countAnt_2 = 0;
// last count
  speed_act_1 = ((count_1 - countAnt_1)*(60*(100/LOOPTIME)))/(334);          // 16 pulses X 29 gear ratio = 464 counts per output shaft rev
  countAnt_1 = count_1; 
  speed_act_2 = ((count_2 - countAnt_2)*(60*(100/LOOPTIME)))/(334);          // 16 pulses X 29 gear ratio = 464 counts per output shaft rev
  countAnt_2 = count_2;     
  //voltage = int(analogRead(Vpin) * 2.22 * 12.2/2.2);                          // battery voltage: mV=ADC*3300/1024, voltage divider 10K+2K
  //current = int(analogRead(Apin) * 2.22 * .77 *(1000.0/132.0));               // motor current - output: 130mV per Amp
  //current = digital_smooth(current, readings);                                // remove signal noise
}

int updatePid_1(int command, int targetValue, int currentValue)   {             // compute PWM value
float pidTerm = 0;                                                            // PID correction
int error=0;                                  
static int last_error_1=0;                             
  error = abs(targetValue) - abs(currentValue); 
  pidTerm = (Kp * error) + (Kd * (error - last_error_1));                            
  last_error_1 = error;
  return constrain(command + int(pidTerm), 0, 255);
}

int updatePid_2(int command, int targetValue, int currentValue)   {             // compute PWM value
float pidTerm = 0;                                                            // PID correction
int error=0;                                  
static int last_error_2=0;                             
  error = abs(targetValue) - abs(currentValue); 
  pidTerm = (Kp * error) + (Kd * (error - last_error_2));                            
  last_error_2 = error;
  return constrain(command + int(pidTerm), 0, 255);
}

void printMotorInfo()  {                                                      // display data
  if((millis()-lastMilliPrint) >= 500)   {                     
    lastMilliPrint = millis();
    Serial.print("  SP:");            Serial.println(velocity);  
    Serial.print("  RPM_1:");          Serial.println(speed_act_1);
    Serial.print("  count_1:");          Serial.println(count_1); 
    Serial.print("  RPM_2:");          Serial.println(speed_act_2);
    Serial.print("  count_2:");          Serial.println(count_2);  

    //if (current > CURRENT_LIMIT)               Serial.println("*** CURRENT_LIMIT ***");                
    //if (voltage > 1000 && voltage < LOW_BAT)   Serial.println("*** LOW_BAT ***");                
  }
}

void rencoder_1()  {                                    // pulse and direction, direct port reading to save cycles
  if (digitalRead(encodPin2)==LOW)    count_1++;                // if(digitalRead(encodPin2)==HIGH)   count ++;
  else                      count_1--;                // if (digitalRead(encodPin2)==LOW)   count --;
}

void rencoder_2()  {                                    // pulse and direction, direct port reading to save cycles
  if (digitalRead(encodPinB2)==LOW)    count_2++;                // if(digitalRead(encodPin2)==HIGH)   count ++;
  else                      count_2--;                // if (digitalRead(encodPin2)==LOW)   count --;
}

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

//int check()
//{
//  if (Serial.available() > 0)    //Check for data on the serial lines.
//  {   
//    dataIn = Serial.read();  //Get the character sent by the phone and store it in 'dataIn'.
//        if (dataIn == 'F')
//        {     
//          determinant = 'F';
//        }  
//        else if (dataIn == 'B')
//        { 
//          determinant = 'B'; 
//        }
//        else if (dataIn == 'L')  
//        { 
//          determinant = 'L';
//        }
//        else if (dataIn == 'R')  
//        { 
//          determinant = 'R';
//        } 
//        else if (dataIn == 'I')  
//        { 
//          determinant = 'I'; 
//        }  
//        else if (dataIn == 'J')  
//        {  
//          determinant = 'J';
//        }          
//        else if (dataIn == 'G') 
//        {
//          determinant = 'G'; 
//        }    
//        else if (dataIn == 'H')  
//        {
//          determinant = 'H'; 
//        }   
//        else if (dataIn == 'S') 
//        {
//          determinant = 'S';
//        }
//        else if (dataIn == '0') 
//        {
//          velocity = 0;    //"velocity" does not need to be returned.
//        }
//        else if (dataIn == '1') 
//        {
//          velocity = 25;
//        }
//        else if (dataIn == '2') 
//        {
//          velocity = 50;
//        }
//        else if (dataIn == '3') 
//        {
//          velocity = 75;
//        }
//        else if (dataIn == '4') 
//        {
//          velocity = 100;
//        }
//        else if (dataIn == '5') 
//        {
//          velocity = 125;
//        }
//        else if (dataIn == '6') 
//        {
//          velocity = 150;
//        }
//        else if (dataIn == '7') 
//        {
//          velocity = 175;
//        }
//        else if (dataIn == '8') 
//        {
//          velocity = 200;
//        }
//        else if (dataIn == '9') 
//        {
//          velocity = 225;
//        }
//        else if (dataIn == 'q') 
//        {
//          velocity = 255;
//        }
//  }
//return determinant;
//}
