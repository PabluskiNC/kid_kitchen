/*
Kid Kitchen re-brain
Remove the existing board, leds, and buttons from an 
IKEA DUKTIG stove unit and add an ESP32, buttons with leds, 
and a couple of WS2812B strips to light up the stove top and
the oven area.
*/

#include <DFRobotDFPlayerMini.h>
#include <FastLED.h>
#include <ez_switch_lib.h>

// ----------------------------------------------- \\

// MP3 definitions 
DFRobotDFPlayerMini MP3;
void printDetail(uint8_t type, int value);
int sound_mode = 0;
// 0 - colors;
// 1 - sound effects
// 2 -

int playLang=1; // 1 Eng; 2 Spa

// ----------------------------------------------- \\

// Button definitions
const unsigned int btnPinR = 26;
const unsigned int ledPinR = 25;

const unsigned int btnPinG = 32;
const unsigned int ledPinG = 33;

const unsigned int btnPinB = 14;
const unsigned int ledPinB = 27;


int     interrupt_pin =  23;  // external interrupt pin
void IRAM_ATTR switch_ISR();

#define LED 2  // Onboard LED fpr esp32-vroom-32 board

#define num_switches     3
//
// 'my_switches' layout.
// one row of data for each switch to be configured, as follows:
// [][0] = switch type
// [][1] = digital pin connected to switch
// [][2] = the switch_id provided by the add_switch function for the switch declared
// [][3] = the circuit type connecting the switch, here the switches
//         will have 10k ohm pull down resistors wired
byte my_switches[num_switches][4] =
{
  button_switch,  26, 0, circuit_C2,
  button_switch,  32, 0, circuit_C2,
  button_switch,  14, 0, circuit_C2,
};

hw_timer_t *My_timer = NULL;
void IRAM_ATTR onTimer();

volatile int s[num_switches];
volatile int t[num_switches];
volatile bool read_buttons = 0;
volatile bool timer_start_flag = 0;
volatile bool timer_running_flag = 0;
const long timer_length = 370000;

// Create the 'Switches' instance (ms) for the given number of switches
Switches ms(num_switches); 

// ----------------------------------------------- \\

// LED Strip config
CRGB color_to_use = CRGB::Black;

void leds_oven( void * data);
void leds_stove( void * data);

// How many leds in your strip(s) and which pin(s)?
#define OVEN_NUM_LEDS 22
#define OVEN_DATA_PIN 13

#define STOVE_NUM_LEDS 2
#define STOVE_DATA_PIN 12

// Define the array of leds
CRGB oven_leds[OVEN_NUM_LEDS];
CRGB stove_leds[STOVE_NUM_LEDS];

void setup() {
  Serial.begin(115200);

  // Enable the LEDs on the switches
  Serial.println("Setting up button LEDs");
  pinMode(ledPinR,OUTPUT);
  pinMode(ledPinG,OUTPUT);
  pinMode(ledPinB,OUTPUT);

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

  Serial.println(F("Play startup sound"));
  MP3.playFolder(99,1); // bootup sound
  
  Serial.println(F("Enable onboard LED for output"));
  pinMode(LED,OUTPUT);  // onboard LED 

  Serial.println(F("Setting up switches"));
  // Add all switches to library switch control structure
  // and link all to same interrupt pin as a linked output
  for (byte sw = 0; sw < num_switches; sw++) {
    my_switches[sw][2] = ms.add_switch(
        my_switches[sw][0],  // switch type
        my_switches[sw][1],  // digital pin switch is wired to
        my_switches[sw][3]); // type of circuit switch is wired as
    ms.link_switch_to_output(
      my_switches[sw][2], // switch id
      interrupt_pin,      // digital pin to link to for interrupt
      LOW);               // start with interrupt pin LOW, as interrupt will be triggered on RISING
  }
  // Now establish the common interrupt service routine (ISR) that
  // will be used for all declared switches
  Serial.println(F("Setting up GPIO interrupt"));
  attachInterrupt(
    digitalPinToInterrupt(interrupt_pin),
    switch_ISR, // name of the sketch's ISR function to handle switch interrupts
    RISING);    // trigger on a rising pin value
  
  Serial.println(F("Setting up TIMER interrupt"));
  My_timer = timerBegin(1, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, false);
  timerAlarmWrite(My_timer, timer_length, true);
  timerStop(My_timer);
  timerAlarmEnable(My_timer); //Just Enable
  
  for(int i=0;i<num_switches;i++){
    s[i]=0;
    t[i]=0;
  }
  read_buttons = false;
    
  // Setup the WS2812B leds
  Serial.println(F("Setup the WS2812B strips"));
	FastLED.addLeds<WS2812B, OVEN_DATA_PIN,  GRB>(oven_leds,  OVEN_NUM_LEDS);
  FastLED.addLeds<WS2812B, STOVE_DATA_PIN, GRB>(stove_leds, STOVE_NUM_LEDS);
  FastLED.setBrightness(20);
	
  Serial.println("Turn the button LEDs ON to half brightness");
  analogWrite(ledPinR,128);
  analogWrite(ledPinG,128);
  analogWrite(ledPinB,128);
  
}

void loop() {
  
   // Any messages from the MP3 player?
  if (MP3.available()) {
    printDetail(MP3.readType(), MP3.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }

// If timer is not yet running and GPIO interrupt happened then start timer
  if(timer_start_flag && ! timer_running_flag){
    if( !timerStarted(My_timer) ){
      //Serial.println(F("Start Timer"));
      digitalWrite(LED,1);
      timerStop(My_timer);
      timerAttachInterrupt(My_timer, &onTimer, false);
      timerAlarmDisable(My_timer);
      timerAlarmWrite(My_timer, timer_length, true);
      timerAlarmEnable(My_timer); 
      timerStart(My_timer);
      timer_running_flag=1;
    }
  }

  for (byte sw = 0; sw < num_switches; sw++) {
    ms.read_switch(my_switches[sw][2]); // my_switches[sw][2] is the switch id for switch sw
  }
  
  // Once the timer expires then we can read the button array
  if(read_buttons){
    Serial.printf("T1:%i T2:%i T3:%i\n",t[0],t[1],t[2]);
    Serial.flush();
    read_buttons = false;
    // play button sound
    
    MP3.playFolder(98,8);
    
    while(MP3.readState()==513){};

    // TODO: Act upon the button presses
    switch (sound_mode*1000+t[0]*100+t[1]*10+t[2]){
        case 1: // rgB
        MP3.playFolder(playLang,3); // Blue
        while(MP3.readState()==513){};
        MP3.playFolder(98,6);
        break;
       case 10: // rGb
        MP3.playFolder(playLang,2); // Green
        break;
       case 11: // rGB
        MP3.playFolder(playLang,5); // Cyan
        break;
      case 100: // Rgb
        MP3.playFolder(playLang,1); // Red
        break;
      case 101: // RgB
        MP3.playFolder(playLang,6); // Magenta
        break;
      case 110: // RGb
        MP3.playFolder(playLang,4); // Yellow
        break;
      case 111: // RGB
        MP3.playFolder(playLang,7); // White
        break;
        case 2: // rrggBB
       case 20: // rrGGbb
       case 22: // rrGGBB
      case 200: // RRggbb
      case 202: // RRggBB
      case 220: // RRGGbb
      case 222: // RRGGBB 
        MP3.playFolder(99,10); // ants
        break;
       case 3: // rrrgggBBB
       case 30: // rrrGGGbbb
       case 33: // rrrGGGBBB
      case 300: // RRRgggbbb
      case 303: // RRRgggBBB
      case 330: // RRRGGGbbb
      case 333: // RRRGGGBBB
      break;
    }
  }
}

// TODO: define some color shows

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// ISR for handling interrupt triggers arising from associated switches
// when they transition to on.  The routine knows which switch has generated
// the interrupt because the ez_switch_lib switch read functions record the
// actuated switch in the library variable 'last_switched_id'.
//
// The routine does nothing more than demonstrate the effectiveness of the 
// use of a single ISR handling multiple switches by using the serial monitor
// to confirm correct operation.
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void switch_ISR()
{
  //Serial.println(F("switch_ISR"));
  
  // Reset the interrupt pin to LOW, so that any other switch will fire the interrupt
  // whist one or more switches in transition stage
  
  byte switch_id = ms.last_switched_id;  // switch id of switch currently switched to on
  
  digitalWrite(ms.switches[switch_id].switch_out_pin, LOW);
    
  // For button switches only, reset the linked output pin status to LOW so that
  // it will trigger the interrupt at every press/release cycle.
  
  if (ms.switches[switch_id].switch_type == button_switch) {
    ms.switches[switch_id].switch_out_pin_status = LOW;
  }
    
  s[switch_id]+=1;
  /*
  Serial.print("** Interrupt triggered for switch id ");
  Serial.println(switch_id); // 'this_switch_id' is the id of the triggering switch
  Serial.flush();
  */
  timer_start_flag = 1;
} // end of switch_ISR

void IRAM_ATTR onTimer(){
  //Serial.println(F("Timer interrupt"));
  //Serial.flush();

  digitalWrite(LED,0);

  timerStop(My_timer);
  timerDetachInterrupt(My_timer);
  
  for(int i=0;i<3;i++){
    t[i] = s[i];
    s[i] = 0;
  }
  read_buttons = true;
  timer_start_flag = 0;
  timer_running_flag = 0;
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

/*
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
*/

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
