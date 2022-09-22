#include <ez_switch_lib.h>

// Button definitions
const unsigned int btnPinR = 26;
const unsigned int ledPinR = 25;

const unsigned int btnPinG = 32;
const unsigned int ledPinG = 33;

const unsigned int btnPinB = 14;
const unsigned int ledPinB = 27;

int     interrupt_pin =  23;  // external interrupt pin
void IRAM_ATTR switch_ISR();

TaskHandle_t ButtonTaskHandle = NULL;

#define num_switches     3
//
// 'my_switches' layout.
// one row of data for each switch to be configured, as follows:
// [][0] = switch type
// [][1] = digital pin connected to switch
// [][2] = the switch_id provided by the add_switch function for the switch declared
// [][3] = the circuit type connecting the switch, here the switches
//         will have 10k ohm pull down resistors wired
byte my_switches[num_switches][5] =
{
  button_switch, btnPinR, 0, circuit_C2, ledPinR,
  button_switch, btnPinG, 0, circuit_C2, ledPinG,
  button_switch, btnPinB, 0, circuit_C2, ledPinB
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

void enable_button_leds(){
// Enable the LEDs on the switches
    Serial.println("Setting up button LEDs");
  /*
  pinMode(ledPinR,OUTPUT);
  pinMode(ledPinG,OUTPUT);
  pinMode(ledPinB,OUTPUT);
  */
    for (byte sw = 0; sw < num_switches; sw++) {
        pinMode(my_switches[sw][3],OUTPUT);
    }
}

void button_led_set(int redButton, int greenButton, int blueButton){
    analogWrite(my_switches[0][4], redButton);
    analogWrite(my_switches[1][4], greenButton);
    analogWrite(my_switches[2][4], blueButton);
}

void button_led_functions(void * data){
    int BLFunction = *(int *) data;
    Serial.print(F("Button function: "));
    Serial.println(BLFunction);
    int l=128;
    int h=255;
    switch(BLFunction){
        case 0: // turn off LEDs
            button_led_set(0,0,0);
            break;
        case 1:
            button_led_set(128,128,128);
            break;
        case 3:
            button_led_set(255,255,255);
            break;
        case 4:
            while(1){
                button_led_set(h,l,l);
                vTaskDelay(250);
                button_led_set(l,h,l);
                vTaskDelay(250);
                button_led_set(l,l,h);
                vTaskDelay(250);
            }
            break;
        case 5:
            while(1){
                button_led_set(h,h,h);
                vTaskDelay(250);
                button_led_set(0,0,0);
                vTaskDelay(250);
            }
            break;
    }
}
void fast_flash_button_leds(){
    for(int i=10;i>0;i--){
      Serial.println(i);
      button_led_set(128,0,0);
      delay(10+6*i);

      button_led_set(0,128,0);
      delay(10+6*i);

      button_led_set(0,0,128);
      delay(10+6*i);
    }
    button_led_set(0,0,0);
    vTaskDelete(NULL);
}

void button_leds_flash( int BLFunction ){
    Serial.println("Button LEDs functions");
    
    xTaskCreate(
        button_led_functions,    // Function that should be called
        "Button LEDs ",  // Name of the task (for debugging)
        1000,            // Stack size (bytes)
        &BLFunction,            // Parameter to pass
        1,               // Task priority
        &ButtonTaskHandle             // Task handle
    );

}
