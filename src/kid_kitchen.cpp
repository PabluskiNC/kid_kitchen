/*
Kid Kitchen re-brain
Remove the existing board, leds, and buttons from an 
IKEA DUKTIG stove unit and add an ESP32, buttons with leds, 
and a couple of WS2812B strips to light up the stove top and
the oven area.
*/

#include <DFRobotDFPlayerMini.h>

//#include <BfButtonManager.h>
#include <BfButton.h>
#include <FastLED.h>

void printDetail(uint8_t type, int value);

int playLang=1; // 01 Eng; 02 Spa

const unsigned int btnPinR = 26;
const unsigned int ledPinR = 25;

const unsigned int btnPinG = 32;
const unsigned int ledPinG = 33;

const unsigned int btnPinB = 14;
const unsigned int ledPinB = 27;

DFRobotDFPlayerMini MP3;

BfButton btn1(BfButton::STANDALONE_DIGITAL, btnPinR, true, LOW);
BfButton btn2(BfButton::STANDALONE_DIGITAL, btnPinG, true, LOW);
BfButton btn3(BfButton::STANDALONE_DIGITAL, btnPinB, true, LOW);

void pressHandler (BfButton *btn, BfButton::press_pattern_t pattern);


// LED Strip config
CRGB color_to_use = CRGB::Black;

void leds_oven( void * data);
void leds_stove( void * data);

// How many leds in your strip(s) and which pin(s)?
#define OVEN_NUM_LEDS 28
#define OVEN_DATA_PIN 13
#define STOVE_NUM_LEDS 2
#define STOVE_DATA_PIN 12

// Define the array of leds
CRGB oven_leds[OVEN_NUM_LEDS];
CRGB stove_leds[STOVE_NUM_LEDS];



void setup() {
  Serial.begin(115200);

  // Setup the MP3-TF-16p player
  Serial2.begin(9600, SERIAL_8N1);  //Serial2.begin(9600);
  Serial.println(F("Initializing MP3 ... (May take 3~5 seconds)"));

  if (!MP3.begin(Serial2)) {
    Serial.println(F("ERROR")); 
      for(int i=10;i>0;i--){
        Serial.println(i);

        analogWrite(ledPinR,128);
        analogWrite(ledPinG,0);
        analogWrite(ledPinB,0);     
        delay(10+6*i);

        analogWrite(ledPinR,0);
        analogWrite(ledPinG,128);
        analogWrite(ledPinB,0);
        delay(10+6*i);

        analogWrite(ledPinR,0);
        analogWrite(ledPinG,0);
        analogWrite(ledPinB,128);
        delay(10+6*i);
      }
      analogWrite(ledPinR,0);
      analogWrite(ledPinG,0);
      analogWrite(ledPinB,0);
      ESP.restart();  // Have you tried turning it off and on again?
  }

  Serial.println(F("MP3 online."));
  MP3.volume(30); // From 0 to 30)
  MP3.setTimeOut(500); //Set serial communictaion time out 500ms
  MP3.disableLoopAll();
  MP3.EQ(DFPLAYER_EQ_NORMAL);
//  MP3.EQ(DFPLAYER_EQ_POP);
//  MP3.EQ(DFPLAYER_EQ_ROCK);
//  MP3.EQ(DFPLAYER_EQ_JAZZ);
//  MP3.EQ(DFPLAYER_EQ_CLASSIC);
//  MP3.EQ(DFPLAYER_EQ_BASS);

  MP3.playFolder(99,1); // bootup sound
  
  // Enable the LEDs on the switches
  Serial.println("Setting up button LEDs");
  pinMode(ledPinR,OUTPUT);
  pinMode(ledPinG,OUTPUT);
  pinMode(ledPinB,OUTPUT);

  btn1.onPress(pressHandler)
     .onDoublePress(pressHandler,300); // default timeout
     // .onPressFor(pressHandler, 1000); // custom timeout for 1 second
  btn2.onPress(pressHandler)
     .onDoublePress(pressHandler,300); // default timeout
     //.onPressFor(pressHandler, 1000); // custom timeout for 1 second
  btn3.onPress(pressHandler)
     .onDoublePress(pressHandler,300); // default timeout
     // .onPressFor(pressHandler, 1000); // custom timeout for 1 second
  
  // Setup the WS2812B leds
	FastLED.addLeds<WS2812B, OVEN_DATA_PIN,  GRB>(oven_leds,  OVEN_NUM_LEDS);
  FastLED.addLeds<WS2812B, STOVE_DATA_PIN, GRB>(stove_leds, STOVE_NUM_LEDS);
  FastLED.setBrightness(20);
	
  Serial.println("Turn the button LEDs ON to half brightness");
  analogWrite(ledPinR,256);
  analogWrite(ledPinG,256);
  analogWrite(ledPinB,256);
  
  delay(500);  
}


void loop() {
  
  // Read the buttons
  btn1.read();
  btn2.read();
  btn3.read();

  // Any messages from the MP3 player?
  if (MP3.available()) {
    printDetail(MP3.readType(), MP3.read()); //Print the detail message from DFPlayer to handle different errors and states.
    delay(5);
  }
   
}

void printDetail(uint8_t type, int value){
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
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      //playing = 0;
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

void pressHandler (BfButton *btn, BfButton::press_pattern_t pattern) {
  int buttonPressed = 0;
  int soundToPlay = 0;
  Serial.print("Button ");

  switch (btn->getID()) {
    case btnPinR: // Red
      buttonPressed=1;
      color_to_use=CRGB::Red;
      soundToPlay=1;
      break;
    case btnPinG: // Green
      buttonPressed=2;
      color_to_use=CRGB::Green;
      soundToPlay=2;
      break;
    case btnPinB: // Blue
      buttonPressed=3;
      color_to_use=CRGB::Blue;
      soundToPlay=3;
      break;
  }
  Serial.println(buttonPressed);

  switch (pattern) {
    case BfButton::SINGLE_PRESS:
      Serial.println(" pressed.");
        // leds_oven(CRGB::Blue);
      xTaskCreate(
        leds_oven,    // Function that should be called
        "Light up the LEDs",  // Name of the task (for debugging)
        1000,            // Stack size (bytes)
        (void *) &color_to_use,            // Parameter to pass
        1,               // Task priority
        NULL             // Task handle
      );
      break;
    case BfButton::DOUBLE_PRESS:
      playLang = playLang + 1;
      if(playLang > 2){
        playLang = 1;
      }
      color_to_use = CRGB::Red;
      Serial.println(" double pressed.");
      xTaskCreate(
        leds_stove,    // Function that should be called
        "Light up the 2 LEDs",  // Name of the task (for debugging)
        1000,            // Stack size (bytes)
        (void *) &color_to_use,            // Parameter to pass
        1,               // Task priority
        NULL             // Task handle
      );

      break;
    case BfButton::LONG_PRESS:
      Serial.println(" long pressed.");
      break;
  }

  MP3.stop();
  MP3.waitAvailable();

  Serial.print("Playing Lang: ");
  Serial.print(playLang);
  Serial.print(" File: ");
  Serial.println(soundToPlay);
  // Serial.print("Playing File ");
  // Serial.println(soundToPlay+1+(playLang * 7 -7));
  // MP3.play(soundToPlay+1+(playLang * 7 -7));
  MP3.playFolder(playLang, soundToPlay);
}

// void specialPressHandler (BfButton *btn, BfButton::press_pattern_t pattern) {
//   Serial.print(btn->getID());
//   Serial.println(" is special!!");
// }

void leds_oven( void * data ){
  Serial.println("leds_oven function");
  CRGB ledColor = *(CRGB *) data;
  int ledBright = 0;
  
  for(ledBright=0;ledBright<=100;ledBright=ledBright+1){
    //Serial.println(ledBright);
    FastLED.setBrightness(ledBright);
    fill_solid(oven_leds, OVEN_NUM_LEDS, ledColor);
    FastLED.show();
    delay(10);
  }
  
  for(ledBright=100;ledBright>=0;ledBright=ledBright-1){
    //Serial.println(ledBright);
    FastLED.setBrightness(ledBright);
    fill_solid(oven_leds, OVEN_NUM_LEDS, ledColor);
    FastLED.show();
    delay(10);
  }
  fill_solid(oven_leds, OVEN_NUM_LEDS, CRGB::Black);
  vTaskDelete(NULL);
}

void leds_stove( void * data ){
  Serial.println("leds_stove function");
  CRGB ledColor = *(CRGB *) data;
  int ledBright = 0;
  
  for(ledBright=0;ledBright<=255;ledBright=ledBright+1){
    //Serial.println(ledBright);
    FastLED.setBrightness(ledBright);
    fill_solid(stove_leds, STOVE_NUM_LEDS, ledColor);
    FastLED.show();
    delay(10);
  }
  delay(10000);
  
  for(ledBright=255;ledBright>=0;ledBright=ledBright-1){
    //Serial.println(ledBright);
    FastLED.setBrightness(ledBright);
    fill_solid(stove_leds, STOVE_NUM_LEDS, ledColor);
    FastLED.show();
    delay(10);
  }
  fill_solid(stove_leds, STOVE_NUM_LEDS, CRGB::Black);
  vTaskDelete(NULL);
}
