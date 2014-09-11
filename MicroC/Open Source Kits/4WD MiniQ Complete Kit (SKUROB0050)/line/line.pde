// DFRobot.com
// MiniQ Sample Code
// Line tracking
// Version 1.0
// Last updated on 10th March 2011



#define EN1 6 // Right Motor Enable Pin
#define IN1 7 // Right Motor Direction Pin
#define EN2 5 // Left Motor Enable Pin
#define IN2 4 // Left Motor Direction Pin

#define FORW 1 // Forward
#define BACK 0 // Backward



#define BUZZER 11 // control the number of IO pins buzzer
#define LED_RED 12 // red LED lights control digital IO pins
#define LED_GREEN 13 // green LED lights control digital IO pins
#define Vr 5 // reference voltage

float data [8] = {
  0X00, 0X00, 0X00, 0X00, 0x00, 0xff, 0x00, 0x00}; // store the 8-channel ad conversion value
unsigned char value [5] = {
  0x00, 0x00, 0x00, 0x00, 0x00}; // five hunt voltage sensor
unsigned char key_1 = 0x00; // number of keys pressed to count 1


void Motor_Control (int M1_DIR, int M1_EN, int M2_DIR, int M2_EN) // Motor control function
{
  ////////// M1 ////////////////////////
  if (M1_DIR == FORW) // Set M1 Motor Direction
    digitalWrite (IN1, HIGH); // HIGH, Rotate clockwise
  else
    digitalWrite (IN1, LOW); // LOW, Anti Rotate clockwise
  if (M1_EN == 0) // M1 Motor Speed
    analogWrite (EN1, LOW); // Low to stop miniQ
  else
    analogWrite (EN1, M1_EN);

  /////////// M2 //////////////////////
  if (M2_DIR == FORW) // Set M2 Motor Direction
    digitalWrite (IN2, HIGH); // HIGH, Rotate clockwise
  else
    digitalWrite (IN2, LOW); // LOW, Anti Rotate clockwise
  if (M2_EN == 0) // M2 Motor Speed
    analogWrite (EN2, LOW); // Low to stop miniQ
  else
    analogWrite (EN2, M2_EN);
}

void Read_Value (void) // read analog value
{
  char i;
  for (i = 0; i <8; i ++)
  {
    data [i] = analogRead (i); // read analog port voltage i
    data [i] = ((data [i] * Vr) / 1024); // convert the analog value
  }
  huntline_deal ();// call subroutine hunt deal
}
void huntline_deal (void) // Hunt
{
  if (key_1 == 6)
  {
    if (data [0]> (value [0] -1) && data [1]> (value [1] -1) && data [2] <(value [2] -1) && data [3]> (value [3 ] -1) && data [7]> (value [4] -1)) // look at the actual measured value
    {
      Motor_Control (FORW, 100, FORW, 100); // forward
    }
    else if (data [0]> (value [0] -1) && data [1]> (value [1] -1) && data [2] <(value [2] -1) && data [3] <(value [ 3] -1) && data [7]> (value [4] -1))
    {
      Motor_Control (BACK, 50, FORW, 100); // turn right
    }
    else if (data [0]> (value [0] -1) && data [1]> (value [1] -1) && data [2]> (value [2] -1) && data [3] <(value [ 3] -1) && data [7]> (value [4] -1))
    {
      Motor_Control (BACK, 100, FORW, 100); // fast right turn
    }
    else if (data [0]> (value [0] -1) && data [1]> (value [1] -1) && data [2]> (value [2] -1) && data [3] <(value [ 3] -1) && data [7] <(value [4] -1))
    {
      Motor_Control (BACK, 100, FORW, 100); // fast right turn
    }
    else if (data [0]> (value [0] -1) && data [1]> (value [1] -1) && data [2]> (value [2] -1) && data [3]> (value [ 3] -1) && data [7] <(value [4] -1))
    {
      Motor_Control (BACK, 100, FORW, 100); // fast right turn
    }
    else if (data [0]> (value [0] -1) && data [1] <(value [1] -1) && data [2] <(value [2] -1) && data [3]> (value [ 3] -1) && data [7]> (value [4] -1))
    {
      Motor_Control (FORW, 100, BACK, 50); // left
    }
    else if (data [0]> (value [0] -1) && data [1] <(value [1] -1) && data [2]> (value [2] -1) && data [3]> (value [ 3] -1) && data [7]> (value [4] -1))
    {
      Motor_Control (FORW, 100, BACK, 100); // fast turn left
    }
    else if (data [0] <(value [0] -1) && data [1] <(value [1] -1) && data [2]> (value [2] -1) && data [3]> (value [ 3] -1) && data [7]> (value [4] -1))
    {
      Motor_Control (FORW, 100, BACK, 100); // fast turn left
    }
    else if (data [0] <(value [0] -1) && data [1]> (value [1] -1) && data [2]> (value [2] -1) && data [3]> (value [ 3] -1) && data [7]> (value [4] -1))
    {
      Motor_Control (FORW, 100, BACK, 100); // fast turn left
    }
  }
}
void key_scan (void) // Scan button
{
  if (data [5]> 4.50 && data [5] <6.00) // no button is pressed
    return; // return
  else
  {
    if (data [5]>= 0.00 && data [5] <0.50) // Button 1 press
    {
      delay (180); // debounce delay
      if (data [5]>= 0.00 && data [5] <0.50) // press button 1 does
      {
        buzzer();//
        key_1 ++;// key 1 count
        if (key_1>= 1 && key_1 <= 5) value_adjust (key_1); // value of the sensor to adjust the hunt
        if (key_1 == 6)
        {
          digitalWrite (LED_RED, LOW); // red light off
          digitalWrite (LED_GREEN, LOW); // Green OFF
        }
      }
    }
    else if (data [5]>= 0.50 && data [5] <2.00)
    {
      delay (180); // debounce delay
      if (data [5]>= 0.50 && data [5] <2.00)
      {
        if (key_1>= 1 && key_1 <= 5) // if the key key1 value between 1 to 5
        {
          buzzer();//
          value [key_1-1 ]++;// sensor tracks threshold to distinguish Gaga (adjusted)
          value_adjust (key_1); // actual value comparison with
        }
      }
    }
    else if (data [5]>= 2.00 && data [5] <3.00)
    {
      delay (180); // debounce delay
      if (data [5]>= 2.00 && data [5] <3.00)
      {
        if (key_1>= 1 && key_1 <= 5) // If the key key1 value between 1 to 5
        {
          buzzer();//
          value [key_1-1 ]--;// sensor tracks threshold resolution and reduction (adjusted)
          value_adjust (key_1); // actual value comparison with
        }
      }
    }
  }
}
void value_adjust (unsigned char num) // adjust the sensor values ??hunt
{
  if (num == 1) // adjust the sensor first hunt
  {
    if (data [0]> value [0])
    {
      digitalWrite (LED_RED, HIGH); // the current value of the red light
      digitalWrite (LED_GREEN, LOW);
    }
    else
    {
      digitalWrite (LED_RED, LOW);
      digitalWrite (LED_GREEN, HIGH); // green light
    }
  }
  if (num == 2) // adjust the sensor the second hunt
  {
    if (data [1]> value [1])
    {
      digitalWrite (LED_RED, HIGH); // the current value of the red light
      digitalWrite (LED_GREEN, LOW);
    }
    else
    {
      digitalWrite (LED_RED, LOW);
      digitalWrite (LED_GREEN, HIGH); // green light
    }
  }
  if (num == 3) // adjust the third hunt sensor
  {
    if (data [2]> value [2])
    {
      digitalWrite (LED_RED, HIGH); // the current value of the red light
      digitalWrite (LED_GREEN, LOW);
    }
    else
    {
      digitalWrite (LED_RED, LOW);
      digitalWrite (LED_GREEN, HIGH); // green light
    }
  }
  if (num == 4) // adjust the sensor fourth hunt
  {
    if (data [3]> value [3])
    {
      digitalWrite (LED_RED, HIGH); // the current value of the red light
      digitalWrite (LED_GREEN, LOW);
    }
    else
    {
      digitalWrite (LED_RED, LOW);
      digitalWrite (LED_GREEN, HIGH); // green light
    }
  }
  if (num == 5) // adjust the sensor fifth hunt
  {
    if (data [4]> value [4])
    {
      digitalWrite (LED_RED, HIGH); // the current value of the red light
      digitalWrite (LED_GREEN, LOW);
    }
    else
    {
      digitalWrite (LED_RED, LOW);
      digitalWrite (LED_GREEN, HIGH); // green light
    }
  }
}
void buzzer (void) // send a buzzer sound
{
  digitalWrite (BUZZER, HIGH); // set high, the buzzer
  delay (1);
  digitalWrite (BUZZER, LOW); // set low, the buzzer did not ring
  delay (10);
}
void setup ()
{
  char i;
  for (i = 4; i <= 7; i ++)// set the port to connect two sets of motor output mode
  {
    pinMode (i, OUTPUT);
  }
  pinMode (BUZZER, OUTPUT); // Set the number of IO pins control the buzzer mode, OUTPUT is the output
  pinMode (LED_RED, OUTPUT); // set the red LED light control digital IO pin mode, OUTPUT is the output
  pinMode (LED_GREEN, OUTPUT); // green LED setting controls the number of IO pin mode, OUTPUT is the output
}
void loop ()
{
  Motor_Control (FORW, 0, FORW, 0); // car stop
  while (1)
  {
    Read_Value ();// read the value of analog port
    key_scan ();// scan button
  }
  }