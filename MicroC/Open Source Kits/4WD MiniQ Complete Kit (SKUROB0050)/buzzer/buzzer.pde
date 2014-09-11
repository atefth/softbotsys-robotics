# define BUZZER 11 // define the buzzer pin
void setup ()
{
   pinMode (BUZZER, OUTPUT); // set the buzzer pin to output mode
}
void loop ()
{
   unsigned char i, j; // define variables
   while (1)
   {
     // Output frequency of a sound
     for (i = 0; i <80; i ++)
     {
       digitalWrite (BUZZER, HIGH); //Make Sound
       delay (1); // Delay 1ms
       digitalWrite (BUZZER, LOW); // do not send sound
       delay (1); // Delay ms
     }
     // Output the sound to another frequency
     for (i = 0; i <100; i ++)
     {
       digitalWrite (BUZZER, HIGH); //Make Sound
       delay (2); // 2ms delay
       digitalWrite (BUZZER, LOW); // do not send sound
       delay (2); // 2ms delay
     }
   }
}
