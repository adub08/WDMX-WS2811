#include <lib_dmx.h> //4 Universe DMX Library http://www.deskontrol.net/blog/arduino-four-universes-dmx-512-library/
#include "FastLED.h" //Fast LED Library https://github.com/FastLED/FastLED
#define    DMX512     (0)    // (250 kbaud - 2 to 512 channels) Standard USITT DMX-512
#define NUM_LEDS 64 //Number of LED Pixels connected
#define DATA_PIN 5  //Data Pin for LED Pixels
CRGB leds[NUM_LEDS]; //Define LED Array
#define COLOUR_ORDER RBG // LED colour order 

volatile boolean flag_on = false;
volatile boolean TEST_COUNT = true;
const int ledPin =  13;      // LED Pin 13 for DMX reception yes/no
unsigned long now = 0; //current time value
const byte channelwidth = 3; //3 DMX channels per pixel
byte channel; //channel number for each pixel RGB
volatile byte currentcounter = 0; //counter for DMX reception
byte previouscounter = 0; //counter for DMX reception 
byte current_hue; //hue value for rainbow effect
byte ledNumber; //current led Pixel being used 
volatile int val;


void setup() 
{ 
  ArduinoDmx0.attachRXInterrupt  (frame_received);  //set function called when all channels received
  ArduinoDmx0.set_control_pin (2);    // control pin for max485   
  ArduinoDmx0.set_rx_address(1);      // set rx0 dmx start address
  ArduinoDmx0.set_rx_channels(NUM_LEDS*channelwidth);     // number of DMX channel to receive 
  ArduinoDmx0.init_rx(DMX512);        // starts universe 0 as rx, NEW Parameter DMX mode
  pinMode (ledPin, OUTPUT);  //status LED set to Output 
  pinMode (4, INPUT_PULLUP);
  LEDS.setBrightness(255); //Set overall LED brightness
  FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS); //initialize LED Pixel output. Using WS2811 12 volt Pixels.
  post(); //run power on self test function each time board resets 
  
}  //end setup()

void loop()
{
  if (digitalRead(4) == LOW) 
  {
  SYS_TEST();
  }
  else{
  //Receiving DMX Timer
  if(currentcounter != previouscounter) //has the value changed?
  {
    now = millis(); //store the time since the value has increased 
    previouscounter = currentcounter; //set the previous value equal to the current value
  }
  
  if((millis() - now) > 5000) //is the time since the value changed greater than 5 seconds?
  {
   digitalWrite(ledPin, LOW); //turn status LED off. We are no longer receiving the DMX signal.  
    noDMX(); //run no DMX function
  }
  else
  {
   digitalWrite(ledPin, HIGH); //turn status LED on.  We are receiving the DMX signal.
   DMXProcess(); //run DMX function
  }
  
}}  //end loop()

// Custom ISR: fired when all DMX channels in one universe are received
void frame_received(uint8_t universe) 
{
  static uint8_t led;
  
  if (universe == 0) // USART0
  {
    currentcounter++;  //increase counter by 1 each time through 
  }
}  // end of ISR


void DMXProcess() //Function to process DMX data
{
channel = 0; //reset DMX channel assignment to 0 each time through loop. 
for(ledNumber = 0; ledNumber < NUM_LEDS; ledNumber++) //Counter for pixel number
{ 
leds[ledNumber]= CHSV(ArduinoDmx0.RxBuffer[channel], ArduinoDmx0.RxBuffer[channel +1], ArduinoDmx0.RxBuffer[channel +2]); //assign DMX channels to RGB channel of each pixel
channel +=channelwidth; //increase last DMX channel number by channel width
}
  FastLED.show(); //Send data to pixels
  delay(50); //delay 60 miliseconds for DMX to catch up.  Less delay will cause flicker. 
} 

void noDMX() //Function to run when no DMX signal is present
{
 // Flash First LED
    FastLED.clear(); //take all LEDs to 0
    leds[0] = CRGB::Red;
    FastLED.show();
    delay(500);
    // Now turn the LED off, then pause
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(500);


} //End noDMX

void SYS_TEST() //Will show battery voltage on a scale of 0-1023 over 0-NUM_LEDs [requires calabration]
{
 
  FastLED.clear(); //take all LEDs to 0
  fill_solid( leds, NUM_LEDS, CRGB(255,255,255));
  FastLED.show();
  delay(200);
  int val = analogRead(0);
  int numLedsToLight = map(val, 0, 1023, 0, NUM_LEDS);
  FastLED.clear(); //take all LEDs to 0
 for(int led = 0; led < numLedsToLight; led++) { 
  leds[led] = CRGB::Green; 
 }
  FastLED.show();
  while(digitalRead(4) == LOW){}
}

void post() //power on self test function.  Make sure all pixels are working and run through colors. This will run once each time the board is reset. 
{
   LEDS.showColor(CRGB(255, 0, 0)); //turn all pixels on red
   delay(1000);
   LEDS.showColor(CRGB(0, 255, 0)); //turn all pixels on green
   delay(1000);
   LEDS.showColor(CRGB(0, 0, 255)); //turn all pixels on blue
   delay(1000);
   LEDS.showColor(CRGB(0, 0, 0)); //turn all pixels off
}
