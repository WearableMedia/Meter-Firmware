

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
void setup() {
  Serial.begin(9600);
  pixels.begin();
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  pinMode(switchPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(switchPin), powerSwitch, CHANGE);

}

void loop() {
  if (LEDOn) {
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
      whiteVolumeMode();
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

