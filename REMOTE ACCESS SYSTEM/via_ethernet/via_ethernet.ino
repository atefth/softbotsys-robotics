#include <SoftwareSerial.h>
#include <String.h>
#include <Button.h>
#include <LCD5110_Graph.h>
#include <SPI.h>
#include <Ethernet.h>

#define BUTTON_PIN 44
#define PULLUP true
#define INVERT true
#define DEBOUNCE_MS 20

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

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetClient client;

SoftwareSerial mySerial(50, 51);

int c = 0;
 
void setup(){
  mySerial.begin(19200);  // the GPRS baud rate   
  Serial.begin(19200);    // the GPRS baud rate 
  delay(500);
  
  connected = false;
  
  successfulRequests = 0;
  totalPacketLoss = 0;
  failRate = 0.0;
  totalRequests = 0;
  
  lastTime = 0;
  
  initLCD();  
  
  site_url = "\"robi-pilot.herokuapp.com/";
  
  while(!connected){
    connected = client.connect(server, 80);
    printConnectionData();
    delay(1000);
  }  
 
  initEthernet();

  myGLCD.clrScr();
  
  for (int i = 0; i < 6; ++i){
    switches[i] = false;
    relays[i] = (2+i);
    pinMode(relays[i], OUTPUT);
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

  delay(6000);
}

void initEthernet(){
   // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
}
 
void loop(){  
  delay(2500);
  myGLCD.clrScr();
  myGLCD.print("Processing...", LEFT, 0);
  myGLCD.update();
  initializeHTTP();
  getAllCommands();
  terminateHTTP();
  clearSerialData();
}

void updateLog(){  
  if(ifRemoteAccess()){
    initializeHTTP();
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
    terminateHTTP();
    clearSerialData();
  }
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
  delay(4000);
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
  
  client.println("GET /search?q=arduino HTTP/1.1");
  client.println("Host: www.google.com");
  client.println("Connection: close");
  client.println();
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


