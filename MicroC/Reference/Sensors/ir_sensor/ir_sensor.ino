int E1 = 5;  
int M1 = 4; 
int E2 = 6;                      
int M2 = 7;

int left_sensor = 0;
int adj_1 = 0;
int s1_min = 60;
int s1_max = 450;

int right_sensor = 0;
int adj_2 = 0;
int s2_min = 85;
int s2_max = 600;

int threshold = 100;

int speed_value = 255;

void setup ()
{
  Serial.begin(9600);
   pinMode(M1, OUTPUT);   
    pinMode(M2, OUTPUT); 
  delay(700);
}
void update_sensors(){
    left_sensor = analogRead(0);
    adj_1 = map(left_sensor, s1_min, s1_max, 0, 255);
    adj_1 = constrain(adj_1, 0, 255);
    
    right_sensor = analogRead(1); 
    adj_2 = map(right_sensor, s2_min, s2_max, 0, 255);
    adj_2 = constrain(adj_2, 0, 255);
}
 
  void forward() { 
    digitalWrite(M1,LOW);   
    digitalWrite(M2, LOW);       
    analogWrite(E1, 238);   //PWM Speed Control
    analogWrite(E2, 255);   //PWM Speed Control
    delay(30);     
   return;
 }
 void halt () {
    digitalWrite(M1,LOW);   
    digitalWrite(M2, LOW);       
    analogWrite(E1, 0);   //PWM Speed Control
    analogWrite(E2, 0);   //PWM Speed Control
    delay(30);                           //wait after stopping
  return;
 }
 void turnleft () {
    digitalWrite(M1,HIGH);   
    digitalWrite(M2, LOW);       
    analogWrite(E1, 200);   //PWM Speed Control
    analogWrite(E2, 200);   //PWM Speed Control
    delay(30); 
  //halt();
  return;
}

void turnright () {
   digitalWrite(M1,LOW);   
    digitalWrite(M2, HIGH);       
    analogWrite(E1, 200);   //PWM Speed Control
    analogWrite(E2, 200);   //PWM Speed Control
    delay(30); 
  //halt();
  return;
}
void loop(){
  update_sensors();
  if (adj_2 > threshold && adj_1 > threshold){
  forward();
  }
  else if (adj_2 < threshold && adj_1 < threshold){
    halt();
  }
  else if (adj_1 > adj_2){
    turnleft();
  }
  else if (adj_2 > adj_1){
    turnright();
  }
Serial.print("left_sensor: ");
Serial.print(left_sensor);
Serial.print(" - ");
Serial.print("Adj 1: ");
Serial.print(adj_1);
Serial.print(" - ");
/////sensor 2 values
Serial.print("right_sensor: ");
Serial.print(right_sensor);
Serial.print(" - ");
Serial.print("Adj 2: ");
Serial.print(adj_2);
Serial.print(" - ");
Serial.println(" ");
delay(500);
}
