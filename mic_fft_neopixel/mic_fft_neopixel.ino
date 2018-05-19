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

//Neopixel stuff --------------------
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

//Circuit Playground stuff --------------------
#include <Adafruit_CircuitPlayground.h>
#include <Wire.h>
#include <SPI.h>

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

#include "Calibration.h"

void setup() {
  CircuitPlayground.begin();

  //JW: comment out circuit playground's pixel animation, use external neopixel strip
  //CircuitPlayground.setBrightness(255);
  //CircuitPlayground.clearPixels();
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

// LOOP FUNCTION - runs over and over - does animation ---------------------

void loop() {
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
    data   = (uint8_t *)pgm_read_word(&binData[i]);
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
    if (level <= 255) {
      uint8_t r = (pgm_read_byte(&reds[i])   * level) >> 8,
              g = (pgm_read_byte(&greens[i]) * level) >> 8,
              b = (pgm_read_byte(&blues[i])  * level) >> 8;
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
