/*
  Make a mechanical rotary encoder into a PC volume control.
*/

#include "HID-Project.h"

#define PIN_LED LED_BUILTIN
// Mute switch
#define PIN_MUTE_BUTTON 2
// Encoder outputs. Switch the pin definitions if it seems to be 
// going backwards
#define PIN_ENCODER_A 3
#define PIN_ENCODER_B 4
#define BIT0  1
#define BIT1  2

// Detect pin state changes
static uint8_t rotaryOldState = 0;

/**
 * Read the output bits of the rotary encoder
 */
int getRotaryBits(){
  uint8_t bits = 0;
  if (!digitalRead(PIN_ENCODER_A)) {
    bits |= BIT0;
  }
  if (!digitalRead(PIN_ENCODER_B)) {
    bits |= BIT1;
  }
  return bits;
}


void setup() {
  // Blink LED when we are busy
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  pinMode(PIN_MUTE_BUTTON, INPUT_PULLUP);
  pinMode(PIN_ENCODER_A, INPUT_PULLUP);
  pinMode(PIN_ENCODER_B, INPUT_PULLUP);

  // get an initial reading on the encoder pins
  rotaryOldState = getRotaryBits();

  // Sends a clean report to the host. This is important on any Arduino type.
  Consumer.begin();
  digitalWrite(PIN_LED, LOW);

}


void loop() {
  // Default no button
  ConsumerKeycode usbAction = HID_CONSUMER_UNASSIGNED;
  uint8_t rotaryNewState = getRotaryBits();

  // Rotary encoder moved.
  if (rotaryNewState != rotaryOldState)
  {
    if (rotaryOldState == B00)
    { 
      // this is the positive-going edge
      if (rotaryNewState == B01) {
        usbAction = MEDIA_VOLUME_UP;
      }
      else if (rotaryNewState == B10) {
        usbAction = MEDIA_VOLUME_DOWN;
      }
    } else if (rotaryOldState == B11)
    {
      // this is the negative-going edge
      if (rotaryNewState == B01) {
        usbAction = MEDIA_VOLUME_DOWN;
      }
      else if (rotaryNewState == B10) {
        usbAction = MEDIA_VOLUME_UP;
      }
    }
    rotaryOldState = rotaryNewState;
  }

  if (!digitalRead(PIN_MUTE_BUTTON)) {
    usbAction = MEDIA_VOLUME_MUTE;
  }
  if (usbAction != HID_CONSUMER_UNASSIGNED) {
    digitalWrite(PIN_LED, HIGH);
    Consumer.write(usbAction);
    // Debounce. Longer for the mute button.
    switch (usbAction) {
      case MEDIA_VOLUME_MUTE:
        delay(300);
        break;
      case MEDIA_VOLUME_UP:
      case MEDIA_VOLUME_DOWN:
        delay(50);
        break;
        
    }
    digitalWrite(PIN_LED, LOW);
  }
}
