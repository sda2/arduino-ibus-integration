// BMW E39/E46 Arduino steering wheel controller
// Robin Liebl / sda2 for e46fanatics.de
// Feel free to use, edit and optimize this code!
   
// This sketch interprets the I-Bus signals via a MCP2025 LIN bus receiver module.
// Depending on the pressed button on the steering wheel, the output is either an HID command to the USB port,
// or a signal to the steering wheel remote input of the Alpine headunit.

/* Possible buttons on the steering wheel and their function:
	LEFT PLUS	Alpine volume increase
	LEFT MINUS	Alpine volume decrease
	LEFT UP		HID next track
	LEFT DOWN	HID previous track
	LEFT R/T	HID play/pause
	LEFT VOICE	Alpine volume mute
	RIGHT PLUS	-
	RIGHT MINUS	-
	RIGHT SET	-
	RIGHT I/O	-
*/

/* Alpine remote control protocol:
	Volume Up	11010111 11011011 10101011 11011011 11010110 11010101
 	Volume Down	11010111 11011011 10101011 01101101 11110110 11010101
	Mute        11010111 11011011 10101011 10101101 11101110 11010101
	Preset Up   11010111 11011011 10101011 10101011 11101111 01010101
	Preset Down	11010111 11011011 10101011 01010101 11111111 01010101
 	Source      11010111 11011011 10101011 10110111 11011011 01010101
 	Next Track	11010111 11011011 10101011 10111011 11011010 11010101
 	Prev. Track 11010111 11011011 10101011 01011101 11111010 11010101
 	Power	    11010111 11011011 10101011 01110111 11101011 01010101
 	Enter/Play	11010111 11011011 10101011 01010111 11111101 01010101
 	Band/Prog	11010111 11011011 10101011 01101011 11110111 01010101
	
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
	
	Write to Alpine bus with alpineCtrl(Array-Name)
*/

//#include "SoftwareSerial.h"
#define ibus Serial1
#define pMcpWake 2  // cable select pin connected to mcp2025_pin2 - must be pulled high for "OP mode" to send data
//#define pMcpRx 5  // transmit pin connected to mcp2025_pin5
//#define pMcpTx 6  // receive pin connected to mcp2025_pin6
#define pAlpineRemote 8 // alpine rc output pin connected to the tip of the TRS connector that is plugged into headunit

// Setup a software serial connection for communicating with MCP2025
// this also frees up Arduino's built in serial port for acting as an HID multimedia keyboard

//SoftwareSerial mcpSerial =  SoftwareSerial(pMcpRx, pMcpTx);

void setup(){  
	// setup the input and output pins
	pinMode(pMcpWake, OUTPUT);
	//pinMode(pMcpRx, INPUT);
	//pinMode(pMcpTx, OUTPUT);
	pinMode(pAlpineRemote, OUTPUT);
	  
	// setup serial ports
	//mcpSerial.begin(9600); // begin serial connection with MCP2025 transceiver
	ibus.begin(9600, SERIAL_8E1);	 // begin serial connection over USB to the computer
	
	// pull CS/LWAKE pin low
	digitalWrite(pMcpWake, LOW);
	
	// setup Alpine remote control codes
	bool aAlpVolUp[48]	= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,0,1,1,0,1,1,1,1,0,1,0,1,1,0,1,1,0,1,0,1,0,1};
	bool aAlpVolDn[48]	= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,0,1,0,1};
	bool aAlpMute[48]	= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,0,1,1,1,1,0,1,1,1,0,1,1,0,1,0,1,0,1};
	bool aAlpPrstUp[48]	= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,1,0,1,0,1,0,1,0,1};
	bool aAlpPrstDn[48]	= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1};
	bool aAlpSource[48]	= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,0,1,0,1,0,1};
	bool aAlpTrckUp[48]	= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1};
	bool aAlpTrckDn[48]	= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,0,1,0,1,1,0,1,0,1,0,1};
	bool aAlpPower[48]	= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,1,1,0,1,1,1,1,1,1,0,1,0,1,1,0,1,0,1,0,1,0,1};
	bool aAlpEntPlay[48]= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1};
	bool aAlpBndPrg[48]	= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1};
	
	// setup I-Bus message bytes
	byte bMflVolUp [6]	= {0x50,0x04,0x68,0x32,0x11,0x1F}; // steering wheel volume up
	byte bMflVolDn [6]	= {0x50,0x04,0x68,0x32,0x10,0x1E}; // steering wheel volume down
	byte bMflTrkPrv [6] = {0x50,0x04,0x68,0x3B,0x08,0x0F}; // steering wheel previous track
	byte bMflTrkNxt [6] = {0x50,0x04,0x68,0x3B,0x01,0x06}; // steering wheel next track
	byte bMflRt [6] 	= {0x50,0x03,0xC8,0x01,0x9A}; // steering wheel R/T
}


void loop(){
	// read I-Bus
	
	// switch / case based on I-Bus message 
	
	// loop that bitch
	
}
  
void fUsbHidPlayPause(){
	Remote.play();  // send play/pause command
	Remote.clear(); // Prevent duplicate activation
}
  
void fUsbHidNextTrack(){
	Remote.next();  // send next track command
	Remote.clear(); // Prevent duplicate activation
}

void fUsbHidPrevTrack(){
	Remote.previous();  // send previous track command
	Remote.clear(); 	// Prevent duplicate activation
}

void fAlpineCtrl(bool aAlpineCode[48]){
	// initialize the Alpine 1-wire bus with 8.0ms high and 4.5ms low
	// output the 6 command bytes over the defined Arduino pin
	// first iterate through the array bit mask
	// if bitwise AND resolves to true  -> output 1 (0.5ms high, 0.5ms low)
	// if bitwise AND resolves to false -> output 0 (1.0ms low)
	digitalWrite(pAlpineRemote, HIGH);
	delayMicroseconds(8000);
	digitalWrite(pAlpineRemote, LOW);
	delayMicroseconds(4500);
	for (int i = 0; i < 48; i++){
		if (aAlpineCode[i] == 1){
			digitalWrite(pAlpineRemote,HIGH);
			delayMicroseconds(500);
			digitalWrite(pAlpineRemote,LOW);
			delayMicroseconds(500);
		}
		else { 						
			delayMicroseconds(1000);
		}
	}
}