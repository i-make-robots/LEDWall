/*  OctoWS2811 VideoDisplay.ino - Video on LEDs, from a PC, Mac, Raspberry Pi
    http://www.pjrc.com/teensy/td_libs_OctoWS2811.html
    Copyright (c) 2013 Paul Stoffregen, PJRC.COM, LLC

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

 
  Required Connections
  --------------------
    pin 2:  LED Strip #1    OctoWS2811 drives 8 LED Strips.
    pin 14: LED strip #2    All 8 are the same length.
    pin 7:  LED strip #3
    pin 8:  LED strip #4    A 100 to 220 ohm resistor should used
    pin 6:  LED strip #5    between each Teensy pin and the
    pin 20: LED strip #6    wire to the LED strip, to minimize
    pin 21: LED strip #7    high frequency ringining & noise.
    pin 5:  LED strip #8
    pin 15 & 16 - Connect together, but do not use
    pin 4:  Do not use
    pin 3:  Do not use as PWM.  Normal use is ok.
    pin 12: Frame Sync

    When using more than 1 Teensy to display a video image, connect
    the Frame Sync signal between every board.  All boards will
    synchronize their WS2811 update using this signal.

    Beware of image distortion from long LED strip lengths.  During
    the WS2811 update, the LEDs update in sequence, not all at the
    same instant!  The first pixel updates after 30 microseconds,
    the second pixel after 60 us, and so on.  A strip of 120 LEDs
    updates in 3.6 ms, which is 10.8% of a 30 Hz video frame time.
    Doubling the strip length to 240 LEDs increases the lag to 21.6%
    of a video frame.  For best results, use shorter length strips.
    Multiple boards linked by the frame sync signal provides superior
    video timing accuracy.

    A Multi-TT USB hub should be used if 2 or more Teensy boards
    are connected.  The Multi-TT feature allows proper USB bandwidth
    allocation.  Single-TT hubs, or direct connection to multiple
    ports on the same motherboard, may give poor performance.
*/

#include <OctoWS2811.h>

#define COLUMNS      (8*4)  // all of the following params need to be adjusted for screen size
#define ROWS         (8*3)  // LED_LAYOUT assumed 0 if ROWS_LEDs > 8
#define PINS_USED    (3)

#define TOTAL_LIGHTS    (COLUMNS * ROWS)
#define LEDS_PER_STRIP  (TOTAL_LIGHTS / PINS_USED)



DMAMEM int displayMemory[LEDS_PER_STRIP*6];
int drawingMemory[LEDS_PER_STRIP*6];
OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, WS2811_RGB | WS2811_800kHz);
int pixeli;


void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  Serial.setTimeout(50);

  Serial.print('0');  // left pixel
  Serial.print(',');
  Serial.print('0');  // top pixel
  Serial.print(',');
  Serial.print(COLUMNS);  // right pixel
  Serial.print(',');
  Serial.print(ROWS);  // bottom pixel

  leds.begin();
  
  for(long i=0;i<TOTAL_LIGHTS;++i) {
    long j = i % COLUMNS;
    long k = i / COLUMNS;
    leds.setPixel(led_map3(i),rainbow(j,k));
  }
  leds.show();
  pixeli=0;
}


int rainbow(long j,long k) {
  float f = (float)j / COLUMNS;
  float v = (float)k / (float)ROWS;
  
  Serial.print(f);
  
  int r=0,g=0,b=0;
  float x=6;
  if(v<0.2) {
    if(f<0.5) {
      r=g=b=255;
    } else {
      r=g=b=0;
    }
  } else if(v<0.8) {
    if(f<1/x) r=255;
    else if(f<2/x) g=255;
    else if(f<3/x) b=255;
    else if(f<4/x) r=g=255;
    else if(f<5/x) g=b=255;
    else if(f<6/x) b=r=255;
  } else {
    r=g=b=255*f;
  }
  
  return ((r << 16) | (g << 8) | b);
}


/**
 * RGB colors are delivered left to right, top to bottom.
 * change the index to match the wiring which (in this case)
 * is a Z pattern.
 */
int led_map2(int i) {
  int row = i / COLUMNS;
  int col = i % COLUMNS;

  if( ( row % 2 ) == 0 ) {
    col = COLUMNS - 1 - col;
  }

  return row * COLUMNS + col;
}


int led_map3(int input) {

  int row = input / LEDS_PER_STRIP;
  input %= LEDS_PER_STRIP;
  
  int y = input / ( COLUMNS );
  int x = input % ( COLUMNS );
  
  if((x%2)==1) {
    y = 7-y;
  }
  
  int output = row * LEDS_PER_STRIP
             + x * 8
             + y;
  return output;
}

int led_map(int i) {
  return i;
}


void loop() {
  do {
    while(Serial.available()<=0); 
    int r = Serial.read();
    int g = Serial.read();
    int b = Serial.read();
    
    // the least significant bit of every pixel is zero, except on the first pixel of each frame.
    // this way if a pixel doesn't get transmitted the teensy can find the start of the next frame
    // without this the video would get increasingly wierd the longer it was on.
    if( (r&0x01)==1 && (g&0x01)==1 && (b&0x01)==1 ) {
      // start of new frame
      pixeli=0;

      leds.show();  // not sure if this function is needed  to update each frame
    }
    if(pixeli < TOTAL_LIGHTS) {
      // fill this frame
      leds.setPixel(led_map(pixeli), ((r << 16) | (g << 8) | b));
      pixeli++;
    }
  } while(1);
}

