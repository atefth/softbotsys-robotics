#include <SdFat.h>

const uint8_t chipSelect = 10;
SdFat sd;
SdFile config;

// Change spiSpeed to SPI_FULL_SPEED for better performance
// Use SPI_QUARTER_SPEED for even slower SPI bus speed
const uint8_t spiSpeed = SPI_HALF_SPEED;

static int site_id;

int const USERS = 10;

static long rfid[USERS];
static int access[USERS];
int siteRead = 0;
int rfidRead[USERS];

void setup() {
  Serial.begin(9600);
  // while (!Serial) {}  // wait for Leonardo
  // Serial.println("Type any character to start");
  // while (Serial.read() <= 0) {}
  // delay(400);  // catch Due reset problem
  Serial.println("Initialising");
  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();
  Serial.println("Connected to storage");  
  clearConfig();
  writeConfig();
  initConfig();
  delay(1000);
}

void clearConfig(){
  //open the current config.txt file to delete
  if (!config.open("config.txt", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening config.txt for write failed");
  }

  Serial.println("removing old config file");

  //remove the file
  if (!config.remove()) Serial.println("failed to remove existing config file");
}

void writeConfig(){    
  // open the file for write at end like the Native SD library
  if (!config.open("config.txt", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening config.txt for write failed");
  }

  // if the file opened okay, write to it:
  Serial.print("Writing to config.txt...");
  config.println("site_id 5");
  config.println("rfid 123456789 T");
  config.println("rfid 987654321 F");
  config.println("rfid 455652231 F");
  config.println("rfid 624534876 T");
  config.println("rfid 333229977 F");
  config.println("rfid 223193411 T");
  config.println("rfid 499518021 T");
  config.println("rfid 700193451 F");
  config.println("rfid 888271234 T");
  config.println("rfid 909923188 F");
  config.println("!");

  // close the file:
  config.close();
  Serial.println("done.");
}

void initConfig(){
  // re-open the file for reading:
  if (!config.open("config.txt", O_READ)) {
    sd.errorHalt("opening config.txt for read failed");
  }
  Serial.println("config.txt:");

  // read from the file until there's nothing else in it:
  int data = config.read();
  while (data >= 0) {
    char c = (char)data;

    //skip input
    while (c != ' ') {
      data = config.read();
      c = (char)data;
    }
    data = config.read();
    c = (char)data;
    char site[4];
    int count = 0;

    //read site id if not read
    if (!siteRead)
    {
      //read site id
      while (data != 13) {
        site[count] = c;
        data = config.read();
        c = (char)data;      
        count++;
      }

      //store site id in ram
      site_id = atol(site);
      siteRead = 1;
    }

    count = 0;
    data = config.read();
    c = (char)data;
    while (c != '!' && count < 10) {

      //skip inputs
      while (c != ' ') {
        data = config.read();
        c = (char)data;
      }

      data = config.read();
      c = (char)data;
      
      if (!rfidRead[count])
      {
        char digits[10];
        for (int i = 0; i < 9; i++)
        {
          digits[i] = 0;
        }
        //read all 9 rfid digits
        for (int i = 0; i < 9; i++)
        {
          digits[i] = c;
          data = config.read();
          c = (char)data;
        }

        //store rfid in ram
        rfid[count] = atol(digits);
        if (rfid[count] < 100)
        {
          rfid[count] = 0;
          break;
        }

        data = config.read();
        c = (char)data;

        //update access to rfid
        if (c == 'F' || c == 'T')
        {
          if (c == 'T')
          {
            access[count] = 1;
          }else{
            access[count] = 0;
          }
        }
        rfidRead[count] = 1;
        count++;
      }      

      data = config.read();
      c = (char)data;      

    }
    break;
  }
  // close the file:
  config.close();
}

void updateConfig(){
  updateNewConfig();
  writeNewConfig();
  // initConfig();
}

void writeNewConfig(){

  clearConfig();
  // open the file for write at end like the Native SD library
  if (!config.open("config.txt", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening config.txt for write failed");
  }
  // if the file opened okay, write to it:
  Serial.print("Updating Config File");
  config.print("site_id ");
  config.println(site_id);
  for (int i = 0; i < USERS; i++)
  {
    config.print("rfid ");
    config.print(rfid[i]);
    config.print(" ");
    if (access[i])
    {
      config.print("T");
    }else{
      config.print("F");
    }
    config.println();
  }

  config.println("!");

  // close the file:
  config.close();
  Serial.println("done.");
}

int updateSiteId(){
  return 102;
}

long updateRFID(int user_id){
  switch (user_id){
    case 0:
      return 909923188;
    case 1:
      return 888271234;
    case 2:
      return 700193451;
    case 3:
      return 499518021;
    case 4:
      return 223193411;
    case 5:
      return 333229977;
    case 6:
      return 624534876;    
    case 7:
      return 455652231;
    case 8:
      return 987654321;
    case 9:
      return 123456789;
  }
}

int updateAccess(int user_id){
  switch (user_id){
    case 0:
      return 0;
    case 1:
      return 1;
    case 2:
      return 0;
    case 3:
      return 1;
    case 4:
      return 1;
    case 5:
      return 0;
    case 6:
      return 1;    
    case 7:
      return 0;
    case 8:
      return 0;
    case 9:
      return 1;
  }
}

void updateNewConfig(){
  site_id = updateSiteId();
  for (int i = 0; i < USERS; i++)
  {
    rfid[i] = updateRFID(i);
    access[i] = updateAccess(i);
  }
}

void printConfig(){
  Serial.print("site_id = ");
  Serial.println(site_id);
  for (int i = 0; i < USERS; i++)
  {
    Serial.print("rfid = ");
    Serial.print(rfid[i]);
    Serial.print(" --- access = ");
    Serial.println(access[i]);
  }
}

void loop() {
  printConfig();  
  delay(10000);
  updateConfig();
  printConfig();
  delay(10000);
}
