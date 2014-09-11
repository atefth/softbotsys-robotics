char dataIn = 'S';        //Character/Data coming from the phone.
int pinForwardBack = 13;     //Pin that controls the car's Forward-Back motor.
int pinLeftRight = 12;       //Pin that controls the car's Left-Right motor.
int pinBrakeLeftRight = 9;  //Pin that enables/disables Left-Right motor.
int pinBrakeForwardBack = 8;  //Pin that enables/disables Forward-Back motor.
int pinLeftRightSpeed = 3;    //Pin that sets the speed for the Left-Right motor.
int pinForwardBackSpeed = 11;  //Pin that sets the speed for the Forward-Back motor.
int pinfrontLights = 4;    //Pin that activates the Front lights.
int pinbackLights = 7;    //Pin that activates the Back lights.
char determinant;         //Used in the check function, stores the character received from the phone.
char det;                 //Used in the loop function, stores the character received from the phone.
int velocity = 0;    //Stores the speed based on the character sent by the phone.

void setup() 
{       
//*************NOTE: If using Bluetooth Mate Silver use 115200 btu
//                   If using MDFLY Bluetooth Module use 9600 btu
Serial.begin(115200);  //Initialize serial communication with Bluetooth module at 115200 btu.
pinMode(pinForwardBack, OUTPUT);
pinMode(pinLeftRight, OUTPUT);
pinMode(pinBrakeLeftRight, OUTPUT);
pinMode(pinBrakeForwardBack, OUTPUT);
pinMode(pinLeftRightSpeed , OUTPUT);
pinMode(pinForwardBackSpeed , OUTPUT);
pinMode(pinfrontLights , OUTPUT);
pinMode(pinbackLights , OUTPUT);
}

void loop()
{ 
    det = check();
    Serial.println(det);
      while (det == 'F')   //if incoming data is a F, move forward
      {     
          digitalWrite(pinForwardBack, LOW); 
          digitalWrite(pinBrakeForwardBack, LOW);
          analogWrite(pinForwardBackSpeed,velocity);
          analogWrite(pinLeftRightSpeed,0);  //To make the Left/Right Motor not move, you just need to make its speed 0.    
          det = check();          
      }  
      while (det == 'B')   //if incoming data is a B, move back
      {    
          digitalWrite(pinForwardBack, HIGH); 
          digitalWrite(pinBrakeForwardBack, LOW);
          analogWrite(pinForwardBackSpeed,velocity); 
          analogWrite(pinLeftRightSpeed,0);         
          det = check();          
      } 

      while (det == 'L')   //if incoming data is a L, move wheels left
      {     
          digitalWrite(pinLeftRight, HIGH);
          digitalWrite(pinBrakeLeftRight, LOW);
          digitalWrite(pinForwardBack, LOW);
          analogWrite(pinForwardBackSpeed,0);    //To make the Forward/Back Motor not move, you just need to make its speed 0.
          analogWrite(pinLeftRightSpeed,255);        
          det = check();          
      }  
      while (det == 'R')   //if incoming data is a R, move wheels right
      {    
          digitalWrite(pinLeftRight, LOW);
          digitalWrite(pinBrakeLeftRight, LOW);
          digitalWrite(pinForwardBack, LOW);
          analogWrite(pinForwardBackSpeed,0);
          analogWrite(pinLeftRightSpeed,255);         
          det = check();          
      }
     
      while (det == 'I')   //if incoming data is a I, turn right forward
      {     
          digitalWrite(pinLeftRight, LOW);
          digitalWrite(pinForwardBack, LOW);          
          digitalWrite(pinBrakeLeftRight, LOW); 
          digitalWrite(pinBrakeForwardBack, LOW);  
          analogWrite(pinForwardBackSpeed,velocity);
          analogWrite(pinLeftRightSpeed,255);                  
          det = check();          
      }  
      while (det == 'J')   //if incoming data is a J, turn right back
      {      
          digitalWrite(pinLeftRight, LOW);
          digitalWrite(pinForwardBack, HIGH);         
          digitalWrite(pinBrakeLeftRight, LOW); 
          digitalWrite(pinBrakeForwardBack, LOW);
          analogWrite(pinForwardBackSpeed,velocity);
          analogWrite(pinLeftRightSpeed,255);          
          det = check();          
      }          
      while (det == 'G')   //if incoming data is a G, turn left forward
      { 
          digitalWrite(pinLeftRight, HIGH);       
          digitalWrite(pinForwardBack, LOW);
          digitalWrite(pinBrakeLeftRight, LOW); 
          digitalWrite(pinBrakeForwardBack, LOW); 
          analogWrite(pinForwardBackSpeed,velocity);
          analogWrite(pinLeftRightSpeed,255);          
          det = check();          
      }    
      while (det == 'H')   //if incoming data is a H, turn left back
      {
          digitalWrite(pinLeftRight, HIGH);
          digitalWrite(pinForwardBack, HIGH); 
          digitalWrite(pinBrakeLeftRight, LOW); 
          digitalWrite(pinBrakeForwardBack, LOW);
          analogWrite(pinForwardBackSpeed,velocity);
          analogWrite(pinLeftRightSpeed,255);          
          det = check();                                              
      }   
      while (det == 'S')   //if incoming data is a S, stop
      {
          digitalWrite(pinForwardBack, LOW); 
          digitalWrite(pinLeftRight, LOW);
          analogWrite(pinForwardBackSpeed,0);
          analogWrite(pinLeftRightSpeed,0);          
          det = check(); 
      }
      while (det == 'U')   //if incoming data is a U, turn ON front lights
      {
          digitalWrite(pinfrontLights, HIGH);          
          det = check(); 
      }
      while (det == 'u')   //if incoming data is a u, turn OFF front lights
      {
          digitalWrite(pinfrontLights, LOW);          
          det = check(); 
      }
      while (det == 'W')   //if incoming data is a W, turn ON back lights
      {
          digitalWrite(pinbackLights, HIGH);          
          det = check(); 
      }
      while (det == 'w')   //if incoming data is a w, turn OFF back lights
      {
          digitalWrite(pinbackLights, LOW);
          det = check(); 
      }
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
        else if (dataIn == 'U') 
        {
          determinant = 'U';
        }
        else if (dataIn == 'u') 
        {
          determinant = 'u';
        }
        else if (dataIn == 'W') 
        {
          determinant = 'W';
        }
       
        else if (dataIn == 'w') 
        {
          determinant = 'w';
        }
  }
return determinant;
}

