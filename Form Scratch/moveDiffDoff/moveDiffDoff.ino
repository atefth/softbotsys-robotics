//#include <Servo.h>

int power =  255;
#define FORWARD LOW
#define BACKWARD HIGH

int motorDir_A1 = 8;
int motorDir_A2 = 7;
int motorPWM_1 = 9;

//Servo leftServo;
//Servo rightServo;

int motorDir_B1 = 12;
int motorDir_B2 = 13;
int motorPWM_2 = 11;

long revolutionTime = 406480;

long minDistance = 3.999999969;

int steering = 1;

long last_micros = 0;
long last_lift_micros = 0;
long movementTime = 0;

long turningTime = 1385480;

int forkliftAngle = 45;

long distanceToMove = 30;

void setup(){
  Serial.begin(9600);  
  pinMode(motorDir_A1, OUTPUT);
  pinMode(motorDir_A2, OUTPUT);
  pinMode(motorPWM_1, OUTPUT);
  pinMode(motorDir_B1, OUTPUT);
  pinMode(motorDir_B2, OUTPUT);
  pinMode(motorPWM_2, OUTPUT); 
  last_micros = revolutionTime;
  movementTime = timeToMove(distanceToMove);
  //leftServo.attach(5);
  //rightServo.attach(6);
}

void loop(){
  //  leftServo.write(0);
  //  rightServo.write(0);
  last_micros = micros();
  //  movementTime = timeToMove(25);
  //  MoveLiftUp();
  //  delay(1000);
  //  MoveLiftDown();
  moveForward();
  //  rectangle();
  //  turnLeft();
  delay(1000);

  //  movementTime = timeToMove(25);
  //  moveForward();
  //  delay(1000);
  //  pickObject();
  //  delay(1000);
  //  movementTime = timeToMove(25);
  //  moveForward();
  //  delay(1000);
  //  MoveLiftDown();
  //  delay(1000);
}

void accelerate(){
  power = 0;
  for(int i = 0; i < 255; i++){
    digitalWrite(motorDir_A1, FORWARD);
    digitalWrite(motorDir_A2, BACKWARD);
    analogWrite(motorPWM_1, power + i);

    digitalWrite(motorDir_B1, FORWARD);
    digitalWrite(motorDir_B2, BACKWARD);
    analogWrite(motorPWM_2, power + i);  
    delay(20);
  }
  for(int i = 0; i < 255; i++){
    digitalWrite(motorDir_A1, BACKWARD);
    digitalWrite(motorDir_A2, FORWARD);
    analogWrite(motorPWM_1, power + i);

    digitalWrite(motorDir_B1, BACKWARD);
    digitalWrite(motorDir_B2, FORWARD);
    analogWrite(motorPWM_2, power + i);
    delay(20);
  }
}

void rectangle(){
  moveForward();
  last_micros = micros();
  turnLeft();
  last_micros = micros();
  moveForward();
  last_micros = micros();
  turnLeft();
  last_micros = micros();
  moveForward();
  last_micros = micros();
  turnLeft();
  last_micros = micros();
  moveForward();
  last_micros = micros();
  turnLeft();  
}

//void pickObject(){
//  last_lift_micros = micros();
//  moveForward(10000);
//  delay(500);
//  MoveLiftUp(); 
//}
//
//void MoveLiftDown(){
//  for (int i = 0; i < forkliftAngle; i++){
//    leftServo.write(i);
//    rightServo.write(-i);
//    //delay(10);
//  } 
//}
//
//void MoveLiftUp(){
//  for (int i = 0; i > -forkliftAngle; i--){
//    leftServo.write(i);
//    rightServo.write(-i);
//    //delay(10);
//  } 
//}

void printTimeData(){
  Serial.print("last micros : ");
  Serial.print(last_micros);
  Serial.print(" ---- current micros : ");
  Serial.print(micros());
  Serial.print(" ---- time moved : ");
  Serial.print(micros() - last_micros);  
  Serial.print(" ---- distance moved : ");
  Serial.print(getDistanceMoved());
  Serial.println();  
}

long timeToMove(int distanceToMove){
  return ((distanceToMove/minDistance) * revolutionTime);
}

boolean isTime(){
  if (micros() - last_micros < movementTime){
    return false; 
  }
  else{
    return true; 
  }
}

int getDistanceMoved(){
  return (((micros()-last_micros)/revolutionTime) * minDistance);
}

void moveForward(){
  while(!isTime()){      
    digitalWrite(motorDir_A1, FORWARD);
    digitalWrite(motorDir_A2, BACKWARD);
    analogWrite(motorPWM_1, power);

    digitalWrite(motorDir_B1, FORWARD);
    digitalWrite(motorDir_B2, BACKWARD);
    analogWrite(motorPWM_2, power);  
    printTimeData();
  }
  analogWrite(motorPWM_1, 0);
  analogWrite(motorPWM_2, 0);
}

boolean isTimeSpan(long span){
  if(micros() - last_lift_micros < span){
    return false;
  }
  else{
    return true;
  } 
}

void moveForward(long span){
  while(!isTimeSpan(span)){      
    digitalWrite(motorDir_A1, FORWARD);
    digitalWrite(motorDir_A2, BACKWARD);
    analogWrite(motorPWM_1, power);

    digitalWrite(motorDir_B1, FORWARD);
    digitalWrite(motorDir_B2, BACKWARD);
    analogWrite(motorPWM_2, power);  
    printTimeData();
  }
  analogWrite(motorPWM_1, 0);
  analogWrite(motorPWM_2, 0);
}

//void moveForwardLift(){
//  while(!isTime()){      
//    digitalWrite(motorDir_A1, FORWARD);
//    digitalWrite(motorDir_A2, BACKWARD);
//    analogWrite(motorPWM_1, power);
//
//    digitalWrite(motorDir_B1, FORWARD);
//    digitalWrite(motorDir_B2, BACKWARD);
//    analogWrite(motorPWM_2, power);  
//    MoveLiftUp();
//    MoveLiftDown();
//    printTimeData();
//  }
//  analogWrite(motorPWM_1, 0);
//  analogWrite(motorPWM_2, 0);
//}

void moveBackward(){
  while(micros()%revolutionTime != 0){  
    digitalWrite(motorDir_A1, BACKWARD);
    digitalWrite(motorDir_A2, FORWARD);
    analogWrite(motorPWM_1, power);

    digitalWrite(motorDir_B1, BACKWARD);
    digitalWrite(motorDir_B2, FORWARD);
    analogWrite(motorPWM_2, power);
    printTimeData();
  }
}

boolean isTurnTime(){
  if (micros() - last_micros < turningTime){
    return false; 
  }
  else{
    return true; 
  }
}

void turnLeft(){
  while(!isTurnTime()){      
    Serial.print("here");
    digitalWrite(motorDir_A1, FORWARD);
    digitalWrite(motorDir_A2, BACKWARD);
    analogWrite(motorPWM_1, power);

    digitalWrite(motorDir_B1, BACKWARD);
    digitalWrite(motorDir_B2, FORWARD);
    analogWrite(motorPWM_2, power);  
    printTimeData();
  }
  analogWrite(motorPWM_1, 0);
  analogWrite(motorPWM_2, 0);
}

void turnRight(){
  while(!isTurnTime()){      
    digitalWrite(motorDir_A1, BACKWARD);
    digitalWrite(motorDir_A2, FORWARD);
    analogWrite(motorPWM_1, power);

    digitalWrite(motorDir_B1, FORWARD);
    digitalWrite(motorDir_B2, BACKWARD);
    analogWrite(motorPWM_2, power);  
    printTimeData();
  }
  analogWrite(motorPWM_1, 0);
  analogWrite(motorPWM_2, 0);
}

