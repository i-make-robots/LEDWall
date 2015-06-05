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

#define PANEL_WIDTH    8
#define PANEL_HEIGHT   8
#define COLS_LEDs      8*4 // all of the following params need to be adjusted for screen size
#define ROWS_LEDs      8*3  // LED_LAYOUT assumed 0 if ROWS_LEDs > 8
#define LEDS_PER_STRIP (8*8*4)//(COLS_LEDs * (ROWS_LEDs / 6))
#define PANELS_PER_PIN 4



DMAMEM int displayMemory[LEDS_PER_STRIP*6];
int drawingMemory[LEDS_PER_STRIP*6];
OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, WS2811_RGB | WS2811_800kHz);


void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Serial.setTimeout(50);
  leds.begin();
  leds.show();
}


void loop() {
  int i,r,g,b;

  int s = LEDS_PER_STRIP * PANELS_PER_PIN;//(sizeof(drawingMemory)/3)-1;
  int flip13=LOW;
  i=0;
  
  //if(Serial.available()) {
  while(1) {
    while(Serial.available()<=0); 
    r = Serial.read();
    //while(Serial.available()<=0); 
    g = Serial.read();
    //while(Serial.available()<=0); 
    b = Serial.read();
    
    if( (r&0x01)==1 && (g&0x01)==1 && (b&0x01)==1 ) {
      // start of new frame
      i=0;
      if(flip13==HIGH) flip13=LOW;
      else flip13=HIGH;
      digitalWrite(13, flip13);
      leds.show();  // not sure if this function is needed  to update each frame
      //r=g=b=0;
    }
    if(i < s) {
      leds.setPixel(i, ((r << 16) | (g << 8) | b));
//      leds.setPixel(led_map(i,((r << 16) | (g << 8) | b)));
      i++;
//      drawingMemory[i++]=r;
//      drawingMemory[i++]=g;
//      drawingMemory[i++]=b;
    }
  }
}

