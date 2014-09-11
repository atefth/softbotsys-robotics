void setup(){
  Serial.begin(9600);  
  pinMode(motorDir_A1, OUTPUT);
  pinMode(motorDir_A2, OUTPUT);
  pinMode(motorPWM_1, OUTPUT);
  pinMode(motorDir_B1, OUTPUT);
  pinMode(motorDir_B2, OUTPUT);
  pinMode(motorPWM_2, OUTPUT); 
  last_micros = revolutionTime;
  movementTime = timeToMove(25);
  leftServo.attach(5);
  rightServo.attach(6);
}

void loop(){
  //  leftServo.write(0);
  //  rightServo.write(0);
  last_micros = micros();
  //movementTime = timeToMove(25);
  //  MoveLiftUp();
  //  delay(1000);
  //  MoveLiftDown();
  //goForward();
  accelerate();
  delay(1000);

  //  movementTime = timeToMove(25);
  //  goForward();
  //  delay(1000);
  //  pickObject();
  //  delay(1000);
  //  movementTime = timeToMove(25);
  //  goForward();
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
  goForward();
  last_micros = micros();
  turnLeft();
  last_micros = micros();
  goForward();
  last_micros = micros();
  turnLeft();
  last_micros = micros();
  goForward();
  last_micros = micros();
  turnLeft();
  last_micros = micros();
  goForward();
  last_micros = micros();
  turnLeft();  
}

//void pickObject(){
//  last_lift_micros = micros();
//  goForward(10000);
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

//void goForwardLift(){
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