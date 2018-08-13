//BLE stuff --------------
#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#if SOFTWARE_SERIAL_AVAILABLE
#include <SoftwareSerial.h>
#endif

#include "BluefruitConfig.h"

#define FACTORYRESET_ENABLE     1

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
//BLE stuff ends here ----------------------

//Neopixel stuff ----------------------------------------
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      48

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

uint8_t red, green, blue = 255;

//Neopixel stuff ends here----------------------------------------


//Button and switch stuff ----------------------------------------
const int leftButtonPin = 5;
const int rightButtonPin = 10;
int leftButtonCounter = 0;
bool prevLeftButtonPressed = false;

int rightButtonCounter = 0;
bool prevRightButtonPressed = false;

bool LEDOn = true;

const int interruptPin = 0;
const int switchPin = 11;

int animationMode = 3;

//Button and switch stuff ends here----------------------------------------

//rainbow animation----------------------------------------

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 20;           // interval at which to blink (milliseconds)
long rainbowCounter = 0;

int animationThreashold = 8;
int animationCounter = 0;
int prevAnimationMode = 0;
//rainbow animation ends here----------------------------------------

//microphone stuff----------------------------------------
const int sampleWindow = 100; // Sample window width in mS (250 mS = 4Hz)
unsigned int knock;
int micBrightness;
//microphone stuff ends here----------------------------------------

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}


// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// the packet buffer
extern uint8_t packetbuffer[];


void setup() {
  Serial.begin(115200);
  pixels.begin();
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  pinMode(switchPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(switchPin), powerSwitch, CHANGE);

  //BLE setup
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
  Serial.println(F("Then activate/use the sensors, color picker, game controller, etc!"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  //  /* Wait for connection */
  //  while (! ble.isConnected()) {
  //      delay(500);
  //  }

  Serial.println(F("***********************"));

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("***********************"));

}



void loop() {
  if (LEDOn) {
    if (ble.isConnected()) {
      /* Wait for new data to arrive */
      uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
      if (len != 0) {

        /* Got a packet! */
        // printHex(packetbuffer, len);

        if (packetbuffer[1] == 'B') {
          uint8_t button = packetbuffer[2];
          if (button == 56) {
            uint8_t buttonState = packetbuffer[3];
            if (buttonState == 48) {
              animationMode++;
            }
          } else if (button == 55) {
            uint8_t buttonState = packetbuffer[3];
            if (buttonState == 48) {
              animationMode--;
            }
          }
        }
        //Color
        if (packetbuffer[1] == 'C') {
          animationMode = 1;
          red = packetbuffer[2];
          green = packetbuffer[3];
          blue = packetbuffer[4];
        }
      }
    }


    sampleMic();
    //check the switch
    //  bool slideSwitch = CircuitPlayground.slideSwitch();

    //check the button
    bool leftButtonPressed = digitalRead(leftButtonPin);
    bool rightButtonPressed = digitalRead(rightButtonPin);
    leftButtonPressed = !leftButtonPressed;
    rightButtonPressed = !rightButtonPressed;

    if (leftButtonPressed ) {
      if (!prevLeftButtonPressed) {
        //leftButtonCounter++;
        animationMode --;
        Serial.println("--");
      }
    }


    if (rightButtonPressed ) {
      if (!prevRightButtonPressed) {
        //rightButtonCounter++;
        animationMode++;
        Serial.println("++");
      }
    }

    prevLeftButtonPressed = leftButtonPressed;
    prevRightButtonPressed = rightButtonPressed;


    if (animationMode > 3) {
      animationMode = 1;
    } else if (animationMode < 1) {
      animationMode = 3;
    }

    if (animationMode == 1) {
      //single color mode
      Serial.print ("RGB #");
      if (red < 0x10) Serial.print("0");
      Serial.print(red, HEX);
      if (green < 0x10) Serial.print("0");
      Serial.print(green, HEX);
      if (blue < 0x10) Serial.print("0");
      Serial.println(blue, HEX);

      for (uint8_t i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(red, green, blue));
      }
      pixels.show(); // This sends the updated pixel color to the hardware.
    } else if (animationMode == 2) {
      whiteVolumeMode();
    } else if (animationMode == 3) {
      rainbowMode();
    } else {
    }
  } else {
    for (int i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, 0, 0, 0);
    }
    pixels.show();
  }
}

void powerSwitch() {
  LEDOn = digitalRead(switchPin);
  Serial.println(LEDOn);
}
void sampleMic() {
  unsigned long start = millis(); // Start of sample window
  unsigned int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;

  // collect data for 250 miliseconds
  while (millis() - start < sampleWindow)
  {
    knock = analogRead(0);
    if (knock < 1024)  //This is the max of the 10-bit ADC so this loop will include all readings
    {
      if (knock > signalMax)
      {
        signalMax = knock;  // save just the max levels
      }
      else if (knock < signalMin)
      {
        signalMin = knock;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
  micBrightness = (peakToPeak * 256) / 1024;  // convert to brightness
}

void whiteVolumeMode() {
  for (int i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, micBrightness, micBrightness, micBrightness);
  }
  pixels.show();

}

void rainbowMode() {
  rainbowCounter++;
  if (rainbowCounter == 256) {
    rainbowCounter = 0;
  }
  rainbow();
}


void rainbow() {
  uint16_t i, j;
  unsigned long currentMillis = millis();

  //  for (j = 0; j < 256; j++) {
  for (i = 0; i < pixels.numPixels(); i++) {
    //if (currentMillis - previousMillis >= interval) {
    pixels.setPixelColor(i, Wheel((i + rainbowCounter) & 255));
    //}
    pixels.show();
  }
  //  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

