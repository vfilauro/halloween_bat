#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_NeoPixel.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>


//#define DEBUG
#define ServoPin D3
#define MotionSensorPin D5 
#define MOTION_COUNT_TRIGGER 1 //how mny times motion is to be detected in order to trigger an action
#define LedPin D6
#define NUMLEDS 0
#define MAX_STEPS_IN_SEQ 10


struct sequence {
	byte no_of_steps;
	byte audioclip;
	byte col_ON[3];
	byte col_SBY[3];
	byte pos[MAX_STEPS_IN_SEQ];
	byte del[MAX_STEPS_IN_SEQ];	//in 10's of milliseconds (e.g. 120 is equal to 1.2 sec - or 1200ms)
};


SoftwareSerial mySoftwareSerial(D2, D1); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
Servo myServo;

#define NO_OF_SEQS 1
sequence bat[NO_OF_SEQS] =
{
	{
		3,
		2,
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 180,	180, 0 },
		{ 255,	255, 255 }
	}
};


// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)


// Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMLEDS, LedPin, NEO_GRB + NEO_KHZ800);


// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup()
{
	mySoftwareSerial.begin(9600);

#ifdef DEBUG
	Serial.begin(115200);
	Serial.println();
	Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
#endif


	while (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
#ifdef DEBUG
		Serial.println(F("Unable to begin:"));
		Serial.println(F("1.Please recheck the connection!"));
		Serial.println(F("2.Please insert the SD card!"));
#endif
		delay(1000);
	}


#ifdef DEBUG
	Serial.println(F("DFPlayer Mini online."));
#endif

	myDFPlayer.volume(20);  //Set volume value. From 0 to 30
	myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

#ifdef DEBUG
	//----Read information----
	Serial.print("State: ");
	Serial.println(myDFPlayer.readState()); //read mp3 state
	Serial.print("Volume: ");
	Serial.println(myDFPlayer.readVolume()); //read current volume
	Serial.print("EQ: ");
	Serial.println(myDFPlayer.readEQ()); //read EQ setting
	Serial.print("File Counts: ");
	Serial.println(myDFPlayer.readFileCounts()); //read all file counts in SD card
	Serial.print("Current File Number: ");
	Serial.println(myDFPlayer.readCurrentFileNumber()); //read current play file number
	Serial.print("File Counts in Folder 1: ");
	Serial.println(myDFPlayer.readFileCountsInFolder(1)); //read fill counts in folder SD:/03




	Serial.println(F("Initializing Motion Sensor"));
#endif


	pinMode(MotionSensorPin, INPUT);

#ifdef DEBUG
	Serial.println(F("Initializing Servo"));
#endif

	myServo.attach(ServoPin);
	myServo.write(0);              // reset servo position
	delay(15);                       // waits 15ms for the servo to reach the position  
//	strip.begin();
//	strip.show(); // Initialize all pixels to 'off'

	randomSeed(analogRead(A0)); //initialize random number generator


}





void loop() {
	static byte motion_count = 0;
	static byte seq_count;
	byte active_seq;
	int motion = digitalRead(MotionSensorPin); // Read digital OUT value

	if (motion) {
		motion_count++;
	}
	else
		motion_count = 0;


	if (motion_count == MOTION_COUNT_TRIGGER)
	{
		//active_seq = random(0, NO_OF_SEQS);
		//active_seq = seq_count++%NO_OF_SEQS;
		active_seq = 0;

		myDFPlayer.playFolder(1, bat[active_seq].audioclip);  //play specific mp3 in SD:/15/004.mp3; Folder Name(1~99); File Name(1~255)    

//		strip.setPixelColor(0, strip.Color(bat[active_seq].col_ON[0], bat[active_seq].col_ON[1], bat[active_seq].col_ON[2]));
//		strip.setPixelColor(1, strip.Color(bat[active_seq].col_ON[0], bat[active_seq].col_ON[1], bat[active_seq].col_ON[2]));
//		strip.show(); // This sends the updated pixel color to the hardware.
		for (int i = 0; i < bat[active_seq].no_of_steps - 1; i++)
		{
			myServo.write(bat[active_seq].pos[i]);              // tell servo to go to position in variable 'pos'
			delay(bat[active_seq].del[i] * 10);
		}

//		strip.setPixelColor(0, strip.Color(bat[active_seq].col_SBY[0], bat[active_seq].col_SBY[1], bat[active_seq].col_SBY[2]));
//		strip.setPixelColor(1, strip.Color(bat[active_seq].col_SBY[0], bat[active_seq].col_SBY[1], bat[active_seq].col_SBY[2]));
//		strip.show(); // This sends the updated pixel color to the hardware.
		myServo.write(bat[active_seq].pos[bat[active_seq].no_of_steps - 1]);              // tell servo to go to position in variable 'pos'
		delay(bat[active_seq].del[bat[active_seq].no_of_steps - 1] * 10);

		myServo.write(0);
//		strip.setPixelColor(0, strip.Color(0, 0, 0));
//		strip.setPixelColor(1, strip.Color(0, 0, 0));
//		strip.show(); // This sends the updated pixel color to the hardware.
		delay(6000);

	}
}


#ifdef DEBUG

void DFPlayer_printDetail(uint8_t type, int value) {
	switch (type) {
	case TimeOut:
		Serial.println(F("Time Out!"));
		break;
	case WrongStack:
		Serial.println(F("Stack Wrong!"));
		break;
	case DFPlayerCardInserted:
		Serial.println(F("Card Inserted!"));
		break;
	case DFPlayerCardRemoved:
		Serial.println(F("Card Removed!"));
		break;
	case DFPlayerCardOnline:
		Serial.println(F("Card Online!"));
		break;
	case DFPlayerPlayFinished:
		Serial.print(F("Number:"));
		Serial.print(value);
		Serial.println(F(" Play Finished!"));
		break;
	case DFPlayerError:
		Serial.print(F("DFPlayerError:"));
		switch (value) {
		case Busy:
			Serial.println(F("Card not found"));
			break;
		case Sleeping:
			Serial.println(F("Sleeping"));
			break;
		case SerialWrongStack:
			Serial.println(F("Get Wrong Stack"));
			break;
		case CheckSumNotMatch:
			Serial.println(F("Check Sum Not Match"));
			break;
		case FileIndexOut:
			Serial.println(F("File Index Out of Bound"));
			break;
		case FileMismatch:
			Serial.println(F("Cannot Find File"));
			break;
		case Advertise:
			Serial.println(F("In Advertise"));
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

#endif
