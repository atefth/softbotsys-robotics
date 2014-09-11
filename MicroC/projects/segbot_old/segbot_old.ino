// Test MD03a / Pololu motor with encoder
// speed control (PI), V & I display
// Credits:
//   Dallaby   http://letsmakerobots.com/node/19558#comment-49685
//   Bill Porter  http://www.billporter.info/?p=286
//   bobbyorr (nice connection diagram) http://forum.pololu.com/viewtopic.php?f=15&t=1923


#define InA1            4                      // INA motor pin
#define InB1            7                      // INB motor pin 
#define PWM1            5                       // PWM motor pin
#define PWM2            6
#define encodPinA1      2                       // encoder A pin
#define encodPinB1      3                       // encoder B pin
#define encodPinA2      19
#define encodPinB2      18

#define CURRENT_LIMIT   1000                     // high current warning
#define LOW_BAT         10000                   // low bat warning
#define LOOPTIME        100                     // PID loop time
#define NUMREADINGS     10                      // samples for Amp average

int readings[NUMREADINGS];
unsigned long lastMilli = 0;                    // loop timing 
unsigned long lastMilliPrint = 0;               // loop timing
static int velocity = 0;                            // speed (Set Point)
int speed_act_1 = 0;
int speed_act_2 = 0;// speed (actual value)
int PWM_1 = 0;
int PWM_2 = 0;// (25% = 64; 50% = 127; 75% = 191; 100% = 255)
int voltage = 0;                                // in mV
int current = 0;                                // in mA
volatile long count_1 = 0;
volatile long count_2 = 0;// rev counter
float Kp =   .5;                                // PID proportional control Gain
float Kd =    1;                                // PID Derivitave control gain

//for bluetooth
static char determinant;         //Used in the check function, stores the character received from the phone.
char det;                 //Used in the loop function, stores the character received from the phone.
char dataIn = 'S';        //Character/Data coming from the phone.

void setup() {
  Serial.begin(115200);
  //analogReference(EXTERNAL);                            // Current external ref is 3.3V
  
  pinMode(InA1, OUTPUT);
  pinMode(PWM1, OUTPUT);
  
  pinMode(InB1, OUTPUT);
  pinMode(PWM2, OUTPUT);
  pinMode(encodPinA1, INPUT); 
  pinMode(encodPinB1, INPUT); 
  digitalWrite(encodPinA1, HIGH);                      // turn on pullup resistor
  digitalWrite(encodPinB1, HIGH);
  pinMode(encodPinA2, INPUT); 
  pinMode(encodPinB2, INPUT); 
  digitalWrite(encodPinA2, HIGH);                      // turn on pullup resistor
  digitalWrite(encodPinB2, HIGH);
  attachInterrupt(1, rencoder_1, FALLING);
  attachInterrupt(5, rencoder_2, FALLING);
  for(int i=0; i<NUMREADINGS; i++)   readings[i] = 0;  // initialize readings to 0

  
}

void loop() {                                                                // check keyboard
  //moveForward();
    det = check();
    Serial.print(det);
    while(det == 'F'){
      Serial.print("Forward"); 
      moveForward();
      det = check();
      //Serial.print(det);
    }
    while(det == 'B'){
      Serial.print("Backward"); 
      moveBackward();
      det = check();
      //Serial.print(det);
    }  
}

void initiate(){
    if((millis()-lastMilli) >= LOOPTIME){                                    // enter tmed loop
    lastMilli = millis();
    getMotorData();                                           
    PWM_1= updatePid_1(PWM_1, velocity, speed_act_1);
    PWM_2= updatePid_2(PWM_2, velocity, speed_act_2); 
    }
}

void moveForward(){
  initiate();
  analogWrite(PWM1, PWM_1);          //giving motor required power "PWM_1" which is on pin "PWM1"
  digitalWrite(InA1, LOW);          //motor pin for direction "InA1" is provided with "LOW" input
  analogWrite(PWM2, PWM_2);
  digitalWrite(InB1, LOW);
  printMotorInfo();  
}

void moveBackward(){
  initiate();
  analogWrite(PWM1, PWM_1);
  analogWrite(PWM2, PWM_2);
  digitalWrite(InA1, HIGH);
  digitalWrite(InB1, HIGH);
  printMotorInfo();  
}

void getMotorData()  {                                                        // calculate speed, volts and Amps
static long countAnt_1 = 0;
static long countAnt_2 = 0;
// last count
  speed_act_1 = ((count_1 - countAnt_1)*(60*(100/LOOPTIME)))/(334);          // 16 pulses X 29 gear ratio = 464 counts per output shaft rev
  countAnt_1 = count_1; 
speed_act_2 = ((count_2 - countAnt_2)*(60*(100/LOOPTIME)))/(334);          // 16 pulses X 29 gear ratio = 464 counts per output shaft rev
  countAnt_2 = count_2;     
  //voltage = int(analogRead(Vpin) * 2.22 * 12.2/2.2);                          // battery voltage: mV=ADC*3300/1024, voltage divider 10K+2K
  //current = int(analogRead(Apin) * 2.22 * .77 *(1000.0/132.0));               // motor current - output: 130mV per Amp
  //current = digital_smooth(current, readings);                                // remove signal noise
}

int updatePid_1(int command, int targetValue, int currentValue)   {             // compute PWM value
float pidTerm = 0;                                                            // PID correction
int error=0;                                  
static int last_error_1=0;                             
  error = abs(targetValue) - abs(currentValue); 
  pidTerm = (Kp * error) + (Kd * (error - last_error_1));                            
  last_error_1 = error;
  return constrain(command + int(pidTerm), 0, 255);
}

int updatePid_2(int command, int targetValue, int currentValue)   {             // compute PWM value
float pidTerm = 0;                                                            // PID correction
int error=0;                                  
static int last_error_2=0;                             
  error = abs(targetValue) - abs(currentValue); 
  pidTerm = (Kp * error) + (Kd * (error - last_error_2));                            
  last_error_2 = error;
  return constrain(command + int(pidTerm), 0, 255);
}

void printMotorInfo()  {                                                      // display data
  if((millis()-lastMilliPrint) >= 500)   {                     
    lastMilliPrint = millis();
    Serial.print("SP:");             Serial.print(velocity);  
    Serial.print("  RPM_1:");          Serial.print(speed_act_1);
    Serial.print("  PWM_1:");          Serial.print(PWM_1); 
       Serial.print("  RPM_2:");          Serial.print(speed_act_2);
    Serial.print("  PWM_2:");          Serial.print(PWM_2);  
    Serial.print("  V:");            Serial.print(float(voltage)/1000,1);
    Serial.print("  mA:");           Serial.println(current);

    if (current > CURRENT_LIMIT)               Serial.println("*** CURRENT_LIMIT ***");                
    if (voltage > 1000 && voltage < LOW_BAT)   Serial.println("*** LOW_BAT ***");                
  }
}

void rencoder_1()  {                                    // pulse and direction, direct port reading to save cycles
  if (digitalRead(encodPinB1)==LOW)    count_1++;                // if(digitalRead(encodPinB1)==HIGH)   count ++;
  else                      count_1--;                // if (digitalRead(encodPinB1)==LOW)   count --;
}

void rencoder_2()  {                                    // pulse and direction, direct port reading to save cycles
  if (digitalRead(encodPinB2)==LOW)    count_2++;                // if(digitalRead(encodPinB1)==HIGH)   count ++;
  else                      count_2--;                // if (digitalRead(encodPinB1)==LOW)   count --;
}

int digital_smooth(int value, int *data_array)  {    // remove signal noise
static int ndx=0;                                                         
static int count=0;                          
static int total=0;                          
  total -= data_array[ndx];               
  data_array[ndx] = value;                
  total += data_array[ndx];               
  ndx = (ndx+1) % NUMREADINGS;                                
  if(count < NUMREADINGS)      count++;
  return total/count;
}

int check()
{
  if (Serial.available() > 0)    //Check for data on the serial lines.
  {   
    dataIn = Serial.read();  //Get the character sent by the phone and store it in 'dataIn'.
        if (dataIn == 'F')
        {     
          determinant = 'F';
        }  
        else if (dataIn == 'B')
        { 
          determinant = 'B'; 
        }
        else if (dataIn == 'L')  
        { 
          determinant = 'L';
        }
        else if (dataIn == 'R')  
        { 
          determinant = 'R';
        } 
        else if (dataIn == 'I')  
        { 
          determinant = 'I'; 
        }  
        else if (dataIn == 'J')  
        {  
          determinant = 'J';
        }          
        else if (dataIn == 'G') 
        {
          determinant = 'G'; 
        }    
        else if (dataIn == 'H')  
        {
          determinant = 'H'; 
        }   
        else if (dataIn == 'S') 
        {
          determinant = 'S';
        }
        else if (dataIn == '0') 
        {
          velocity = 0;    //"velocity" does not need to be returned.
        }
        else if (dataIn == '1') 
        {
          velocity = 25;
        }
        else if (dataIn == '2') 
        {
          velocity = 50;
        }
        else if (dataIn == '3') 
        {
          velocity = 75;
        }
        else if (dataIn == '4') 
        {
          velocity = 100;
        }
        else if (dataIn == '5') 
        {
          velocity = 125;
        }
        else if (dataIn == '6') 
        {
          velocity = 150;
        }
        else if (dataIn == '7') 
        {
          velocity = 175;
        }
        else if (dataIn == '8') 
        {
          velocity = 200;
        }
        else if (dataIn == '9') 
        {
          velocity = 225;
        }
        else if (dataIn == 'q') 
        {
          velocity = 255;
        }
  }
return determinant;
}
