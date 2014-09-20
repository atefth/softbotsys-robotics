#include <SoftwareSerial.h>
//#include <String.h>
#include <LCD5110_Graph.h>


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//CONFIG VARIABLES
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  static int site_id = 10;
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//GSM VARIABLES
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  SoftwareSerial mySerial(50, 51);
  #define GSM_POWER 6
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//LCD VARIABLES
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  LCD5110 myGLCD(9,10,11,12,13);
  extern uint8_t SmallFont[];
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//APPLICATION LOGIC
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  static boolean connected;
  static boolean switches[6];
  static int relays[6];
  static String site_url;
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//QOS VARIABLES
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
 int successfulRequests;
 int totalPacketLoss;
 int totalRequests;
 double failRate;
 long duration;
 long lastTime;
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

// Schedular checkForRFIDInput;
// Schedular checkForWebCommand;

int c = 0;
 
void setup(){
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  //INITIALISE SERIAL INTERFACE
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//  
    mySerial.begin(19200);
    Serial.begin(19200);
    delay(500);
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    

  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  //INITIALISE QOS
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    successfulRequests = 0;
    totalPacketLoss = 0;
    failRate = 0.0;
    totalRequests = 0;
    lastTime = 0;
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  //INITIALISE LCD
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    initLCD();
    myGLCD.clrScr();
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  //INITIALISE GSM
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    powerOn();
    connected = false;
    while(!connected){
      mySerial.println("AT+CSQ");
      delay(1000);
      showSerialData();
      mySerial.println("AT+CGATT?");
      showSerialData();
      delay(1000);
      checkConnection();
      printConnectionData();
      delay(10000);
    }  
    initGSM();
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  //APPLICATION LOGIC
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    site_url = "\"softbot-ras.herokuapp.com/";
    for (int i = 0; i < 6; ++i){
      switches[i] = false;
      relays[i] = (22+i);
      pinMode(relays[i], OUTPUT);
    }
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//POWER ON GSM
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  void powerOn(){
    pinMode(GSM_POWER, OUTPUT);
    digitalWrite(GSM_POWER, HIGH);
    delay(2000);
    digitalWrite(GSM_POWER, LOW);
    delay(2000);
  }

  void checkConnection(){
    char current;
    while(mySerial.available() != 0){
      current = mySerial.read();
      if ( current == '1')
      {
        connected = true;
      }
    }
  }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//SET UP LCD
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  void initLCD(){
    myGLCD.InitLCD();
    myGLCD.setFont(SmallFont);
    myGLCD.clrScr();
    
    myGLCD.print("SoftBot Systems", CENTER, 0);
    myGLCD.print("RASS 1.1", LEFT, 12);
    myGLCD.print("PROTOTYPE", LEFT, 30);
    myGLCD.update();

    delay(3000);
  }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//SETUP GSM
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  void initGSM(){
    mySerial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");//setting the SAPBR, the connection type is using gprs
    showSerialData();
    delay(1500);
   
    mySerial.println("AT+SAPBR=3,1,\"APN\",\"internet\"");//setting the APN, the second need you fill in your local apn server
    showSerialData();
    delay(1500);
    
    mySerial.println("AT+SAPBR=1,1");//setting the APN, the second need you fill in your local apn server
    showSerialData();
    delay(1400);
    
    mySerial.println("AT+SAPBR=2,1");//setting the APN, the second need you fill in your local apn server
    showSerialData();  
    delay(1400);
  }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

 
void loop(){  
  delay(500);
  syncServer();
  clearSerialData();
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//SYNC WITH CENTRAL SERVER
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  void syncServer(){
    initializeHTTP();
    getAllCommands();
    terminateHTTP();
  }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//BEGIN HTTP REQUEST
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  void initializeHTTP(){
    mySerial.println("AT+HTTPINIT"); //init the HTTP request
    delay(500);
    clearSerialData();
  }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//MAKE HTTP REQUEST
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  void makeHTTPRequest(String url){
    
    mySerial.println("AT+HTTPPARA=\"CID\",1");  
    delay(1000); 
    showSerialData();
    
    // setting the httppara, the second parameter is the website you want to access
    mySerial.println(url);
    delay(500);
    showSerialData();
   
    //submit the request 
    mySerial.println("AT+HTTPACTION=0");
    delay(4000);
    showSerialData();

    // read the data from the website you access
    mySerial.println("AT+HTTPREAD");  
    delay(1000);
    totalRequests++;
  }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//END HTTP REQUEST
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  void terminateHTTP(){
    mySerial.println("AT+HTTPTERM");
    delay(500);
    clearSerialData();
  }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//CHANGE RELAY STATUS
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  void turnSwitch(int id, int command){
    if (command == '1' || command == true)
    {
      switches[id] = true;
      digitalWrite(relays[id], HIGH);
    }else{
      switches[id] = false;
      digitalWrite(relays[id], LOW);
    }
  }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//QOS LOGIC FOR LCD
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  void printTimeData(){
    myGLCD.clrScr();
    myGLCD.print("DURATION : ", LEFT, 0);
    myGLCD.printNumI((int)duration, 65, 0);
    myGLCD.update();
  }

  void printConnectionData(){
    myGLCD.clrScr();  
    if (connected)
    {
      myGLCD.print("Connected!", 15, 0);
    }else{
      myGLCD.print("Not Connected!", LEFT, 0);
      myGLCD.print("Retrying...", LEFT, 25);
    }
    myGLCD.update();
  }

  void printRequestData(){
    failRate = ((totalPacketLoss*1.0/totalRequests)*100);
    String temp = "";
    
    myGLCD.clrScr();
    myGLCD.print("SUCCESS : ", LEFT, 0);
    myGLCD.printNumI(successfulRequests, 65, 0);
    myGLCD.print("LOSS : ", LEFT, 12);
    myGLCD.printNumI(totalPacketLoss, 65, 12);
    myGLCD.print("TOTAL : ", LEFT, 24);
    myGLCD.printNumI(totalRequests, 65, 24);
    myGLCD.print("FAIL : ", LEFT, 36);
    myGLCD.printNumI((int)failRate, 60, 36);
    myGLCD.print("%", RIGHT, 36);
    myGLCD.update();
  }

  void printStatusData(){  
    myGLCD.clrScr();
    myGLCD.print("DOOR : ", LEFT, 0);
    if (isDoorOpen())
    {
      myGLCD.print("OPEN", 45, 0);
    }else{
      myGLCD.print("CLOSED", 45, 0);
    }
    myGLCD.print("LIGHT : ", LEFT, 10);
    if (isLightOn())
    {
      myGLCD.print("ON", 65, 10);
    }else{
      myGLCD.print("OFF", 65, 10);
    }
    myGLCD.print("ALARM : ", LEFT, 20);
    if (isAlarmOn())
    {
      myGLCD.print("ON", 65, 20);
    }else{
      myGLCD.print("OFF", 65, 20);
    }
    myGLCD.print("GENERATOR : ", LEFT, 30);
    if (isGeneratorOn())
    {
      myGLCD.print("ON", 65, 30);
    }else{
      myGLCD.print("OFF", 65, 30);
    }
    myGLCD.print("AC : ", LEFT, 40);
    if (isAcOn())
    {
      myGLCD.print("ON", 65, 40);
    }else{
      myGLCD.print("OFF", 65, 40);
    }  
    myGLCD.print("MAINS : ", LEFT, 50);
    if (isMainsOn())
    {
      myGLCD.print("ON", 65, 50);
    }else{
      myGLCD.print("OFF", 65, 50);
    }
    myGLCD.update();  
  }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//APPLCIATION LOGIC
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  int readInput(){
    char returned = ' ';
    boolean isLoss = true;
    while(mySerial.available()!=0){
      returned = mySerial.read();
      if(returned == '<'){
          isLoss = false;        
      }else if(returned == '0' || returned == '1')
      {
        return returned;
      }
    }
    if (isLoss)
    {
      return -1;
    }
  }
  boolean readAllInputs(){
    char returned = ' ';
    int count = 0;
    boolean packetLoss = true;
    boolean temp[6];
    while(mySerial.available()!=0 && count < 6){
      returned = readInput();
      if (returned != -1)
      {
        if ( returned == '0' || returned == '1')
        {
          packetLoss = false;
          if (returned == '1'){
            temp[count] = true;
          }else{
            temp[count] = false;
          }
          count++;
        }
      }else{
      }
    }
    if (count == 6)
    {
      for (int i = 0; i < 6; i++)
      {
        switches[i] = temp[i];
      }
    }
    clearSerialData();
    if (!packetLoss){
      duration = ((millis() - lastTime)/ 1000);
      lastTime = millis();
      successfulRequests++;
    }else{
      totalPacketLoss++;
    }
    return packetLoss;
  }

  boolean isDoorOpen(){
    return switches[0];
  }

  boolean isLightOn(){
    return switches[1];
  }

  boolean isAlarmOn(){
    return switches[2];
  }

  boolean isGeneratorOn(){
    return switches[3];
  }

  boolean isAcOn(){
    return switches[4];
  }

  boolean isMainsOn(){
    return switches[5];
  }

  //Get commands for all the switches from server
  void getAllCommands(){
    String url = "AT+HTTPPARA=\"URL\",";
    url += site_url + "getAll\"" + site_id;
    makeHTTPRequest(url);
    if(!readAllInputs()){ 
      updateChangesToSwitches();
    }
    showRequestData();
    clearSerialData();
    printRequestData();
    delay(3000);
    printStatusData();
    delay(3000);
    printTimeData();
  }

  //Update local switches
  void updateChangesToSwitches(){
    for (int i = 0; i < 6; i++){
      turnSwitch(i, switches[i]);
    }
  }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//SERIAL DATA
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  void clearSerialData(){
    while(mySerial.available()!=0)
      mySerial.read();
  }

  void showSerialData(){
    while(mySerial.available()!=0)
      Serial.write(mySerial.read());
  }

  void showRequestData(){
    failRate = ((totalPacketLoss*1.0/totalRequests)*100);
    Serial.print("successful requests : ");
    Serial.print(successfulRequests);
    Serial.print(" ---- ");
    Serial.print("failed requests : ");
    Serial.print(totalPacketLoss);
    Serial.print(" ---- ");
    Serial.print("total requests : ");
    Serial.print(totalRequests);
    Serial.print(" ---- ");
    Serial.print("fail rate : ");
    Serial.print(failRate);
    Serial.print("%");
    Serial.println();
  }
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
