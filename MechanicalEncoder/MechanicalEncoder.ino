/*
  Copyright (c) 2014-2015 NicoHood
  See the readme for credit to other people.

  Consumer example
  Press a button to play/pause music player

  You may also use SingleConsumer to use a single report.

  See HID Project documentation for more Consumer keys.
  https://github.com/NicoHood/HID/wiki/Consumer-API
*/

#include "HID-Project.h"

const int pinLed = LED_BUILTIN;
const int pinButton = 2;
#define PIN_ENCODER_A 3
#define PIN_ENCODER_B 4
#define BIT0  1
#define BIT1  2
#define BIT2  4
#define BIT3  8
#define BIT4  16

static uint8_t enc_prev_pos = 0;
static uint8_t enc_flags    = 0;

 

void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinButton, INPUT_PULLUP);
  pinMode(PIN_ENCODER_A, INPUT);
  pinMode(PIN_ENCODER_B, INPUT);
  digitalWrite(PIN_ENCODER_A, HIGH);
  digitalWrite(PIN_ENCODER_B, HIGH);
  Serial1.begin(9600);
  

  // get an initial reading on the encoder pins
  if (digitalRead(PIN_ENCODER_A) == LOW) {
    enc_prev_pos |= (1 << 0);
  }
  if (digitalRead(PIN_ENCODER_B) == LOW) {
    enc_prev_pos |= (1 << 1);
  }

  // Sends a clean report to the host. This is important on any Arduino type.
  Consumer.begin();
  Serial1.println("START");

}

void loop() {

  int8_t enc_action = 0; // 1 or -1 if moved, sign is direction

  // note: for better performance, the code will now use
  // direct port access techniques
  // http://www.arduino.cc/en/Reference/PortManipulation
  uint8_t enc_cur_pos = 0;
  // read in the encoder state first
  if (digitalRead(PIN_ENCODER_A) == LOW) {
    enc_cur_pos |= (1 << 0);
  }
  if (digitalRead(PIN_ENCODER_B) == LOW) {
    enc_cur_pos |= (1 << 1);
  }

  // if any rotation at all
  if (enc_cur_pos != enc_prev_pos)
  {
    Serial1.println(enc_cur_pos);
    if (enc_prev_pos == 0x00)
    {
      // this is the first edge
      if (enc_cur_pos == 0x01) {
        enc_flags |= (1 << 0);
      }
      else if (enc_cur_pos == 0x02) {
        enc_flags |= (1 << 1);
      }
    }

    if (enc_cur_pos == 0x03)
    {
      // this is when the encoder is in the middle of a "step"
      enc_flags |= (1 << 4);
    }
    else if (enc_cur_pos == 0x00)
    {
      // this is the final edge
      if (enc_prev_pos == 0x02) {
        enc_flags |= (1 << 2);
      }
      else if (enc_prev_pos == 0x01) {
        enc_flags |= (1 << 3);
      }

      // check the first and last edge
      // or maybe one edge is missing, if missing then require the middle state
      // this will reject bounces and false movements
      if ((enc_flags & BIT0) && ((enc_flags & BIT1) || (enc_flags & BIT3))) {
        enc_action = 1;
      }
      else if ((enc_flags & BIT2) && ((enc_flags & BIT0) || (enc_flags & BIT4))) {
        enc_action = 1;
      }
      else if ((enc_flags & BIT1) && ((enc_flags & BIT3) || (enc_flags & BIT4))) {
        enc_action = -1;
      }
      else if ((enc_flags & BIT3) && ((enc_flags & BIT1) || (enc_flags & BIT4))) {
        enc_action = -1;
      }

      enc_flags = 0; // reset for next time
    }
  }

  enc_prev_pos = enc_cur_pos;
  

  if (enc_action > 0) {
    Consumer.write(MEDIA_VOLUME_UP);
  }
  else if (enc_action < 0) {
    Consumer.write(MEDIA_VOLUME_DOWN);
  }

  if (!digitalRead(pinButton)) {
    digitalWrite(pinLed, HIGH);

    // See HID Project documentation for more Consumer keys
    Consumer.write(MEDIA_VOLUME_MUTE);

    // Simple debounce
    delay(300);
    digitalWrite(pinLed, LOW);
  }
}
