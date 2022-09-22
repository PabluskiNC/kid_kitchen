/* Handle the kid_kitchen led functions
*/
#define FASTLED_INTERNAL
#include <FastLED.h>

// LED Strip config
CRGB color_to_use = CRGB::Black;

//void leds_oven( void * data);
//void leds_stove( void * data);

void leds_oven( CRGB ledColor);
void leds_stove( CRGB ledColor);

// How many leds in your strip(s) and which pin(s)?
#define OVEN_NUM_LEDS 22
#define OVEN_DATA_PIN 13

#define STOVE_NUM_LEDS 2
#define STOVE_DATA_PIN 12
#define BLACK CRGB::Black

// Define the array of leds
CRGB oven_leds[OVEN_NUM_LEDS];
CRGB stove_leds[STOVE_NUM_LEDS];

TaskHandle_t ovenTaskHandle = NULL;
TaskHandle_t stoveTaskHandle = NULL;

void oven_leds_off(){
  fill_solid(stove_leds, STOVE_NUM_LEDS, BLACK);
  FastLED.show();
}

void stove_leds_off(){
  fill_solid(stove_leds, STOVE_NUM_LEDS, BLACK);
  FastLED.show();
}

void all_led_strips_off(){
  fill_solid(stove_leds, STOVE_NUM_LEDS, BLACK);
  fill_solid(oven_leds, OVEN_NUM_LEDS, BLACK);
  FastLED.show();
}

void setup_led_strips(){
    // Setup the WS2812B leds
  Serial.println(F("Setup the WS2812B strips"));
	FastLED.addLeds<WS2812B, OVEN_DATA_PIN,  GRB>(oven_leds,  OVEN_NUM_LEDS);
  FastLED.addLeds<WS2812B, STOVE_DATA_PIN, GRB>(stove_leds, STOVE_NUM_LEDS);
  FastLED.setBrightness(20);
  all_led_strips_off();
}
void leds_oven( void * data ){
  Serial.println("leds_oven function");
  CRGB ledColor = *(CRGB *) data;
  int ledBright = 0;
  
  for(ledBright=0;ledBright<=254;ledBright=ledBright+2){
    //Serial.println(ledBright);
    FastLED.setBrightness(ledBright);
    fill_solid(oven_leds, OVEN_NUM_LEDS, ledColor);
    FastLED.show();
    vTaskDelay(10);
    //delay(10);
  }
  
  for(ledBright=254;ledBright>=0;ledBright=ledBright-2){
    //Serial.println(ledBright);
    FastLED.setBrightness(ledBright);
    fill_solid(oven_leds, OVEN_NUM_LEDS, ledColor);
    FastLED.show();
    vTaskDelay(10);
    //delay(10);
  }
  fill_solid(oven_leds, OVEN_NUM_LEDS, CRGB::Black);
  FastLED.show();
  vTaskDelete(NULL);
  Serial.println(F("leds_oven function done"));
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
    vTaskDelay(10);
    //delay(10);
  }
  vTaskDelay(10000);
  //delay(1000);
  
  for(ledBright=255;ledBright>=0;ledBright=ledBright-1){
    //Serial.println(ledBright);
    FastLED.setBrightness(ledBright);
    fill_solid(stove_leds, STOVE_NUM_LEDS, ledColor);
    FastLED.show();
    vTaskDelay(10);
    //delay(10);
  }
  fill_solid(stove_leds, STOVE_NUM_LEDS, BLACK);
  FastLED.show();
  vTaskDelete(NULL);
  Serial.println(F("leds_stove function done"));
}

