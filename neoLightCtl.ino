#include "test.h"
#define PIXELS 144  // Number of pixels in the string

int t=0;
char incomingCommand[30];
int incComOffset = 0;


void parseCommand() {

}

void parseSerial() {
	if (Serial.available()) {
		char incomingByte = Serial.read();
		incomingCommand[incComOffset]=incomingByte;
		incComOffset = (incComOffset + 1) % sizeof(incomingCommand);

		if (incomingByte == ';') for(
				int i = incComOffset+1;
				i != incComOffset;
				i = (i + 1) % sizeof(incomingCommand)
		) {
			Serial.print( incomingCommand[i] );
			incomingCommand[i] = '\0';
		}
	}
}

void displayLights() {
	t=(1+t)%PIXELS;
	for (int i = 0; i < PIXELS-2; i++) {
		if(i==t) {
			sendPixel(10,10,0);
			sendPixel(10,10,0);
			sendPixel(10,10,0);		
		} else
			sendPixel(0,3,3);
	}
}


void setup() {
	ledsetup();
	Serial.begin(9600);
}

void loop() {
	displayLights();	
	show();
	parseSerial();
}