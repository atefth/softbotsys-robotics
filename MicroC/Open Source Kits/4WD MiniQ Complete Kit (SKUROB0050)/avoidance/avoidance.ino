#define EN1 6//Right Motor Enable Pin
#define IN1 7//Right Motor Direction Pin
#define EN2 5//Left Motor Enable Pin
#define IN2 4//Left Motor Direction Pin

#define FORW 1//Forward
#define BACK 0//Backward

#define IR_IN  8//Infrared Receiver (Digital Pin 8)
#define L_IR 9//Left Infared Transmitter (Digital Pin 9)
#define R_IR 10//Right Infrared Transmitter (Digital Pin 10)

int count;//Pulse counter

void Motor_Control(int M1_DIR,int M1_EN,int M2_DIR,int M2_EN)//Motor control function
{
  //////////M1////////////////////////
  if(M1_DIR==FORW)//Set M1 Motor Direction  
    digitalWrite(IN1,HIGH);//HIGH, Rotate clockwise
  else
    digitalWrite(IN1,LOW);//LOW, Anti Rotate clockwise  
  if(M1_EN==0)//M1 Motor Speed
    analogWrite(EN1,LOW);//Low to stop miniQ
  else
    analogWrite(EN1,M1_EN);

  ///////////M2//////////////////////
  if(M2_DIR==FORW)//Set M2 Motor Direction    
    digitalWrite(IN2,HIGH);//HIGH, Rotate clockwise 
  else
    digitalWrite(IN2,LOW);//LOW, Anti Rotate clockwise   
  if(M2_EN==0) //M2 Motor Speed
    analogWrite(EN2,LOW);//Low to stop miniQ
  else
    analogWrite(EN2,M2_EN); 
}

void L_Send38KHZ(void)//Left Infrared transmitter generate 38kHZ pulse
{
  digitalWrite(L_IR,HIGH);//Set the infrared to HIGH level
  delayMicroseconds(10);//delay 10 microsecond
  digitalWrite(L_IR,LOW);//Set the infrared to LOW level
  delayMicroseconds(10);//delay 10 microsecond
}
void R_Send38KHZ(void)//Right Infrared transmitter generate 38kHZ pulse
{
  digitalWrite(R_IR,HIGH);//Set the infrared to HIGH level
  delayMicroseconds(10);//delay 10 microsecond
  digitalWrite(R_IR,LOW);//Set the infrared to LOW level
  delayMicroseconds(10);//delay 10 microsecond
}
void pcint0_init(void)//Init the interrupter
{
  PCICR = 0X01;//
  PCMSK0 = 0X01;//
}

ISR(PCINT0_vect)//
{
  count++;//Counting the received pulser
}
void Obstacle_Avoidance(void)
{
  char i;
  for(i=0;i<20;i++)//Left infrared transmit Send 20 38khz pulse
  {
    L_Send38KHZ();     
  }
  if(count>20)//If the counting is more than 10, there is obstacle avoidance  
  {
    count=0;//reset the counter
    Motor_Control(BACK,200,BACK,200);//Backward
    delay(500);//Delay for 0.5 second
    Motor_Control(BACK,200,FORW,200);//Turn right
    delay(500);//Delay for 0.5 second
  }
  else
  {
    Motor_Control(FORW,200,FORW,200);//Forward
  }

  for(i=0;i<20;i++)//Right infrared transmit Send 20 38khz pulse
  {
    R_Send38KHZ();     
  }
  if(count>20)
  {
    count=0;
    Motor_Control(BACK,200,BACK,200);//Backward
    delay(500);//delay for 0.5 second
    Motor_Control(FORW,200,BACK,200);//turn left
    delay(500);//delay for 0.5 second
  }
  else
  {
    Motor_Control(FORW,200,FORW,200);//Move forward
  }
}
void setup()
{
  char i;
  for(i=4;i<=7;i++)//configure the pin mode
  {
    pinMode(i,OUTPUT);
  }
  pinMode(L_IR,OUTPUT);//Set the L_IR pin to output mode
  pinMode(R_IR,OUTPUT);//Set the R_IR pin to output mode
  pinMode(IR_IN,INPUT);//Set the IR receiver pin to input mode
  pcint0_init();//Init the interrupter
  sei();     //Enable the interrupter
  
}
void loop()
{
  Motor_Control(FORW,180,FORW,180);//Set the speed     
  while(1)
  {
    Obstacle_Avoidance();//Obstacle avoidance
  }
}





