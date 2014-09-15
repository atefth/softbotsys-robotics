#include <SoftwareSerial.h>
//#include <String.h>
#include <Button.h>
#include <LCD5110_Graph.h>

#include <easyscheduler.h>

#include <Wiegand.h>
WIEGAND wg;

#define GRANTED 4419486

#define BUTTON_PIN 44
#define PULLUP true
#define INVERT true
#define DEBOUNCE_MS 20
#define GMS_POWER 6

Schedular checkForRFIDInput;
Schedular checkForWebCommand;

LCD5110 myGLCD(9,10,11,12,13);

Button myBtn(BUTTON_PIN, PULLUP, INVERT, DEBOUNCE_MS);

extern uint8_t SmallFont[];

static boolean connected;

static boolean switches[6];
static int relays[6];
static String site_url;
static int successfulRequests;
static int totalPacketLoss;
static int totalRequests;
static double failRate;
static long duration;
static long lastTime;

SoftwareSerial mySerial(50, 51);

int c = 0;
 
void setup(){
  mySerial.begin(19200);  // the GPRS baud rate   
  Serial.begin(19200);    // the GPRS baud rate 
  delay(500);

  checkForRFIDInput.start();
  checkForWebCommand.start();
  
  connected = false;

  wg.begin();
  
  successfulRequests = 0;
  totalPacketLoss = 0;
  failRate = 0.0;
  totalRequests = 0;
  
  lastTime = 0;
  
  initLCD();  
  
  site_url = "\"softbot-ras.herokuapp.com/";

  powerOn();
  
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

  myGLCD.clrScr();
  
  for (int i = 0; i < 6; ++i){
    switches[i] = false;
    relays[i] = (22+i);
    pinMode(relays[i], OUTPUT);
  }
  
}

void powerOn(){
  pinMode(GMS_POWER, OUTPUT);
  digitalWrite(GMS_POWER, HIGH);
  delay(2000);
  digitalWrite(GMS_POWER, LOW);
  delay(2000);
}

void checkRFID(){
  if(wg.available())
  {   
    long code = wg.getCode();
    if (code == GRANTED)
    {
      Serial.println(code);
      digitalWrite(relays[0], HIGH);
      delay(3000);
      // digitalWrite(relays[0], LOW);
      Serial.println("Granted");
    }else{
      Serial.println(code);
      digitalWrite(relays[0], LOW);
      delay(3000);
      // digitalWrite(relays[0], HIGH);
      Serial.println("Denied");
    }
  }
}

void initLCD(){
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  myGLCD.clrScr();
  
  myGLCD.print("SoftBot Systems", CENTER, 0);
  myGLCD.print("Robi Pilot", LEFT, 12);
  myGLCD.print("Demo Project", LEFT, 30);
  myGLCD.update();

  delay(3000);
}

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
 
void loop(){  
  // delay(500);
  // myGLCD.clrScr();
  // myGLCD.print("Processing...", LEFT, 0);
  // myGLCD.update();
  
  checkForWebCommand.check(syncServer, 10000);
  checkForRFIDInput.check(checkRFID, 1);
  // checkRFID();
  // updateLog();
  
  clearSerialData();
}

void syncServer(){
  initializeHTTP();
  getAllCommands();
  terminateHTTP();
}

void updateLog(){  
    String url = "AT+HTTPPARA=\"URL\",";
    if(switches[0]){
      switches[0] = false;
      updateChangesToSwitches();
      url += site_url + "uploadLog/site_1234/1/0/123456789/2014-08-06 21:23:14\"";
    }else{
      switches[0] = true;
      updateChangesToSwitches();
      url += site_url + "uploadLog/site_1234/1/1/123456789/2014-08-06 21:23:14\"";
    }
    makeHTTPRequest(url);
}

boolean ifRemoteAccess(){
  myBtn.read();
  if (myBtn.wasReleased()) {
    Serial.println("button was pressed");
    return true;
  }else{
    return false; 
  }
}

void initializeHTTP(){
  mySerial.println("AT+HTTPINIT"); //init the HTTP request
  delay(500);
  clearSerialData();
}

void terminateHTTP(){
  mySerial.println("AT+HTTPTERM");
  delay(500);
  clearSerialData();
}

//Get the command for a switch from server
void getCommand(int id){
  String url = "AT+HTTPPARA=\"URL\",";
  url += site_url + "getCommand/" + id + "\"";
  makeHTTPRequest(url);
  int status = readInput();  
  if (status != -1)
  {
    turnSwitch((id - 1), status); 
  }
}

void switchStatusChange(int id, int status){
  String url = "AT+HTTPPARA=\"URL\",";
  if (status == 1)
  {
    url += site_url + "turnOnSwitch/";
    switches[id] = true;
  }else{
    url += site_url + "turnOffSwitch/";
    switches[id] = false;
  }  
  url += id;
  url += "\"";
  makeHTTPRequest(url);
}

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

//Get commands for all the switches from server
void getAllCommands(){
  String url = "AT+HTTPPARA=\"URL\",";
  url += site_url + "getAll\"";
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

void updateChangesToSwitches(){
  for (int i = 0; i < 6; i++){
    turnSwitch(i, switches[i]);
  }
}

void verifyRFID(String id){
  String url = "AT+HTTPPARA=\"URL\",";
  url += site_url + "verifyRFID/";
  url += id + "\"";
  makeHTTPRequest(url);

}

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
 
void clearSerialData(){
  while(mySerial.available()!=0)
    mySerial.read();
}

void showSerialData(){
  while(mySerial.available()!=0)
    Serial.write(mySerial.read());
}







