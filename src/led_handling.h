/* Handle the kid_kitchen led functions
*/

#include <FastLED.h>

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

void leds_oven( void * data ){
  Serial.println("leds_oven function");
  CRGB ledColor = *(CRGB *) data;
  int ledBright = 0;
  
  for(ledBright=0;ledBright<=254;ledBright=ledBright+2){
    //Serial.println(ledBright);
    FastLED.setBrightness(ledBright);
    fill_solid(oven_leds, OVEN_NUM_LEDS, ledColor);
    FastLED.show();
    delay(10);
  }
  
  for(ledBright=254;ledBright>=0;ledBright=ledBright-2){
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
