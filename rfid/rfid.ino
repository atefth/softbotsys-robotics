#include <Wiegand.h>
WIEGAND wg;

#define GRANTED 4419486

long code;

void setup() {
	Serial.begin(19200);
	wg.begin();
}
void loop() {
	if(wg.available())
	{		
        code = wg.getCode();
		if (code == GRANTED)
		{
        	Serial.println(code);
			Serial.println("Granted");
		}else{
            Serial.println(code);
            Serial.println("Denied");
        }
	}
}






