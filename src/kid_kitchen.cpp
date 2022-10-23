/*
Kid Kitchen re-brain
Remove the existing board, leds, and buttons from an 
IKEA DUKTIG stove unit and add an ESP32, buttons with leds, 
and a couple of WS2812B strips to light up the stove top and
the oven area.
*/

#include "ota.h"
#include "led_handling.h"
#include "button_handling.h"
#include "sound_handling.h"

#define LED 2  // Onboard LED fpr esp32-vroom-32 board

void setup() {
  Serial.begin(115200);

  ota_wifi_setup();

  enable_button_leds();
  
  mp3_setup();
  
  Serial.println(F("Enable ESP32 onboard LED for output"));
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
    // pinMode(my_switches[sw][3],OUTPUT);
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
  
  // zero out the Button arrays
  for(int i=0;i<num_switches;i++){
    s[i]=0;
    t[i]=0;
  }
  read_buttons = false;  // flag to tell the main loop to check the array
    
  setup_led_strips();
	
  Serial.println("Turn the button LEDs ON to half brightness");
  //button_led_set(128,128,128);

  button_leds_flash(4);
  /*
  int fif = files_in_folder(SNDS_FOLDER);
  Serial.print("Files in folder(");
  Serial.print(SNDS_FOLDER);
  Serial.print(") : ");
  Serial.println(fif);
  */
}

void loop() {
  //MP3_is_playing();

  // check for WiFi OTA updates
  ArduinoOTA.handle();
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
    
    int fif=MP3.readFileCountsInFolder(BTNS_FOLDER);
    int rn=random(1,fif+1);
    while(MP3_is_playing());
    Serial.printf("Files in folder: %i  Played: %i\n",fif,rn);
    MP3.playFolder(BTNS_FOLDER,rn);
    while(MP3_is_playing()){};
   // while(MP3.readState()==513){};

    // TODO: Act upon the button presses
    switch (sound_mode*1000+t[0]*100+t[1]*10+t[2]){
        case 1: // rgB
        //Serial.println(F("Say: Blue"));
        //CRGB c2u = CRGB::Blue;
        //leds_oven((void *) * c2u);
        sayColor(3,1); // Blue
        break;
       case 10: // rGb
        sayColor(2,1); // Green
        break;
       case 11: // rGB
        sayColor(5,1); // Cyan
        break;
      case 100: // Rgb
        sayColor(1,1); // Red
        break;
      case 101: // RgB
        sayColor(6,1); // Magenta
        break;
      case 110: // RGb
        sayColor(4,1); // Yellow
        break;
      case 111: // RGB
        sayColor(7,1); // White
        break;
      case 2: // rrggBB
        color_to_use = CRGB::White;
        Serial.println("Stove LEDs");
        // Kill the old task, if it is running
        /*
        if(stoveTaskHandle != NULL) {
          Serial.println(F("Killing stove task"));
          vTaskDelete(stoveTaskHandle);
        }
        */
        stove_leds_off();
        xTaskCreate(
          leds_stove,             // Function that should be called
          "Light up the 2 LEDs",  // Name of the task (for debugging)
          1000,                   // Stack size (bytes)
          (void *) &color_to_use, // Parameter to pass
          1,                      // Task priority
          &stoveTaskHandle        // Task handle
        );
        
        //leds_stove(color_to_use);
        MP3.playFolder(SNDS_FOLDER,6);  // cooking
        break;
       case 20: // rrGGbb
          nextLang();
          break;
       case 22: // rrGGBB
      case 200: // RRggbb
      case 202: // RRggBB
      case 220: // RRGGbb
        color_to_use = CRGB::Magenta;
        Serial.println("Oven LEDs");
        // Kill the old task, if it is running
        /*
        if(ovenTaskHandle != NULL) {
          Serial.println(F("Killing oven task"));
          vTaskDelete(ovenTaskHandle);
        }
        */
        oven_leds_off();
        xTaskCreate(
          leds_oven,             // Function that should be called
          "Light up the oven LEDs",  // Name of the task (for debugging)
          1000,                   // Stack size (bytes)
          (void *) &color_to_use, // Parameter to pass
          1,                      // Task priority
          &ovenTaskHandle        // Task handle
        );
        MP3.playFolder(SNDS_FOLDER,9);  // motor
        break;
      case 222: // RRGGBB 
        MP3.playFolder(SNDS_FOLDER,10); // ants
                      color_to_use = CRGB::Red;
        Serial.println("Oven LEDs");
        // Kill the old task, if it is running
        /*
        if(ovenTaskHandle != NULL) {
          Serial.println(F("Killing oven task"));
          vTaskDelete(ovenTaskHandle);
        }
        */
        oven_leds_off();
        xTaskCreate(
          leds_oven,             // Function that should be called
          "Light up the oven LEDs",  // Name of the task (for debugging)
          1000,                   // Stack size (bytes)
          (void *) &color_to_use, // Parameter to pass
          1,                      // Task priority
          &ovenTaskHandle        // Task handle
        );
        //while(!MP3_is_playing()){};
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
      Lang = Lang + 1;
      if(Lang > 2){
        Lang = 1;
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
  Serial.print(Lang);
  Serial.print(" File: ");
  Serial.println(soundToPlay);
  // Serial.print("Playing File ");
  // Serial.println(soundToPlay+1+(Lang * 7 -7));
  // MP3.play(soundToPlay+1+(Lang * 7 -7));
  MP3.playFolder(Lang, soundToPlay);
}

// void specialPressHandler (BfButton *btn, BfButton::press_pattern_t pattern) {
//   Serial.print(btn->getID());
//   Serial.println(" is special!!");
// }
*/

