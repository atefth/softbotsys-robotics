#define EN1 6//Right motor enable pin
#define IN1 7//Right motor direction pin
#define EN2 5//Left motor enable pin
#define IN2 4//Left motor direction pin
#define FORW 1//forward
#define BACK 0//back
int Pulse_Width=0;//Store the pulse width
int   ir_code=0x00;    //Command value
void Motor_Control(int M1_DIR,int M1_EN,int M2_DIR,int M2_EN)//Control motor speed 
{
//////////M1////////////////////////
if(M1_DIR==FORW)  
digitalWrite(IN1,HIGH); //set high, direction forward
else
digitalWrite(IN1,LOW);//set low, direction back
if(M1_EN==0) 
analogWrite(EN1,LOW);// set low, MiniQ stop
else
analogWrite(EN1,M1_EN);// Otherwise, you set the appropriate values
///////////M2//////////////////////
if(M2_DIR==FORW)  
digitalWrite(IN2,HIGH); 
else
digitalWrite(IN2,LOW);   
if(M2_EN==0) 
analogWrite(EN2,LOW);
else
analogWrite(EN2,M2_EN); 
} 
 
//counter1 init
void timer1_init(void)
{
TCCR1A = 0X00; 
TCCR1B = 0X05;//The clock source to a timer
TCCR1C = 0X00;
TCNT1 = 0X00;
TIMSK1 = 0X00;  //Overflow timer interrupt is prohibited
 
}
 
//IR controler
void remote_deal(void)
{ 
switch(ir_code)
{
case 0xff00:
Motor_Control(FORW,0,FORW,0);//stop
break;
case 0xfe01:
Motor_Control(FORW,150,FORW,150);//forward
break;
case 0xf609:
Motor_Control(BACK,150,BACK,150);//back
break;
case 0xfb04:
Motor_Control(FORW,150,BACK,150);//turn left
break;
case 0xf906:
Motor_Control(BACK,150,FORW,150);//turn right
break;
} 
}
char logic_value()
{
while(!(digitalRead(8))); //low wait
Pulse_Width=TCNT1;
TCNT1=0;
if(Pulse_Width>=7&&Pulse_Width<=10)//Low voltage 560us
{
while(digitalRead(8));//if the value is high ,waiting
Pulse_Width=TCNT1;
TCNT1=0;
if(Pulse_Width>=7&&Pulse_Width<=10)//continous high value 560us
return 0;
else if(Pulse_Width>=25&&Pulse_Width<=27) //continous high value 1.7ms
return 1;
}
return -1;
}
 
// decoder of remote controllor
void pulse_deal()
{
int i;
//执行 8 个 0
for(i=0; i<8; i++)
{
if(logic_value() != 0) //not  0
return;
}
//Execute 6 "1"
for(i=0; i<6; i++)
{
if(logic_value()!= 1) //not 1
return; 
}
//Execute 1 "0"
if(logic_value()!= 0) //no 0
return;
//Execute 1 "1"
if(logic_value()!= 1) //no 1
return;
//Parse the command of remote control code
ir_code=0x00;//clear
for(i=0; i<16;i++ )
{
if(logic_value() == 1)
{
ir_code |=(1<<i);
}
}
}
 
// decoder of remote controllor
void remote_decode(void)
{
TCNT1=0X00;       
while(digitalRead(8))//if the value is high ,waiting
{
if(TCNT1>=1563)   //When the duration of high level is more than 100 ms, show that  no button pressed
{
ir_code = 0xff00;
return;
}  
}
 
//If high level duration is less than 100 ms
 
TCNT1=0X00;
while(!(digitalRead(8))); //low wait
Pulse_Width=TCNT1;
TCNT1=0;
if(Pulse_Width>=140&&Pulse_Width<=141)//9ms
{
while(digitalRead(8));//if the value is high ,waiting
Pulse_Width=TCNT1;
TCNT1=0;
if(Pulse_Width>=68&&Pulse_Width<=72)//4.5ms
{
pulse_deal();
return;
}
else if(Pulse_Width>=34&&Pulse_Width<=36)//2.25ms
{
while(!(digitalRead(8)));//low wait
Pulse_Width=TCNT1;
TCNT1=0;
if(Pulse_Width>=7&&Pulse_Width<=10)//560us
{
return; 
}
}
}
}
void setup()
{
unsigned char i;
for(i=4;i<=7;i++)
{
pinMode(i,OUTPUT);
}
pinMode(8,INPUT);
}
void loop()
{  
timer1_init();
while(1)
{
remote_decode();   //decoding
remote_deal();    //Performs decoding results
}  
}
