#define EN1 6 // right motor enable pins
#define IN1 7 // right motor direction of the pin
#define EN2 5 // left motor enable pins
#define IN2 4 // left motor direction of the pin

#define FORW 1 // forward
#define BACK 0 // back
void Motor_Control (int M1_DIR, int M1_EN, int M2_DIR, int M2_EN) // control the motor rotation
{
   ////////// M1 ////////////////////////
   if (M1_DIR == FORW) // M1 direction of the motor
     digitalWrite (IN1, FORW); // set high, set the direction of the forward
   else
     digitalWrite (IN1, BACK); // set low, set the direction of the back
   if (M1_EN == 0) // M1 motor speed
     analogWrite (EN1, LOW); // set low, miniQ stop
   else
     analogWrite (EN1, M1_EN); // Otherwise, set the corresponding value

   /////////// M2 //////////////////////
   if (M2_DIR == FORW) // M2 motor direction
     digitalWrite (IN2, FORW); // set high, the direction of forward
   else
     digitalWrite (IN2, BACK); // set low, the direction of backward
   if (M2_EN == 0) // M2 motor speed
     analogWrite (EN2, LOW); // set low, to stop
   else
     analogWrite (EN2, M2_EN); // set the value for a given
}
void setup ()
{
   unsigned char i;
   for (i = 4; i <= 7; i ++) // settings control two of the four pins for the output motor
     pinMode (i, OUTPUT);
}
void loop ()
{
   char i;
   Motor_Control (FORW, 50, FORW, 50);
   delay (500);
   Motor_Control (BACK, 50, BACK, 50);
   delay (500);
}
