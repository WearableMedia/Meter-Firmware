
// CALIBRATION CONSTANTS ---------------------------------------------------

const uint8_t PROGMEM
// Low-level noise initially subtracted from each of 32 FFT bins
noise[]    = { 0x04, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x02,
               0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01,
               0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
               0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
             },
             // FFT bins (32) are then filtered down to 10 output bins (to match the
             // number of NeoPixels on Circuit Playground).  10 arrays here, one per
             // output bin.  First element of each is the number of input bins to
             // merge, second element is index of first merged bin, remaining values
             // are scaling weights as each input bin is merged into output.  The
             // merging also "de-linearizes" the FFT output, so it's closer to a
             // logarithmic scale with octaves evenly-ish spaced, music looks better.
             bin0data[] = { 1, 2, 147 },
                          bin1data[] = { 2, 2, 89, 14 },
                                       bin2data[] = { 2, 3, 89, 14 },
                                           bin3data[] = { 4, 3, 15, 181, 58, 3 },
                                               bin4data[] = { 4, 4, 15, 181, 58, 3 },
                                                   bin5data[] = { 6, 5, 6, 89, 185, 85, 14, 2 },
                                                       bin6data[] = { 7, 7, 5, 60, 173, 147, 49, 9, 1 },
                                                           bin7data[] = { 10, 8, 3, 23, 89, 170, 176, 109, 45, 14, 4, 1 },
                                                               bin8data[] = { 13, 11, 2, 12, 45, 106, 167, 184, 147, 89, 43, 18, 6, 2, 1 },
                                                                   bin9data[] = { 18, 14, 2, 6, 19, 46, 89, 138, 175, 185, 165, 127, 85, 51, 27, 14, 7, 3, 2, 1 },
                                                                       // Pointers to 10 bin arrays, because PROGMEM arrays-of-arrays are weird:
                                                                       * const binData0[] = { bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data,
                                                                                              bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data,
                                                                                              bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data,
                                                                                              bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data,
                                                                                              bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data,
                                                                                              bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data, bin9data
                                                                                            },
                                                                           // Pointers to 10 bin arrays, because PROGMEM arrays-of-arrays are weird:
                                                                           * const binData1[] = { bin0data, bin1data, bin2data, bin3data, bin4data, bin5data, bin6data, bin7data,
                                                                                                  bin8data, bin9data, bin0data, bin1data, bin2data, bin3data, bin4data, bin5data,
                                                                                                  bin6data, bin7data, bin8data, bin9data, bin0data, bin1data, bin2data, bin3data,
                                                                                                  bin4data, bin5data, bin6data, bin7data, bin8data, bin9data, bin0data, bin1data,
                                                                                                  bin2data, bin3data, bin4data, bin5data, bin6data, bin7data, bin8data, bin9data,
                                                                                                  bin0data, bin1data, bin2data, bin3data, bin4data, bin5data, bin6data, bin7data
                                                                                                },
                                                                               // Pointers to 10 bin arrays, because PROGMEM arrays-of-arrays are weird:
                                                                               * const binData2[] = { bin9data, bin9data, bin8data, bin8data, bin7data, bin7data, bin6data, bin6data,
                                                                                                      bin5data, bin5data, bin4data, bin4data, bin4data, bin3data, bin3data, bin3data,
                                                                                                      bin2data, bin2data, bin2data, bin1data, bin1data, bin1data, bin0data, bin0data,
                                                                                                      bin0data, bin0data, bin1data, bin1data, bin1data, bin2data, bin2data, bin2data,
                                                                                                      bin3data, bin3data, bin3data, bin4data, bin4data, bin4data, bin5data, bin5data,
                                                                                                      bin6data, bin6data, bin7data, bin7data, bin8data, bin8data, bin9data, bin9data
                                                                                                    },
                                                                                   * const binData3[] = { bin0data, bin0data, bin1data, bin1data, bin2data, bin2data, bin3data, bin3data,
                                                                                                          bin4data, bin4data, bin5data, bin5data, bin5data, bin6data, bin6data, bin6data,
                                                                                                          bin7data, bin7data, bin7data, bin8data, bin8data, bin8data, bin9data, bin9data,
                                                                                                          bin9data, bin9data, bin8data, bin8data, bin8data, bin7data, bin7data, bin7data,
                                                                                                          bin6data, bin6data, bin6data, bin5data, bin5data, bin5data, bin4data, bin4data,
                                                                                                          bin3data, bin3data, bin2data, bin2data, bin1data, bin1data, bin0data, bin0data
                                                                                                        },
                                                                                       * const binData4[] = { bin0data, bin0data, bin0data, bin0data, bin0data, bin1data, bin1data, bin1data,
                                                                                                              bin1data, bin1data, bin2data, bin2data, bin2data, bin2data, bin2data, bin3data,
                                                                                                              bin3data, bin3data, bin3data, bin3data, bin4data, bin4data, bin4data, bin4data,
                                                                                                              bin4data, bin5data, bin5data, bin5data, bin5data, bin5data, bin6data, bin6data,
                                                                                                              bin6data, bin6data, bin6data, bin7data, bin7data, bin7data, bin7data, bin7data,
                                                                                                              bin8data, bin8data, bin8data, bin8data, bin9data, bin9data, bin9data, bin9data
                                                                                                            },
                                                                                           // R,G,B values for color wheel covering 10 NeoPixels:
                                                                                           reds0[]   = { 0xAD, 0x9A, 0x84, 0x65, 0x00, 0x00, 0x00, 0x00, 0x65, 0x84, 0xAD, 0x9A, 0x84, 0x65, 0x00, 0x00, 0x00, 0x00, 0x65, 0x84, 0xAD, 0x9A, 0x84, 0x65, 0x00, 0x00, 0x00, 0x00, 0x65, 0x84, 0xAD, 0x9A, 0x84, 0x65, 0x00, 0x00, 0x00, 0x00, 0x65, 0x84, 0xAD, 0x9A, 0x84, 0x65, 0x00, 0x00, 0x00, 0x00, 0x65, 0x84 },
                                                                                               greens0[] = { 0x00, 0x66, 0x87, 0x9E, 0xB1, 0x87, 0x66, 0x00, 0x00, 0x00, 0x00, 0x66, 0x87, 0x9E, 0xB1, 0x87, 0x66, 0x00, 0x00, 0x00, 0x00, 0x66, 0x87, 0x9E, 0xB1, 0x87, 0x66, 0x00, 0x00, 0x00, 0x00, 0x66, 0x87, 0x9E, 0xB1, 0x87, 0x66, 0x00, 0x00, 0x00, 0x00, 0x66, 0x87, 0x9E, 0xB1, 0x87, 0x66, 0x00, 0x00, 0x00 },
                                                                                                   blues0[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0xE4, 0xFF, 0xE4, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0xE4, 0xFF, 0xE4, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0xE4, 0xFF, 0xE4, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0xE4, 0xFF, 0xE4, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0xE4, 0xFF, 0xE4, 0xC3 },
                                                                                                       // R,G,B values for color wheel covering 10 NeoPixels:
                                                                                                       reds1[]   = { 0xAD, 0x9A, 0x84, 0x65, 0x00, 0x00, 0x00, 0x00, 0x65, 0x84, 0xAD, 0x9A},
                                                                                                           greens1[] = { 0x00, 0x66, 0x87, 0x9E, 0xB1, 0x87, 0x66, 0x00, 0x00, 0x00, 0x00, 0x66},
                                                                                                               blues1[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0xC3, 0xE4, 0xFF, 0xE4, 0xC3, 0x00, 0x00 },
                                                                                                                   // R,G,B values for color wheel covering 10 NeoPixels:
                                                                                                                   reds2[]   = { 204, 204, 255, 102, 153, 255},
                                                                                                                       greens2[] = { 102, 153, 0, 255, 51, 102},
                                                                                                                           blues2[]  = { 255, 255, 255, 255, 255, 255},
gamma8[] = { // Gamma correction improves the appearance of midrange colors
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03,
  0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06,
  0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09,
  0x0A, 0x0A, 0x0A, 0x0B, 0x0B, 0x0B, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0E,
  0x0E, 0x0F, 0x0F, 0x10, 0x10, 0x11, 0x11, 0x12, 0x12, 0x13, 0x13, 0x14,
  0x14, 0x15, 0x15, 0x16, 0x16, 0x17, 0x18, 0x18, 0x19, 0x19, 0x1A, 0x1B,
  0x1B, 0x1C, 0x1D, 0x1D, 0x1E, 0x1F, 0x1F, 0x20, 0x21, 0x22, 0x22, 0x23,
  0x24, 0x25, 0x26, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2A, 0x2B, 0x2C, 0x2D,
  0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x44, 0x45, 0x46,
  0x47, 0x48, 0x49, 0x4B, 0x4C, 0x4D, 0x4E, 0x50, 0x51, 0x52, 0x54, 0x55,
  0x56, 0x58, 0x59, 0x5A, 0x5C, 0x5D, 0x5E, 0x60, 0x61, 0x63, 0x64, 0x66,
  0x67, 0x69, 0x6A, 0x6C, 0x6D, 0x6F, 0x70, 0x72, 0x73, 0x75, 0x77, 0x78,
  0x7A, 0x7C, 0x7D, 0x7F, 0x81, 0x82, 0x84, 0x86, 0x88, 0x89, 0x8B, 0x8D,
  0x8F, 0x91, 0x92, 0x94, 0x96, 0x98, 0x9A, 0x9C, 0x9E, 0xA0, 0xA2, 0xA4,
  0xA6, 0xA8, 0xAA, 0xAC, 0xAE, 0xB0, 0xB2, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC,
  0xBF, 0xC1, 0xC3, 0xC5, 0xC7, 0xCA, 0xCC, 0xCE, 0xD1, 0xD3, 0xD5, 0xD7,
  0xDA, 0xDC, 0xDF, 0xE1, 0xE3, 0xE6, 0xE8, 0xEB, 0xED, 0xF0, 0xF2, 0xF5,
  0xF7, 0xFA, 0xFC, 0xFF
};
const uint16_t PROGMEM
// Scaling values applied to each FFT bin (32) after noise subtraction
// but prior to merging/filtering.  When multiplied by these values,
// then divided by 256, these tend to produce outputs in the 0-255
// range (VERY VERY "ISH") at normal listening levels.  These were
// determined empirically by throwing lots of sample audio at it.
binMul[] = { 405, 508, 486, 544, 533, 487, 519, 410,
             481, 413, 419, 410, 397, 424, 412, 411,
             511, 591, 588, 577, 554, 529, 524, 570,
             546, 559, 511, 552, 439, 488, 483, 547,
           },
           // Sums of bin weights for bin-merging tables above.
           binDiv[]   = { 147, 103, 103, 257, 257, 381, 444, 634, 822, 1142 };

