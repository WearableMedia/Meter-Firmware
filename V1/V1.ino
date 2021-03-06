// FFT-based audio visualizer for Adafruit Circuit Playground: uses the
// built-in mic on A4, 10x NeoPixels for display.  Built on the ELM-Chan
// FFT library for AVR microcontrollers.

// The fast Fourier transform (FFT) algorithm converts a signal from the
// time domain to the frequency domain -- e.g. turning a sampled audio
// signal into a visualization of frequencies and magnitudes -- an EQ meter.

// The FFT algorithm itself is handled in the Circuit Playground library;
// the code here is mostly for converting that function's output into
// animation.  In most AV gear it's usually done with bargraph displays;
// with a 1D output (the 10 NeoPixels) we need to get creative with color
// and brightness...it won't look great in every situation (seems to work
// best with LOUD music), but it's colorful and fun to look at.  So this
// code is mostly a bunch of tables and weird fixed-point (integer) math
// that probably doesn't make much sense even with all these comments.


/*********************************************************************
  This is an example for our nRF51822 based Bluefruit LE modules

  Pick one up today in the adafruit shop!

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  MIT license, check LICENSE for more information
  All text above, and the splash screen below must be included in
  any redistribution
*****************************************************/

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

//Circuit Playground stuff ----------------------------------------
#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>

// GLOBAL STUFF ------------------------------------------------------------

// Displaying EQ meter output straight from the FFT may be 'correct,' but
// isn't always visually interesting (most bins spend most time near zero).
// Dynamic level adjustment narrows in on a range of values so there's
// always something going on.  The upper and lower range are based on recent
// audio history, and on a per-bin basis (some may be more active than
// others, so this keeps one or two "loud" bins from spoiling the rest.

#define BINS   10          // FFT output is filtered down to this many bins
#define FRAMES 4           // This many FFT cycles are averaged for leveling
uint8_t lvl[FRAMES][BINS], // Bin levels for the prior #FRAMES frames
        avgLo[BINS],       // Pseudo rolling averages for bins -- lower and
        avgHi[BINS],       // upper limits -- for dynamic level adjustment.
        frameIdx = 0;      // Counter for lvl storage

int leftButtonCounter = 0;
bool prevLeftButtonPressed = false;

int rightButtonCounter = 0;
bool prevRightButtonPressed = false;

bool LEDOn = true;
#include "Calibration.h"

int animationMode = 3;

//rainbow animation
unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 20;           // interval at which to blink (milliseconds)
long rainbowCounter = 0;

int animationThreashold = 8;
int animationCounter = 0;
int prevAnimationMode = 0;
void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
  pixels.begin();

  // Initialize rolling average ranges
  uint8_t i;
  for (i = 0; i < BINS; i++) {
    avgLo[i] = 0;
    avgHi[i] = 255;
  }
  for (i = 0; i < FRAMES; i++) {
    memset(&lvl[i], 127, sizeof(lvl[i]));
  }
}

void loop() {
  //check the switch
  bool slideSwitch = CircuitPlayground.slideSwitch();

  //check the button
  bool leftButtonPressed = CircuitPlayground.leftButton();
  bool rightButtonPressed = CircuitPlayground.rightButton();

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

  //  if (leftButtonCounter >= 5) {
  //    leftButtonCounter = 0;
  //  }
  prevLeftButtonPressed = leftButtonPressed;
  prevRightButtonPressed = rightButtonPressed;
  //
  //
  //
  //  if (rightButtonCounter >= 6) {
  //    rightButtonCounter = 0;
  //  }


  if (animationMode > 3) {
    animationMode = 0;
  } else if (animationMode < 0) {
    animationMode = 3;
  }

  if (animationMode == 0) {
    leftButtonCounter = 0;
    rightButtonCounter = 0;
  } else if (animationMode == 1) {
    leftButtonCounter = 2;
    rightButtonCounter = 2;
  } else if (animationMode == 2) {
    leftButtonCounter = 4;
    rightButtonCounter = 4;
  } else if (animationMode == 3) {
    leftButtonCounter = 0;
    rightButtonCounter = 0;
  }

  if (slideSwitch && LEDOn) {
    if (animationMode == 3) {
      rainbowCounter++;
      if (rainbowCounter == 256) {
        rainbowCounter = 0;
      }
      rainbow();
    } else {

      uint16_t spectrum[32]; // FFT spectrum output buffer

      CircuitPlayground.mic.fft(spectrum);


      // spectrum[] is now raw FFT output, 32 bins.

      // Remove noise and apply EQ levels
      uint8_t  i, N;
      uint16_t S;
      for (i = 0; i < 32; i++) {
        N = pgm_read_byte(&noise[i]);
        if (spectrum[i] > N) { // Above noise threshold: scale & clip
          S           = ((spectrum[i] - N) *
                         (uint32_t)pgm_read_word(&binMul[i])) >> 8;
          spectrum[i] = (S < 255) ? S : 255;
        } else { // Below noise threshold: clip
          spectrum[i] = 0;
        }
      }

      // spectrum[] is now noise-filtered, scaled & clipped
      // FFT output, in range 0-255, still 32 bins.

      // Filter spectrum[] from 32 elements down to 10,
      // make pretty colors out of it:

      uint16_t sum, level;
      uint8_t  j, minLvl, maxLvl, nBins, binNum, *data;

      for (i = 0; i < NUMPIXELS; i++) { // For each output bin (and each pixel)...
        if (leftButtonCounter == 0) {
          data   = (uint8_t *)pgm_read_word(&binData0[i]);
        } else if (leftButtonCounter == 1) {
          data   = (uint8_t *)pgm_read_word(&binData1[i]);
        } else if (leftButtonCounter == 2) {
          data   = (uint8_t *)pgm_read_word(&binData2[i]);
        } else if (leftButtonCounter == 3) {
          data   = (uint8_t *)pgm_read_word(&binData3[i]);
        } else if (leftButtonCounter == 4) {
          data   = (uint8_t *)pgm_read_word(&binData4[i]);
        }

        nBins  = pgm_read_byte(&data[0]); // Number of input bins to merge
        binNum = pgm_read_byte(&data[1]); // Index of first input bin
        data  += 2;
        for (sum = 0, j = 0; j < nBins; j++) {
          sum += spectrum[binNum++] * pgm_read_byte(&data[j]); // Total
        }
        sum /= pgm_read_word(&binDiv[i]);                      // Average
        lvl[frameIdx][i] = sum;      // Save for rolling averages
        minLvl = maxLvl = lvl[0][i]; // Get min and max range for bin
        for (j = 1; j < FRAMES; j++) { // from prior stored frames
          if (lvl[j][i] < minLvl)      minLvl = lvl[j][i];
          else if (lvl[j][i] > maxLvl) maxLvl = lvl[j][i];
        }

        // minLvl and maxLvl indicate the extents of the FFT output for this
        // bin over the past few frames, used for vertically scaling the output
        // graph (so it looks interesting regardless of volume level).  If too
        // close together though (e.g. at very low volume levels) the graph
        // becomes super coarse and 'jumpy'...so keep some minimum distance
        // between them (also lets the graph go to zero when no sound playing):
        if ((maxLvl - minLvl) < 23) {
          maxLvl = (minLvl < (255 - 23)) ? minLvl + 23 : 255;
        }
        avgLo[i] = (avgLo[i] * 7 + minLvl) / 8; // Dampen min/max levels
        avgHi[i] = (maxLvl >= avgHi[i]) ?       // (fake rolling averages)
                   (avgHi[i] *  3 + maxLvl) /  4 :       // Fast rise
                   (avgHi[i] * 31 + maxLvl) / 32;        // Slow decay

        // Second fixed-point scale then 'stretches' each bin based on
        // dynamic min/max levels to 0-256 range:
        level = 1 + ((sum <= avgLo[i]) ? 0 :
                     256L * (sum - avgLo[i]) / (long)(avgHi[i] - avgLo[i]));
        // Clip output and convert to color:
        uint8_t r, g, b;
        if (level <= 255) {
          if (rightButtonCounter == 0) {
            r = (pgm_read_byte(&reds0[i])   * level) >> 8;
            g = (pgm_read_byte(&greens0[i]) * level) >> 8;
            b = (pgm_read_byte(&blues0[i])  * level) >> 8;
          } else if (rightButtonCounter == 1) {
            r = (pgm_read_byte(&reds1[i % 12])   * level) >> 8;
            g = (pgm_read_byte(&greens1[i % 12]) * level) >> 8;
            b = (pgm_read_byte(&blues1[i % 12])  * level) >> 8;
          } else if (rightButtonCounter == 2) {
            r = (pgm_read_byte(&reds2[i % 6])   * level) >> 8;
            g = (pgm_read_byte(&greens2[i % 6]) * level) >> 8;
            b = (pgm_read_byte(&blues2[i % 6])  * level) >> 8;
          } else if (rightButtonCounter == 3) {
            r = (pgm_read_byte(&reds1[i / 6])   * level) >> 8;
            g = (pgm_read_byte(&greens1[i / 6]) * level) >> 8;
            b = (pgm_read_byte(&blues1[i / 6])  * level) >> 8;
          } else {
            r = (pgm_read_byte(174)   * level) >> 8;
            g = (pgm_read_byte(174) * level) >> 8;
            b = (pgm_read_byte(174)  * level) >> 8;
          }

          pixels.setPixelColor(i,
                               pgm_read_byte(&gamma8[r]),
                               pgm_read_byte(&gamma8[g]),
                               pgm_read_byte(&gamma8[b]));


        } else { // level = 256, show white pixel OONTZ OONTZ
          //CircuitPlayground.strip.setPixelColor(i, 0x56587F);
          pixels.setPixelColor(i, 0x56587F);
        }
      }
      pixels.show();

      if (++frameIdx >= FRAMES) frameIdx = 0;
    }
  } else {
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, 0, 0, 0);
      pixels.show();
    }
  }
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

