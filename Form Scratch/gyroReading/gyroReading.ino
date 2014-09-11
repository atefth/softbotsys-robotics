int xRead;
int xRest;
double Gx;

int sensorPin = 0;

void setup(){
  
 Serial.begin(9600);
 delay(100); 
 xRest = analogRead(sensorPin);
 Serial.print(xRest);
 
}

void loop(){
 xRead = analogRead(sensorPin) - xRest;
 Gx  = xRead / 67.584;
 Serial.print("Gyro X: ");
 Serial.println(Gx);
 delay(500);
}
