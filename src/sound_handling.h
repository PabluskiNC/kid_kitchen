
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