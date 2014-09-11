
int power =  255;
#define FORWARD LOW
#define BACKWARD HIGH

int motorDir_A1 = 8;
int motorDir_A2 = 7;
int motorPWM_1 = 9;

int motorDir_B1 = 12;
int motorDir_B2 = 13;
int motorPWM_2 = 11;

void setup(){
  Serial.begin(9600);  
  pinMode(motorDir_A1, OUTPUT);
  pinMode(motorDir_A2, OUTPUT);
  pinMode(motorPWM_1, OUTPUT);
  pinMode(motorDir_B1, OUTPUT);
  pinMode(motorDir_B2, OUTPUT);
  pinMode(motorPWM_2, OUTPUT); 
}

void loop(){
  moveForward();
  delay(1000);
  brake();
  moveBackward();
  delay(1000);
  brake();
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


void moveForward(){      
  digitalWrite(motorDir_A1, FORWARD);
  digitalWrite(motorDir_A2, BACKWARD);
  analogWrite(motorPWM_1, power);

  digitalWrite(motorDir_B1, FORWARD);
  digitalWrite(motorDir_B2, BACKWARD);
  analogWrite(motorPWM_2, power);  
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

