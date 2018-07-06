//https://github.com/mathertel/OneButton
//http://www.mathertel.de/Arduino/OneButtonLibrary.aspx

//Libraries
#include <FreqCount.h>
#include <LiquidCrystal.h>
#include <OneButton.h>
#include <EEPROM.h>

//Floating Map Function
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//THC A-D Input must be on pin 5 on Uno or pin 13 on Teensy 3.1
//OneButton pin need swapped if changing between Uno and Teensy

  //LCD Pins
  #define LCD_RS      12
  #define LCD_EN      11
  #define LCD_D4       6
  #define LCD_D5       4
  #define LCD_D6       8
  #define LCD_D7       7
  #define LCD_CHARS   20
  #define LCD_LINES    4

  //Encoder Pins
  int encoderPinA = 3;
  int encoderPinB = 2;

  //New OneButton
  OneButton button(5,true);

//LCD Pin Setup and Custom Character
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
uint8_t testChar[8] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}; // Custom char


//Variable Setup
int16_t last, value, testval, tipval, TargetVal, EncoderValue, TipEncoder, HysEncoder, TargetHys, PrevEncoder;
bool flash; //Bool value for flashing text on screen
unsigned long startMillis, torchMillis, currentMillis, count, codeMillis, codeMillis2;  //some global variables available anywhere in the program
long int frq;
float TorchVal, PrevTorch;
int ButtonPress, tUp, tDown, tOff;


int lastEncoded;
int TorchHysteresis; //Voltage offset from target before outputs activate




void setup() {

    FreqCount.begin(10);

    Serial.begin(9600);

  //Variable setup
    flash = false;
    lastEncoded = 0;
    ButtonPress = 0;
		tUp = 0; 
		tDown = 0; 
		tOff = 0; 


    TargetVal = EEPROM.read(0);
    TipEncoder = TargetVal;
    TargetHys = EEPROM.read(1);
    HysEncoder = TargetHys;

    EncoderValue = TipEncoder; //Start with TipEncoder since it is first in button statement

  //Timer setup
    startMillis = millis();
    torchMillis = millis();
    
    
  //Button setup
    button.attachClick(ClickFunction); //Create Click function for button
    button.attachDoubleClick(DoubleClickFunction); //Create Double Click function for button
    button.attachLongPressStart(HoldFunction); //Create Hold function for button
  
  
  //Output setup
    pinMode(9, OUTPUT); //Torch Up output
    digitalWrite(9, LOW); //Turn output off    
    pinMode(10, OUTPUT); //Torch Down output
    digitalWrite(10, LOW); //Turn output off
  
  
  //LCD Setup
    lcd.begin(LCD_CHARS, LCD_LINES);
    lcd.clear();
    lcd.createChar(0, testChar); //Create custom full block char and send to LCD

    
  //LCD startup message
    lcd.setCursor(0,0);
    lcd.print("CNC Plasma Torch");
    lcd.setCursor(0,1);
    lcd.print("Height Control");
    lcd.setCursor(0,2);
    lcd.print("Created by");
    lcd.setCursor(0,3);
    lcd.print("Tyler Bennett");
    delay(5000);
    lcd.clear();

    
  //LCD text setup
    lcd.setCursor(0, 0);
    lcd.print("Tip Voltage:");
		lcd.setCursor(13,0);
    lcd.print("-----");
    lcd.setCursor(0, 1);
    lcd.print("Tip Target:");
    lcd.setCursor(12, 1);
    lcd.print(TargetVal); //Write initial TargetVal, updated by button press
    lcd.setCursor(0, 2);
    lcd.print("Hysteresis:");
    lcd.setCursor(12, 2);
    lcd.print(TargetHys); //Write initial TargetHys, updated by button press
    lcd.setCursor(0,3);
    lcd.print("UP:");
    lcd.setCursor(6,3);
    lcd.print("Down:");
    lcd.setCursor(14,3);
    lcd.print("Off:");

    
  //Encoder Setup
    pinMode(encoderPinA, INPUT_PULLUP); 
    pinMode(encoderPinB, INPUT_PULLUP);

    //get starting position
    int lastMSB = digitalRead(encoderPinA);
    int lastLSB = digitalRead(encoderPinB);

    //let start be lastEncoded so will index on first click
    lastEncoded = (lastMSB << 1) |lastLSB;
  
}//Setup

void loop() {  
  
	//codeMillis = micros();

	currentMillis = millis(); //Grab current loop time, millis used to not delay code execution

	button.tick(); //Check button for input

	if (FreqCount.available()) {
		TorchVal = mapfloat(FreqCount.read(), 1197, 9265, 0.0, 10.0);
		TorchVal = TorchVal*50; 
		if (currentMillis - torchMillis >= 250 && TorchVal != PrevTorch) { //Delay printing of TorchVal to make it easier to read and dont print if same as previous value
			torchMillis = millis();
			PrevTorch = TorchVal;
			lcd.setCursor(13,0);
			lcd.print("       ");
			lcd.setCursor(13,0);
			if (TorchVal <= 25){
				lcd.print("-----");
			} //End If
			else{
				lcd.print(TorchVal,1);
			} //End Else
		} //End If
	} //End If

	if (ButtonPress == 1 || ButtonPress == 2) {
		int MSB = digitalRead(encoderPinA); //MSB = most significant bit
		int LSB = digitalRead(encoderPinB); //LSB = least significant bit
		int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
		int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
		if(sum == 0b1101 || sum == 0b0010)EncoderValue ++;  
		if(sum == 0b1110 || sum == 0b0001)EncoderValue --;
		lastEncoded = encoded; //store this value for next time
	} //End If

	if (ButtonPress == 1) {

		if(EncoderValue != TargetVal){
			if(EncoderValue <= 100)EncoderValue = 100; //Don't let EncoderValue below 100
			if(EncoderValue >= 150)EncoderValue = 150; //Don't let EncoderValue above 150
			if(EncoderValue != PrevEncoder) {
				lcd.setCursor(12, 1);
				lcd.print("        ");
				lcd.setCursor(12, 1);
				lcd.print(EncoderValue);
				PrevEncoder = EncoderValue;
			} //End If
		} //End If
		else if(EncoderValue != PrevEncoder){
			lcd.setCursor(12, 1);
			lcd.print("        ");
			lcd.setCursor(12, 1);
			lcd.print(EncoderValue);
			PrevEncoder = EncoderValue;
		} //End Else If

		//Begin code for flashing Tip Target text
		if (currentMillis - startMillis >= 500 && flash) { //Hide Tip Target text
			startMillis = millis();
			lcd.setCursor(0, 1);
			lcd.print("           ");
			flash = false;
		} //End If
		else if(currentMillis - startMillis >= 300 && !flash) { //Show Tip Target text
			startMillis = millis();
			lcd.setCursor(0, 1);
			lcd.print("Tip Target:");
			flash = true;
		} //End Else If

	}
	else if (ButtonPress ==2){

		if(EncoderValue != TargetHys){
			if(EncoderValue <= 0)EncoderValue = 0; //Don't let EncoderValue below 0
			if(EncoderValue >= 15)EncoderValue = 15; //Don't let EncoderValue above 15
			if(EncoderValue != PrevEncoder) {
				lcd.setCursor(12, 2);
				lcd.print("        ");
				lcd.setCursor(12, 2);
				lcd.print(EncoderValue);
				PrevEncoder = EncoderValue;
			} //End If
		} //End If
		else if(EncoderValue != PrevEncoder) {
			lcd.setCursor(12, 2);
			lcd.print("        ");
			lcd.setCursor(12, 2);
			lcd.print(EncoderValue);
			PrevEncoder = EncoderValue;
		} //End Else If

		//Begin code for flashing Tip Target text
		if (currentMillis - startMillis >= 500 && flash) { //Hide Tip Target text
			startMillis = millis();
			lcd.setCursor(0, 2);
			lcd.print("           ");
			flash = false;
		} //End If
		else if(currentMillis - startMillis >= 300 && !flash) { //Show Tip Target text
			startMillis = millis();
			lcd.setCursor(0, 2);
			lcd.print("Hysteresis:");
			flash = true;
		} //End Else If

	}

	//Code for moving torch based on tip value compared to requested value, add outputs here.
	if(TorchVal < (TargetVal - TargetHys) && TorchVal >= 100){ //Voltage too low, raise torch
		//Turn Down off, Up on
		digitalWrite(10, LOW);       
		digitalWrite(9, HIGH);       
		//Print Square
		if(tUp != 1) { //Test if square has already been printed
			tUp = 1; 
			tDown = 0; 
			tOff = 0; 
			lcd.setCursor(4,3);
			lcd.print((char)0);
			//Clear Other Squares
			lcd.setCursor(12,3);
			lcd.print(" ");
			lcd.setCursor(19,3);
			lcd.print(" ");
		} //End If
	} //End If
	else if(TorchVal > (TargetVal + TargetHys) && TorchVal <= 150){ //Voltage too high, lower torch
		//Turn Up off, Down on
		digitalWrite(9, LOW);       
		digitalWrite(10, HIGH); 
		//Print Square
		if(tDown != 1) { //Test if square has already been printed
			tUp = 0; 
			tDown = 1; 
			tOff = 0; 
			lcd.setCursor(12,3);
			lcd.print((char)0);
			//Clear Other Squares
			lcd.setCursor(19,3);
			lcd.print(" ");
			lcd.setCursor(4,3);
			lcd.print(" ");
		}
	} //End Else If
	else { //Voltage Stable - Outputs Off
		//Turn both outputs off
		digitalWrite(10, LOW);       
		digitalWrite(9, LOW); 
		//Print Square
		if(tOff != 1) { //Test if square has already been printed
			tUp = 0; 
			tDown = 0; 
			tOff = 1; 
			lcd.setCursor(19,3);
			lcd.print((char)0);
			//Clear Other Squares
			lcd.setCursor(4,3);
			lcd.print(" ");
			lcd.setCursor(12,3);
			lcd.print(" ");
		}
	} //End Else

    codeMillis2 = micros();;

    //Serial.println(codeMillis2 - codeMillis);
	
}// Loop

  
void ClickFunction() { //Runs on single click of encoder button
	if(ButtonPress == 3)ButtonPress = 0; //Loop back around

	if(ButtonPress == 1){
		if(EncoderValue != TargetVal){//Encoder was changed, 
			TargetVal = EncoderValue;
			TipEncoder = EncoderValue;
			ButtonPress = -1;
			lcd.setCursor(12, 1);
			lcd.print(TargetVal);
		}
		else {
			Serial.print("HysEncoder Set: ");
			Serial.println(HysEncoder);
			EncoderValue = HysEncoder; //Swap to HysEncoder so it is ready for next button press
			lcd.setCursor(12, 1);
			lcd.print("        ");
			lcd.setCursor(12, 1);
			lcd.print(TargetVal);
		}
	}

	if(ButtonPress == 2){   
		if(EncoderValue != TargetHys){
			TargetHys = EncoderValue;
			HysEncoder = EncoderValue;
			ButtonPress = -1;
			lcd.setCursor(12, 2);
			lcd.print(TargetHys);
		}
		else {
			lcd.setCursor(12, 2);
			lcd.print("        ");
			lcd.setCursor(12, 2);
			lcd.print(TargetHys);
		}
		EncoderValue = TipEncoder;
	}
	//Write back to LCD in case lines were blank when exiting flashing statement
	lcd.setCursor(0, 1);
	lcd.print("Tip Target:");
	lcd.setCursor(0, 2);
	lcd.print("Hysteresis:");
	ButtonPress++;
}

void DoubleClickFunction() { //Runs on double click of encoder button
	if(ButtonPress == 1) {
		EncoderValue = EEPROM.read(0);
		lcd.setCursor(12, 1);
		lcd.print("        ");
		lcd.setCursor(12, 1);
		lcd.print(EncoderValue);
	}
	if(ButtonPress == 2) {
		EncoderValue = EEPROM.read(1);
		lcd.setCursor(12, 2);
		lcd.print("        ");
		lcd.setCursor(12, 2);
		lcd.print(EncoderValue);
	}
}

void HoldFunction() { //Runs on hold of encoder button
	if(ButtonPress == 1) {
		EEPROM.write(0,0); //Clear EEPROM before write, maybe unnecessary
		EEPROM.write(0, TargetVal); //Write current TargetVal to EEPROM 0
		lcd.setCursor(12, 1);
		lcd.print("        ");
		lcd.setCursor(12, 1);
		lcd.print("SAVED");
	}
	if(ButtonPress == 2) {
		EEPROM.write(1,0); //Clear EEPROM before write, maybe unnecessary
		EEPROM.write(1, TargetHys); //Write current TargetVal to EEPROM 0
		lcd.setCursor(12, 2);
		lcd.print("        ");
		lcd.setCursor(12, 2);
		lcd.print("SAVED");
	}
}

