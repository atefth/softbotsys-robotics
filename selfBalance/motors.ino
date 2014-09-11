//#define motor1ForwardDir HIGH
//#define motor1BackwardDir LOW
//#define motor2ForwardDir LOW
//#define motor2BackwardDir HIGH
//#define maxSpeed 100
//#define speedMultiplier 1
//
//volatile long leftMotorPosition  = 0;
//volatile long rightMotorPosition = 0;
//long lastLeftMotorPosition  = 0;
//long lastRightMotorPosition  = 0;
//int leftMotorSpeedEncoder;
//int rightMotorSpeedEncoder;
//
//int leftMotorSpeed;
//int rightMotorSpeed;
//
//int loopCounter = 0;
//int lastSpeedUpdate = 0;
//int lastDebugEncoders = 0;

void updateMotorStatuses() {

  motorSpeed = (leftMotorSpeed+rightMotorSpeed)/20;
  //speedPIDInput = smooth(speedFIR.process(motorSpeed));
  speedPIDInput = speedMovingAvarageFilter.process(speedFIR.process(motorSpeed));

  /*leftMotorPIDInput = leftMotorSpeed;
  rightMotorPIDInput = rightMotorSpeed;*/
}

void stopMotors() {
  moveMotor(1, 127); // höger
  moveMotor(2, 127); // vänster
}
void moveMotor(int motor, double speed) { // speed is a value in percentage 0-100%
  // motor1 is right
  // motor2 is left

  if(speed > maxSpeed)
    speed = maxSpeed;
  else if (speed < -maxSpeed)
    speed = -maxSpeed;

  int pwmToMotor = 0;
  if (motor == 1)
    pwmToMotor = (abs(speed)*255/100)*motor1Calibration;
  else
    pwmToMotor = (abs(speed)*255/100)*motor2Calibration;
  if (pwmToMotor > 255)
    pwmToMotor = 255;

  if (motor == 1) {
    if (speed == 0) {
      digitalWrite(dir_a, motor1ForwardDir);
      analogWrite(pwm_a, 0);
    }
    else if (speed < 0) {
      digitalWrite(dir_a, motor1ForwardDir);
      analogWrite(pwm_a, pwmToMotor);
    }
    else {
      digitalWrite(dir_a, motor1BackwardDir);
      analogWrite(pwm_a, pwmToMotor);             
    }
  }
  else {
    if (speed == 0) {
      digitalWrite(dir_b, motor2ForwardDir);
      analogWrite(pwm_b, 0);
    }
    else if (speed < 0) {
      digitalWrite(dir_b, motor2ForwardDir);
      analogWrite(pwm_b, pwmToMotor);
    }
    else {
      digitalWrite(dir_b, motor2BackwardDir);
      analogWrite(pwm_b, pwmToMotor);
    }
  }
}

void debugEncoders() {
  //Serial.print(leftMotorPosition);
  //Serial.print(" ");
//  Serial.print(leftMotorSpeed*speedMultiplier);
//  Serial.print(" ");
    //Serial.print(rightMotorPosition);
    //Serial.print(" ");
//  Serial.print(rightMotorSpeed*speedMultiplier);
//  Serial.print(" ");
//  Serial.print(anglePIDInput);
//  Serial.println(" ");
//    Serial.print(motorSpeed);
//    Serial.print(" ");
    //Serial.print(leftMotorSpeed);
    //Serial.print(" ");
    //Serial.print(rightMotorSpeed);
    //Serial.println(" ");
}

//void updateMotorSpeeds() {
//  leftMotorSpeed = leftMotorPosition - lastLeftMotorPosition;
//  //rightMotorSpeed = rightMotorPosition - lastRightMotorPosition;
//  lastLeftMotorPosition = leftMotorPosition;
//  //lastRightMotorPosition = rightMotorPosition; 
// //return leftMotorSpeed1;
//}
//double updateMotorSpeedsRight() {
//  //leftMotorSpeed = leftMotorPosition - lastLeftMotorPosition;
//  rightMotorSpeed2 = rightMotorPosition - lastRightMotorPosition;
//  //lastLeftMotorPosition = leftMotorPosition;
//  lastRightMotorPosition = rightMotorPosition; 
// return rightMotorSpeed2; 
//}
void updateMotorSpeeds() {
  leftMotorSpeedEncoder = leftMotorPosition - lastLeftMotorPosition;
  rightMotorSpeedEncoder = rightMotorPosition - lastRightMotorPosition;
  lastLeftMotorPosition = leftMotorPosition;
  lastRightMotorPosition = rightMotorPosition; 
 }

void leftEncoder() { 
  if(digitalRead(leftEncoder2)==LOW) // read pin 4
      leftMotorPosition++;
  else
    leftMotorPosition--;    
}
void rightEncoder() {
  if(digitalRead(rightEncoder2)==LOW) // read pin 5
      rightMotorPosition++;
  else
    rightMotorPosition--;  
}
