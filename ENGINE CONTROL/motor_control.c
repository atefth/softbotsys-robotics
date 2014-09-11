#define FORWARD LOW
#define BACKWARD HIGH

int power =  255;

int motorDir_A1 = 7;
int motorDir_A2 = 4;
int motorPWM_1 = 6;

int motorDir_B1 = 12;
int motorDir_B2 = 13;
int motorPWM_2 = 5;

void moveForward(){	
	digitalWrite(motorDir_A1, FORWARD);
    digitalWrite(motorDir_A2, BACKWARD);
    analogWrite(motorPWM_1, power);

    digitalWrite(motorDir_B1, FORWARD);
    digitalWrite(motorDir_B2, BACKWARD);
    analogWrite(motorPWM_2, power);    
}

void moveForward(int pwm){	
	digitalWrite(motorDir_A1, FORWARD);
    digitalWrite(motorDir_A2, BACKWARD);
    analogWrite(motorPWM_1, pwm);

    digitalWrite(motorDir_B1, FORWARD);
    digitalWrite(motorDir_B2, BACKWARD);
    analogWrite(motorPWM_2, pwm);    
}

void brake(){
	analogWrite(motorPWM_1, 0);
	analogWrite(motorPWM_2, 0);
}

void moveBackward(){
	digitalWrite(motorDir_A1, BACKWARD);
    digitalWrite(motorDir_A2, FORWARD);
    analogWrite(motorPWM_1, power);

    digitalWrite(motorDir_B1, BACKWARD);
    digitalWrite(motorDir_B2, FORWARD);
    analogWrite(motorPWM_2, power);	
}

void moveBackward(int pwm){
	digitalWrite(motorDir_A1, BACKWARD);
    digitalWrite(motorDir_A2, FORWARD);
    analogWrite(motorPWM_1, pwm);

    digitalWrite(motorDir_B1, BACKWARD);
    digitalWrite(motorDir_B2, FORWARD);
    analogWrite(motorPWM_2, pwm);	
}

void moveLeft(){
	digitalWrite(motorDir_A1, FORWARD);
    digitalWrite(motorDir_A2, BACKWARD);
    analogWrite(motorPWM_1, power);

    digitalWrite(motorDir_B1, BACKWARD);
    digitalWrite(motorDir_B2, FORWARD);
    analogWrite(motorPWM_2, power);	
}

void moveLeft(int pwm){
	digitalWrite(motorDir_A1, FORWARD);
    digitalWrite(motorDir_A2, BACKWARD);
    analogWrite(motorPWM_1, pwm);

    digitalWrite(motorDir_B1, BACKWARD);
    digitalWrite(motorDir_B2, FORWARD);
    analogWrite(motorPWM_2, pwm);	
}

void moveRight(){
	digitalWrite(motorDir_A1, BACKWARD);
    digitalWrite(motorDir_A2, FORWARD);
    analogWrite(motorPWM_1, power);

    digitalWrite(motorDir_B1, FORWARD);
    digitalWrite(motorDir_B2, BACKWARD);
    analogWrite(motorPWM_2, power);	
}

void moveRight(int pwm){
	digitalWrite(motorDir_A1, BACKWARD);
    digitalWrite(motorDir_A2, FORWARD);
    analogWrite(motorPWM_1, pwm);

    digitalWrite(motorDir_B1, FORWARD);
    digitalWrite(motorDir_B2, BACKWARD);
    analogWrite(motorPWM_2, pwm);	
}