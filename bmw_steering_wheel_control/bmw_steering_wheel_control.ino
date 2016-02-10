// BMW E39/E46 Arduino steering wheel controller
// Robin Liebl / sda2 for e46fanatics.de
// Feel free to use, edit and optimize this code!
   
// This sketch interprets the I-bus signals via a MCP2025 LIN bus receiver module.
// Depending on the pressed button on the steering wheel, the output is either an HID command to the USB port,
// or a signal to the steering wheel remote input of the Alpine headunit.

/* Possible buttons on the steering wheel and their function:
// 	LEFT PLUS	Alpine volume increase
// 	LEFT MINUS	Alpine volume decrease
// 	LEFT UP		HID next track
// 	LEFT DOWN	HID previous track
// 	LEFT R/T	HID tabulator
// 	LEFT VOICE	Alpine volume mute
// 	RIGHT PLUS	-
// 	RIGHT MINUS	-
// 	RIGHT SET	-
// 	RIGHT I/O	-
*/

/*Alpine remote control protocol:
	Volume Up	11010111 11011011 10101011 11011011 11010110 11010101 -> 
 	Volume Down	11010111 11011011 10101011 01101101 11110110 11010101 -> 
	Mute        11010111 11011011 10101011 10101101 11101110 11010101 -> 
	Preset Up   11010111 11011011 10101011 10101011 11101111 01010101 -> 
	Preset Down	11010111 11011011 10101011 01010101 11111111 01010101 -> 
 	Source      11010111 11011011 10101011 10110111 11011011 01010101 -> 
 	Next Track	11010111 11011011 10101011 10111011 11011010 11010101 -> 
 	Prev. Track 11010111 11011011 10101011 01011101 11111010 11010101 -> 
 	Power	    11010111 11011011 10101011 01110111 11101011 01010101 -> 
 	Enter/Play	11010111 11011011 10101011 01010111 11111101 01010101 -> 
 	Band/Prog	11010111 11011011 10101011 01101011 11110111 01010101 -> 
	
	bool aAlpVolUp[24]    = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,0,1,1,0,1,1,1,1,0,1,0,1,1,0,1,1,0,1,0,1,0,1};
	bool aAlpVolDn[24]    = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,0,1,0,1};
	bool aAlpMute[24]     = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,0,1,1,1,1,0,1,1,1,0,1,1,0,1,0,1,0,1};
	bool aAlpPrstUp[24]   = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,1,0,1,0,1,0,1,0,1};
	bool aAlpPrstDn[24]   = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,‭0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1};
	bool aAlpSource[24]   = {‭1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,0,1,0,1,0,1};
	bool aAlpTrckUp[24]   = {‭1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1};
	bool aAlpTrckPrev[24] = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,‭0,1,0,1,1,1,0,1,1,1,1,1,1,0,1,0,1,1,0,1,0,1,0,1};
	bool aAlpPower[24]    = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,‭0,1,1,1,0,1,1,1,1,1,1,0,1,0,1,1,0,1,0,1,0,1,0,1};
	bool aAlpEntPlay[24]  = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1};
	bool aAlpBndPrg[24]   = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1};
	
	Write to Alpine bus with alpineBusWrite(Array-Name)
*/

#include "SoftwareSerial.h"

#define mcpTxPin 4  // tx pin not used, defined for coding only
#define mcpRxPin 6  // rx pin for the reciever to be plugged into
#define alpinePin 8 // alpine remote control pin to be connect to the tip of the lead that is plugged into headunit

// set up a new serial connection for communicating with MCP2025 
// this also frees up Arduino's built in serial port for acting as an HID keyboard

SoftwareSerial mcpSerial =  SoftwareSerial(mcpRxPin, mcpTxPin);

void setup(){  
	// set up the input and output pins
	pinMode(mcpRxPin, INPUT);
	pinMode(mcpTxPin, OUTPUT);
	pinMode(alpinePin, OUTPUT);
	  
	// set up serial ports
	mcpSerial.begin(9600); // begin serial connection with MCP2025 receiver
	Serial.begin(9600);	 // begin serial connection over USB to the computer
}


void loop(){

	
	/*
	int code = '0'; // sets up an integer to store received code until it is processed
	START:
	code = mcpSerial.read();
	if (code == 01) {goto START;}
	if (code == 99) {alpineMute();}
	if (code == 98) {alpineVolUp();}
	if (code == 97) {alpineVolDown();}
	if (code == 96) {alpineSource();}
	else {goto START;}
	*/
}
  
void usbNextTrack(){
	Remote.next();  // send next track command
	Remote.clear(); // Prevent duplicate activation
}

void usbPrevTrack(){
	Remote.previous();  // send previous track command
	Remote.clear(); 	// Prevent duplicate activation
}

void alpineBusInit(){
	//send start command
	
	digitalWrite(alpinePin, HIGH);	// send 8.0ms high
	delayMicroseconds(8000);
	digitalWrite(alpinePin, LOW);	// send 4.5ms low
	delayMicroseconds(4500);
}

void alpineBusWrite(bool output[24]){
	
	
	
	
	digitalWrite(alpinePin, HIGH); //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);  //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 1000 micro seconds
}

void alpineBusExit(){
	// send end command 
	// end command binary 1010101

	digitalWrite(alpinePin, HIGH);
	delayMicroseconds(500);
	digitalWrite(alpinePin, LOW);
	delayMicroseconds(500);

	delayMicroseconds(1000);

	digitalWrite(alpinePin, HIGH);
	delayMicroseconds(500);
	digitalWrite(alpinePin, LOW);
	delayMicroseconds(500);

	delayMicroseconds(1000);

	digitalWrite(alpinePin, HIGH);
	delayMicroseconds(500);
	digitalWrite(alpinePin, LOW);
	delayMicroseconds(500);

	delayMicroseconds(1000);

	digitalWrite(alpinePin, HIGH);
	delayMicroseconds(500);
	digitalWrite(alpinePin, LOW);
	delayMicroseconds(500);
}

void alpineMute(){
 	// send mute command
	// binary for the mute command  1 0 1 0 1 1 0 1 1 1 1 0 1 1 1 0 1

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds
	// MUTE COMMAND SENT

	//SEND END COMMAND
	//end command binary 1010101

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds
	//END COMMAND SENT

	loop ();//COMMAND SENT TO HEADUNIT GO TO START TO LISTEN FOR FURTHER BUTTON PRESSED COMMANDS
}

void alpineVolUp(){
	//send start command
	//first send 8ms high
	digitalWrite(alpinePin, HIGH);
	delay(8);
	// send 4.5ms low
	digitalWrite(alpinePin, LOW);
	delayMicroseconds(4500);
	//start command sent

	//send volume up command
	//binary for volume up 11011011110101101

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds
	// volume up command send

	//SEND END COMMAND
	//end command binary 1010101

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds
	//END COMMAND SENT
	loop ();//COMMAND SENT TO HEADUNIT GO TO LOOP TO LISTEN FOR FURTHER BUTTON PRESSED COMMANDS
}

void alpineVolDown(){
	//send start command
	//first send 8ms high
	digitalWrite(alpinePin, HIGH);
	delay(8);
	// send 4.5ms low
	digitalWrite(alpinePin, LOW);
	delayMicroseconds(4500);
	//start command sent

	//send volume down command
	//binary for volume down 0 1 1 0 1 1 0 1 1 1 1 1 0 1 1 0 1

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds
	//volume down command sent
	//SEND END COMMAND
	//end command binary 1010101

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds
	//END COMMAND SENT
	loop() ;//COMMAND SENT TO HEADUNIT GO TO START TO LISTEN FOR FURTHER BUTTON PRESSED COMMANDS
}

void alpineSource(){
	//send start command
	//first send 8ms high
	digitalWrite(alpinePin, HIGH);
	delay(8);
	// send 4.5ms low
	digitalWrite(alpinePin, LOW);
	delayMicroseconds(4500);
	//start command sent

	//send source command
	//source command binary 1 0 1 1 0 1 1 1  1 1 0 1 1 0 1 1 0

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds
	//source command sent

	//SEND END COMMAND
	//end command binary 1010101

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds

	delayMicroseconds(1000);       //   0 leave the pin low for 100 micro seconds

	digitalWrite(alpinePin, HIGH);     //   1   sending high 
	delayMicroseconds(500);        //   1   for 500 microseconds
	digitalWrite(alpinePin, LOW);      //   1   sending low
	delayMicroseconds(500);        //   1   for 500 microseconds
	//END COMMAND SENT
	loop ();//COMMAND SENT TO HEADUNIT GO TO START TO LISTEN FOR FURTHER BUTTON PRESSED COMMANDS
}
