# define BUZZER 11 // set the control digital IO foot buzzer
# define LED_RED 12 // set the red LED light control digital IO pins
# define LED_GREEN 13 // set the control green LED digital IO pins
# define Vr 5 // reference voltage

float data = 0xff; // store the analog voltage value, initial value is set to zero the number can not. If the initial value set to 0, will affect 1 to determine whether the pressed key.
void buzzer (void) // send a buzzer sound
{
   digitalWrite (BUZZER, HIGH); // set high, the buzzer
   delay (1);
   digitalWrite (BUZZER, LOW); // set low, the buzzer did not ring
   delay (10);
}
void Read_Value (void) // read voltage
{
   data = analogRead (5); // read the analog voltage value 5
   data = ((data * Vr) / 1024); // digital value into an analog value
   key_scan ();// execute key scan function
}
void key_scan (void) // Scan button
{
   if (data> 4.50 && data <6.00) // no button is pressed
     return; // return
   else
   {
     if (data>= 0.00 && data <0.50) // Button 1 press
     {
       delay (180); // debounce delay
       if (data>= 0.00 && data <0.50) // press button 1 does
       {
         buzzer ();//
         digitalWrite (LED_RED, HIGH); // red light
       }
     }
     else if (data>= 0.50 && data <1.5)
     {
       delay (180);
       if (data>= 0.50 && data <1.5)
       {
         buzzer ();
         digitalWrite (LED_GREEN, HIGH); // green light
       }
     }
     else if (data>= 1.5 && data <2.5)
     {
       delay (180);
       if (data>= 1.5 && data <2.5)
       {
         buzzer ();
         digitalWrite (LED_RED, LOW); // red light off
         digitalWrite (LED_GREEN, LOW); // Green OFF
       }
     }
   }
}
void setup ()
{
   pinMode (BUZZER, OUTPUT); // buzzer output port mode
   pinMode (LED_RED, OUTPUT); // red port to output mode
   pinMode (LED_GREEN, OUTPUT); // green light of the port output mode
}
void loop ()
{
   Read_Value ();// read voltage
}
