#define power 255
#define FORWARD LOW
#define BACKWARD HIGH

int motorDir_1 = 5;
int motorPWM_1 = 9;

int motorDir_2 = 6;
int motorPWM_2 = 10;

int steering = 1;

void setup(){  
 pinMode(motorDir_1, OUTPUT);
 pinMode(motorPWM_1, OUTPUT);
 pinMode(motorDir_2, OUTPUT);
 pinMode(motorPWM_2, OUTPUT); 
}

void loop(){
  moveForward();
  delay(500);
  moveBackward();
  delay(500);
}

void moveForward(){
  digitalWrite(motorDir_1, FORWARD);
  analogWrite(motorPWM_1, power);
  
  digitalWrite(motorDir_2, FORWARD);
  analogWrite(motorPWM_2, power);  
}

void moveBackward(){
  digitalWrite(motorDir_1, BACKWARD);
  analogWrite(motorPWM_1, power);
  
  digitalWrite(motorDir_2, BACKWARD);
  analogWrite(motorPWM_2, power);
}

//void moveLeft(){
//  digitalWrite(motorDir_1, FORWARD);
//  analogWrite(motorPWM_1, power);
//  
//  digitalWrite(motorDir_2, BACKWARD);
//  analogWrite(motorPWM_2, power * steering);
//}
//
//void moveRight(){
//  digitalWrite(motorDir_1, BACKWARD);
//  analogWrite(motorPWM_1, power * 1);
//  
//  digitalWrite(motorDir_2, FORWARD);
//  analogWrite(motorPWM_2, power);  
//}
