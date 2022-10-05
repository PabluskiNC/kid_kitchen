
#include <DFRobotDFPlayerMini.h>

// MP3 definitions 
DFRobotDFPlayerMini MP3;
void printDetail(uint8_t type, int value);
int sound_mode = 0;
// 0 - colors;
// 1 - sound effects
// 2 -

int Lang=1; // 1 Eng; 2 Spa  ** must match folders in the SD card **
#define MAX_LANGUAGES 2

#define START_FOLDER 96
#define LANG_FOLDER 97
#define BTNS_FOLDER 98
#define SNDS_FOLDER 99

void mp3_setup(){
  // Setup the MP3-TF-16p player
  Serial2.begin(9600, SERIAL_8N1);  //Serial2.begin(9600);

  Serial.println(F("Initializing MP3 ... (May take 3~5 seconds)"));
  MP3.begin(Serial2);
  if (!MP3.begin(Serial2)) {
    Serial.println(F("ERROR ... restart the ESP32")); 
    fast_flash_button_leds();
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
  MP3.playFolder(START_FOLDER,1); // bootup sound
  while(MP3_is_playing());
}

int MP3_is_playing(){
  int i = MP3.readState();
  Serial.print(".");
  return ( i == 513 );
}

int files_in_folder(int folder){
  return(MP3.readFileCountsInFolder(folder));
}

void sayColor(int c, int w){
  CRGB color_to_use = CRGB::Black;
  switch(c){
    case 1:
      color_to_use=CRGB::Red;
      break;
    case 2:
      color_to_use=CRGB::Green;
      break;
    case 3:
      color_to_use=CRGB::Blue;
      break;
    case 4:
      color_to_use=CRGB::Yellow;
      break;
    case 5:
      color_to_use=CRGB::Cyan;
      break;
    case 6:
      color_to_use=CRGB::Magenta;
      break;
    case 7:
      color_to_use=CRGB::White;
      break;
  }
  Serial.print(F("Setting Oven LEDs to "));
  Serial.println(c);
  
  /*
  // Kill the old task, if it is running
  if(ovenTaskHandle != NULL) {
    Serial.println(F("Killing oven task"));
    vTaskDelete(ovenTaskHandle);
  }
  */
  stove_leds_off();
  xTaskCreate(
    leds_oven,                 // Function that should be called
    "Light up the oven LEDs",  // Name of the task (for debugging)
    1000,                      // Stack size (bytes)
    (void *) &color_to_use,    // Parameter to pass
    1,                         // Task priority
    &ovenTaskHandle            // Task handle
  );
  
  MP3.playFolder(Lang,c);
  //leds_oven(color_to_use);
  if(w>0){
    while(MP3_is_playing()){
      Serial.println(F("Still playing"));
    }; // wait until playing is done
  }
}

void nextLang(){
  Lang+=1;
  if(Lang>MAX_LANGUAGES){
    Lang=1;
  }
  Serial.print(F("Switching language to "));
  Serial.println(Lang);
  MP3.playFolder(LANG_FOLDER, Lang);
  //while(MP3_is_playing()){};
}