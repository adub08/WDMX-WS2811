//Written by Alex Woodmansey using Version 1.6.8
//########Untested######### Needs works, Just an idea really at this stage, was made for a 2 piece LED constume. 
#include "lib_dmx.h" //4 Universe DMX Library http://www.deskontrol.net/blog/arduino-four-universes-dmx-512-library/
#include "FastLED.h" //Fast LED Library https://github.com/FastLED/FastLED
#define    DMX512     (0)    // (250 kbaud - 2 to 512 channels) Standard USITT DMX-512
#define NUM_LEDS_HEAD 55 //Number of LED Pixels connected to head output
#define NUM_LEDS_BODY 130 //Number of LED Pixels connected to body output
#define DATA_PIN_HEAD 3  //Data Pin for LED Pixels for Head
#define DATA_PIN_BODY 6  //Data Pin for LED Pixels for Body
CRGB leds_head[NUM_LEDS_HEAD]; //Define LED Array for head
CRGB leds_body[NUM_LEDS_BODY]; //Define LED Array for body
#define COLOUR_ORDER GRB // LED colour order 

const int ledPin =  5;      // LED Pin 5 for DMX reception yes/no
const int disconnectPin = 2; // switch pin to disconnect wdmx
const int WDMX_button = A5;     // WDMX Button Pin
unsigned long now = 0; //current time value
const byte channelwidth = 3; //3 DMX channels per pixel
int channel; //channel number for each pixel RGB
volatile byte currentcounter = 0; //counter for DMX reception
byte previouscounter = 0; //counter for DMX reception 
byte current_hue; //hue value for rainbow effect
byte ledNumber_head; //current led Pixel being used
byte ledNumber_body; //current led Pixel being used 
volatile int val; //analog voltage from 



void setup() 
{ 
//  EEPROM.write(0, 1);
//  EEPROM.write(1, 64);
  ArduinoDmx0.attachRXInterrupt  (frame_received);  //set function called when all channels received
  ArduinoDmx0.set_control_pin (A2);    // control pin for max485   
  ArduinoDmx0.set_rx_address(1);      // set rx0 dmx start address
  ArduinoDmx0.set_rx_channels(512);     // number of DMX channel to receive 
  ArduinoDmx0.init_rx(DMX512);        // starts universe 0 as rx, NEW Parameter DMX mode
  pinMode (ledPin, OUTPUT);  //status LED set to Output 
  pinMode (WDMX_button, OUTPUT); //WDMX control button
  pinMode (disconnectPin, INPUT_PULLUP); //Disconnect interrupt switch
  digitalWrite(WDMX_button, HIGH); //WDMX control pin left high
  LEDS.setBrightness(255); //Set overall LED brightness
  FastLED.addLeds<WS2812, DATA_PIN_HEAD, COLOUR_ORDER>(leds_head, NUM_LEDS_HEAD); //initialize LED Pixel output for HEAD
  FastLED.addLeds<WS2812, DATA_PIN_BODY, COLOUR_ORDER>(leds_body, NUM_LEDS_BODY); //initialize LED Pixel output for BODY
  attachInterrupt(digitalPinToInterrupt(disconnectPin), disconnect_isr, LOW); //Hardware Interrupt pin for triggering WDMX disconnect 
  FastLED.setMaxPowerInVoltsAndMilliamps(5,2700);  //#experiemental# sets max power at 13.5w for LEDS - Needs latest Libary 
  

  post(); //run power on self test function each time board resets 
  SYS_TEST(); //run's max power test 
}  //end setup()

void loop()
{
  if(currentcounter != previouscounter) //has the value changed?
  {
    now = millis(); //store the time since the value has increased 
    previouscounter = currentcounter; //set the previous value equal to the current value
  }
  
  if((millis() - now) > 5000) //is the time since the value changed greater than 5 seconds?
  {
    noDMX(); //run no DMX function
  }
  else
  {
   digitalWrite(ledPin, HIGH); //turn status LED on.  We are receiving the DMX signal.
   DMXProcess(); //run DMX function
  }
  
}  //end loop()

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
FastLED.clear(); //take all LEDs to 0

channel = 0; //reset DMX channel assignment to 0 each time through loop for teh HEAD output.
for(ledNumber_head = 0; ledNumber_head < NUM_LEDS_HEAD; ledNumber_head++) //Counter for pixel number
{ 
leds_head[ledNumber_head]= CRGB(ArduinoDmx0.RxBuffer[channel], ArduinoDmx0.RxBuffer[channel +1], ArduinoDmx0.RxBuffer[channel +2]); //assign DMX channels to RGB channel of each pixel on the HEAD 
channel +=channelwidth; //increase last DMX channel number by channel width
}

channel = 166; //reset DMX channel assignment to 196 each time through loop for the BODY output.
for(ledNumber_body = 56; ledNumber_body < NUM_LEDS_BODY; ledNumber_body++) //Counter for pixel number
{ 
leds_body[ledNumber_body]= CRGB(ArduinoDmx0.RxBuffer[channel], ArduinoDmx0.RxBuffer[channel +1], ArduinoDmx0.RxBuffer[channel +2]); //assign DMX channels to RGB channel of each pixel on the BODY
channel +=channelwidth; //increase last DMX channel number by channel width
}

  FastLED.show(); //Send data to pixels
  delay(60); //delay 60 miliseconds for DMX to catch up.
} 

void noDMX() //Function to run when no DMX signal is present
{
    digitalWrite(ledPin, LOW); //turn status LED off. We are no longer receiving the DMX signal.
    delay(500);


} //End noDMX

void SYS_TEST() //Will show battery voltage on a scale of 0-1023 over 0-NUM_LEDs [requires calabration]
{
  FastLED.clear(); //take all LEDs to 0
  LEDS.showColor(CRGB(255, 255, 255)); //turn all pixels on white
  delay(300);
  int val = analogRead(0); //reads battery Voltage
  float numLedsToLight = val /10  ; //510 is the number that you need to divide by for 20 pixels from 1024 analogue input but at 212 as 1.2v is 12v with the voltage divier
  FastLED.clear(); //take all LEDs to 0
 for(int led = 0; led < numLedsToLight; led++) { //Counts from LED 0 to the defined number of LEDs
  leds_head[led] = CRGB::Green; 
 }
  FastLED.show();
  delay(10000);
}

void post() //power on self test function.  Make sure all pixels are working and run through colors.
{
   LEDS.showColor(CRGB(255, 0, 0)); //turn all pixels on red
   delay(1000);
   LEDS.showColor(CRGB(0, 255, 0)); //turn all pixels on green
   delay(1000);
   LEDS.showColor(CRGB(0, 0, 255)); //turn all pixels on blue
   delay(1000);
   LEDS.showColor(CRGB(0, 0, 0)); //turn all pixels off
   
}

void disconnect_isr() //Disconnects WDMX module, just smulates a long button press, runs within ISR - could be nicer. 
{
  digitalWrite(WDMX_button,LOW);
  delayMicroseconds(30000);
  delayMicroseconds(30000);
  delayMicroseconds(30000);
  delayMicroseconds(30000);
  digitalWrite(WDMX_button,HIGH);
}
