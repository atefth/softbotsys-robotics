/*Note: this code is a demo for how to using gprs shield to send sms message, dial a voice call and 
  send a http request to the website, upload data to pachube.com by TCP connection,
 
  The microcontrollers Digital Pin 7 and hence allow unhindered
  communication with GPRS Shield using SoftSerial Library. 
  IDE: Arduino 1.0 or later
  Replace the following items in the code:
  1.Phone number, don't forget add the country code
  2.Replace the Access Point Name
  3. Replace the Pachube API Key with your personal ones assigned
  to your account at cosm.com
  */
 
#include <SoftwareSerial.h>
#include <String.h>
#include <Button.h>
#include <TimedAction.h>

#define BUTTON_PIN 36
#define PULLUP true
#define INVERT true
#define DEBOUNCE_MS 20

Button myBtn(BUTTON_PIN, PULLUP, INVERT, DEBOUNCE_MS);

static boolean switches[6];
static int relays[6];
static String site_url;
static int successfulRequests;
static int totalPacketLoss;
static int totalRequests;
static double failRate;

SoftwareSerial mySerial(50, 51);

TimedAction buttonClick = TimedAction(500, updateLog);

int c = 0;
 
void setup()
{
  mySerial.begin(19200);               // the GPRS baud rate   
  Serial.begin(19200);    // the GPRS baud rate 
  delay(500);
  
  successfulRequests = 0;
  totalPacketLoss = 0;
  failRate = 0.0;
  totalRequests = 0;
  
  site_url = "\"robi-pilot.herokuapp.com/";
  
  mySerial.println("AT+CSQ");
  delay(100);
  showSerialData();// this code is to show the data from gprs shield, in order to easily see the process of how the gprs shield submit a http request, and the following is for this purpose too.
 
  mySerial.println("AT+CGATT?");
  delay(100);
  showSerialData();
 
  mySerial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");//setting the SAPBR, the connection type is using gprs
  delay(1000);
  showSerialData();
 
  mySerial.println("AT+SAPBR=3,1,\"APN\",\"internet\"");//setting the APN, the second need you fill in your local apn server
  delay(1000);
  showSerialData();
  
//  mySerial.println("AT+SAPBR=1,1");//setting the APN, the second need you fill in your local apn server
//  delay(400);
//  showSerialData();
  
  mySerial.println("AT+SAPBR=2,1");//setting the APN, the second need you fill in your local apn server
  delay(400);
  showSerialData();
  
  for (int i = 0; i < 6; ++i){
    switches[i] = false;
    relays[i] = (22+i);
    pinMode(relays[i], OUTPUT);
  }
  
}
 
void loop()
{  
//  if (c < 5){
//   initializeHTTP();
//    String url = "AT+HTTPPARA=\"URL\",";
//    if(switches[0]){
//      switches[0] = false;
//      updateChangesToSwitches();
//      url += site_url + "uploadLog/site_1234/1/0/123456789/2014-08-06 21:23:14\"";
//    }else{
//      switches[0] = true;
//      updateChangesToSwitches();
//      url += site_url + "uploadLog/site_1234/1/1/123456789/2014-08-06 21:23:14\"";
//    }
//    makeHTTPRequest(url);
//    terminateHTTP();
//    clearSerialData(); 
//    c++;
//  }
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
        returned = mySerial.read();
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
  while(mySerial.available()!=0 && count < 5){
    returned = mySerial.read();
    if(returned == '<'){
      returned = mySerial.read();
      packetLoss = false;
    }else if ( returned == '0' || returned == '1')
    {
      packetLoss = false;
      if (returned == '1'){
        switches[count] = true;
      }else{
        switches[count] = false;
      }
      count++;
    }    
  }
  if (!packetLoss){
    successfulRequests++;
  }else{
    totalPacketLoss++;
  }
  return packetLoss;
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
  readAllInputs();
  showRequestData();
  updateChangesToSwitches();
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
//  int status = readInput();
//  Serial.write("Received : ");
//  Serial.write(status);
//  Serial.println();
}

void makeHTTPRequest(String url){
  
  mySerial.println("AT+HTTPPARA=\"CID\",1");  
  delay(1000); 
  clearSerialData();
  
  mySerial.println(url);// setting the httppara, the second parameter is the website you want to access
  delay(500);
  clearSerialData();
 
  mySerial.println("AT+HTTPACTION=0");//submit the request 
  delay(3000);
  clearSerialData();
  //the delay is very important, the delay time is base on the return from the website, if the return datas are very large, the time required longer.
  //while(!mySerial.available());

  mySerial.println("AT+HTTPREAD");// read the data from the website you access
  delay(500);
//  Serial.println();
  totalRequests++;
}
 
void clearSerialData()
{
  while(mySerial.available()!=0)
    mySerial.read();
}

void showSerialData()
{
  while(mySerial.available()!=0)
    Serial.write(mySerial.read());
}


