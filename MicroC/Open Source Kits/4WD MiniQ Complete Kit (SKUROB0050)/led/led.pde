# define LED_RED 12 // define pin red led lights
void setup ()
{
   pinMode (LED_RED, OUTPUT); // set the LED light mode for output pin
}
void loop ()
{
   digitalWrite (LED_RED, HIGH); // LED lamp pin high, light LED light
   delay (1000); // delay 1s
   digitalWrite (LED_RED, LOW); // LED pin lights low, turn off LED lights
   delay (1000); // delay 1s
}
