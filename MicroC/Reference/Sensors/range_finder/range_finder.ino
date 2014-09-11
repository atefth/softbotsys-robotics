#include <Servo.h>                                  //includes the servo library

int M1 = 4;
int E1 = 5;
int E2 = 6;
int M2 = 7;
int servopin = 8;
//int sensorpin = 0;
#define trigPin 13
#define echoPin 12
int dist = 0;
int leftdist = 0;
int rightdist = 0;
int object = 10;             //distance at which the robot should look for another route                           

Servo myservo;

void setup ()
{
  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(M1,OUTPUT);
  pinMode(E1,OUTPUT);
  pinMode(E2,OUTPUT);
  pinMode(M2,OUTPUT);
  myservo.attach(servopin);
  myservo.write(90);
  delay(700);
}
void loop(){
   long duration, distance;
   digitalWrite(trigPin, LOW);
   delayMicroseconds(2);
   digitalWrite(trigPin, HIGH);
   delayMicroseconds(10);
   digitalWrite(trigPin, LOW);
   duration =pulseIn(echoPin, HIGH);
   dist = (duration/2)/29.1;
   //dist = analogRead(sensorpin);               //reads the sensor

  if(dist < object) {                         //if distance is less than 550
   forward();                                  //then move forward
  }
  if(dist >= object) {               //if distance is greater than or equal to 550
    findroute();
    
  }
  Serial.print(dist);
  Serial.println("cm");
  delay(500);
}
 
void forward() {                            // use combination which works for you
   digitalWrite(M1,LOW);   
   digitalWrite(M2, LOW);       
   analogWrite(E1, 238);   //PWM Speed Control
   analogWrite(E2, 255);   //PWM Speed Control
   return;
 }
 
void findroute() {
  halt();                                             // stop
  backward();                                       //go backwards
  lookleft();                                      //go to subroutine lookleft
  lookright();                                   //go to subroutine lookright
                                      
  if ( leftdist < rightdist )
  {
    turnleft();
  }
 else
 {
   turnright ();
 }
}

void backward() {
  digitalWrite(M1,HIGH);   
  digitalWrite(M2, HIGH);       
  analogWrite(E1, 238);   //PWM Speed Control
  analogWrite(E2, 255);
  delay(500);
  halt();
  return;
}

void halt () {
  digitalWrite(M1,LOW);   
  digitalWrite(M2, LOW);       
  analogWrite(E1, 0);   //PWM Speed Control
  analogWrite(E2, 0);   //PWM Speed Control
  delay(500);                          //wait after stopping
  return;
}
 
void lookleft() {
  myservo.write(150);
  delay(700);                                //wait for the servo to get there
  leftdist = dist;
  myservo.write(90);
  delay(700);                                 //wait for the servo to get there
  return;
}

void lookright () {
  myservo.write(30);
  delay(700);                           //wait for the servo to get there
  rightdist = dist;
  myservo.write(90);                                  
  delay(700);                        //wait for the servo to get there
  return;
}

void turnleft () {
  digitalWrite(M1,HIGH);   
  digitalWrite(M2, LOW);       
  analogWrite(E1, 200);   //PWM Speed Control
  analogWrite(E2, 200);   //PWM Speed Control
  delay(1000);                     // wait for the robot to make the turn
  halt();
  return;
}

void turnright () {
  digitalWrite(M1,LOW);   
  digitalWrite(M2, HIGH);       
  analogWrite(E1, 200);   //PWM Speed Control
  analogWrite(E2, 200);   //PWM Speed Control
  delay(1000);                              // wait for the robot to make the turn
  halt();
  return;
}
