int steering = 1;

void moveForward(){
  digitalWrite(motorDir_A1, FORWARD);
  digitalWrite(motorDir_A2, BACKWARD);
  analogWrite(motorPWM_1, power);
  
  digitalWrite(motorDir_B1, FORWARD);
  digitalWrite(motorDir_B2, BACKWARD);
  analogWrite(motorPWM_2, power);  
  
  delay(timePeriod);
  brake();
}

void moveBackward(){
  digitalWrite(motorDir_A1, BACKWARD);
  digitalWrite(motorDir_A2, FORWARD);
  analogWrite(motorPWM_1, power);
  
  digitalWrite(motorDir_B1, BACKWARD);
  digitalWrite(motorDir_B2, FORWARD);
  analogWrite(motorPWM_2, power);
  
  delay(timePeriod);
  brake();
}

void brake(){
 digitalWrite(motorDir_A1, FORWARD); 
 digitalWrite(motorDir_A2, FORWARD); 
 
  digitalWrite(motorDir_B1, FORWARD); 
 digitalWrite(motorDir_B2, FORWARD); 
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
