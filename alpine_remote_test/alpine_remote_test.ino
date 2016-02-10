#define alpinePin 8 // alpine remote control pin to be connect to the tip of the lead that is plugged into headunit

void setup(){  
	pinMode(alpinePin, OUTPUT);
	Serial.begin(9600);
	
	bool aAlpVolUp[48]    = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,1,0,1,1,0,1,1,1,1,0,1,0,1,1,0,1,1,0,1,0,1,0,1};
	bool aAlpVolDn[48]    = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,1,0,1,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,0,1,0,1};
	bool aAlpMute[48]  		= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,1,0,1,1,1,1,0,1,1,1,0,1,1,0,1,0,1,0,1};
	bool aAlpPrstUp[48]   = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,0,1,0,1,1,1,1,1,0,1,1,1,1,0,1,0,1,0,1,0,1};
	bool aAlpPrstDn[48]   = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1};
	bool aAlpSource[48]   = {‭1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,0,1,0,1,0,1};
	bool aAlpTrckUp[48]   = {‭1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1};
	bool aAlpTrckDn[48]		= {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,0,1,1,1,0,1,1,1,1,1,1,0,1,0,1,1,0,1,0,1,0,1};
	bool aAlpPower[48]    = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,1,1,0,1,1,1,1,1,1,0,1,0,1,1,0,1,0,1,0,1,0,1};
	bool aAlpEntPlay[48]  = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1,0,1};
	bool aAlpBndPrg[48]   = {1,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,0,1,1,0,1,1,0,1,0,1,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1,0,1};
}

void loop(){
	if (Serial.available() > 0) {
		int inByte = Serial.read();
		// do something different depending on the character received.
		// The switch statement expects single number values for each case;
		// in this exmaple, though, you're using single quotes to tell
		// the controller to get the ASCII value for the character.  For
		// example 'a' = 97, 'b' = 98, and so forth:

		switch (inByte){
			case 'a':
				alpineCtrl(aAlpVolUp);
				break;
			case 'b':
				alpineCtrl(aAlpVolDn);
				break;
			case 'c':
				alpineCtrl(aAlpMute);
				break;
			case 'd':
				alpineCtrl(aAlpPrstUp);
				break;
			case 'e':
				alpineCtrl(aAlpSource);
				break;
			case 'f':
				alpineCtrl(aAlpPrstDn);
				break;
			case 'g':
				alpineCtrl(aAlpTrckUp);
				break;
			case 'h':
				alpineCtrl(aAlpTrckDn);
				break;
			case 'i':
				alpineCtrl(aAlpPower);
				break;
			case 'j':
				alpineCtrl(aAlpEntPlay);
				break;
			case 'k':
				alpineCtrl(aAlpBndPrg);
				break;
		}
	}

void alpineCtrl(bool aOutput[48]){
	// initialize the Alpine 1-wire bus with 8.0ms high and 4.5ms low
	// output the 6 command bytes over the defined Arduino pin
	// first iterate through the array bit mask
	// if bitwise AND resolves to true  -> output 1 (0.5ms high, 0.5ms low)
	// if bitwise AND resolves to false -> output 0 (1.0ms low)
	digitalWrite(alpinePin, HIGH);
	delayMicroseconds(8000);
	digitalWrite(alpinePin, LOW);
	delayMicroseconds(4500);
	for (int i = 0; i < 48; i++){
		if (aOutput[i] == 1){
			digitalWrite(alpinePin,HIGH);
			delayMicroseconds(500);
			digitalWrite(alpinePin,LOW);
			delayMicroseconds(500);
		}
		else { 						
			delayMicroseconds(1000);
		}
	}
}
