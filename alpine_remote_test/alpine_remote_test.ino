// Alpine remote control emulator
// Robin Liebl / sda2 for e46fanatics.de
// Feel free to use, edit and optimize this code!

#define pAlpineRemote 8 // Alpine remote control pin to be connect to the tip of the lead that is plugged into headunit

void setup(){
	// setup Arduino pins
	pinMode(pAlpineRemote, OUTPUT);
	// setup serial com port
	Serial.begin(9600);
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
}

void loop(){
	if (Serial.available() > 0) {
		int inByte = Serial.read();
		switch (inByte){
			case 'a':
				fAlpineCtrl(aAlpVolUp);
				break;
			case 'b':
				fAlpineCtrl(aAlpVolDn);
				break;
			case 'c':
				fAlpineCtrl(aAlpMute);
				break;
			case 'd':
				fAlpineCtrl(aAlpPrstUp);
				break;
			case 'e':
				fAlpineCtrl(aAlpSource);
				break;
			case 'f':
				fAlpineCtrl(aAlpPrstDn);
				break;
			case 'g':
				fAlpineCtrl(aAlpTrckUp);
				break;
			case 'h':
				fAlpineCtrl(aAlpTrckDn);
				break;
			case 'i':
				fAlpineCtrl(aAlpPower);
				break;
			case 'j':
				fAlpineCtrl(aAlpEntPlay);
				break;
			case 'k':
				fAlpineCtrl(aAlpBndPrg);
				break;
		}
	}
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
