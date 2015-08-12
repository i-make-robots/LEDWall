// Modified by dan@marginallyclever.com 2015-07-03

/*  OctoWS2811 movie2serial.pde - Transmit video data to 1 or more
      Teensy 3.0 boards running OctoWS2811 VideoDisplay.ino
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
*/

// To configure this program, edit the following sections:
//
//  1: change myMovie to open a video file of your choice    ;-)
//
//  2: edit the serialConfigure() lines in setup() for your
//     serial device names (Mac, Linux) or COM ports (Windows)
//
//  3: if your LED strips have unusual color configuration,
//     edit colorWiring().  Nearly all strips have GRB wiring,
//     so normally you can leave this as-is.
//
//  4: if playing 50 or 60 Hz progressive video (or faster),
//     edit framerate in movieEvent().

import processing.video.*;
import processing.serial.*;
import java.awt.Rectangle;

SimpleScreenCapture simpleScreenCapture;

//Movie myMovie = new Movie(this, "/Users/danroyer/Movies/Die Hard [1988] DvdRip [Eng] - Thizz/Die Hard [1988] DvdRip [Eng] - Thizz.avi");
//Movie myMovie = new Movie(this, "/Users/danroyer/Movies/The Fifth Element[1997]DvDrip[Eng]-FXG/The Fifth Element[1997]DvDrip[Eng]-FXG.avi");
//Movie myMovie = new Movie(this, "/Users/danroyer/Downloads/test.avi");
//Movie myMovie = new Movie(this, "/Users/danroyer/Movies/voodoo.mp4");
//Movie myMovie = new Movie(this, "/Users/danroyer/Movies/test2.mp4");
//Movie myMovie = new Movie(this, "/Users/danroyer/Movies/silhouette.mp4");

final int SCREEN_WIDTH = 32;
final int SCREEN_HEIGHT = 24;

final int PANEL_WIDTH=8;
final int PANEL_HEIGHT=8;
final int PANELS_PER_PIN = 4;
final int LEDS_PER_STRIP = PANEL_WIDTH * PANEL_HEIGHT * PANELS_PER_PIN;

float gamma = 1.7;

int numPorts=0;  // the number of serial ports in use
int maxPorts=24; // maximum number of serial ports

Serial[] ledSerial = new Serial[maxPorts];     // each port's actual Serial port
Rectangle[] ledArea = new Rectangle[maxPorts]; // the area of the movie each port gets, in % (0-100)
boolean[] ledLayout = new boolean[maxPorts];   // layout of rows, true = even is left->right
PImage[] ledImage = new PImage[maxPorts];      // image sent to each port
int errorCount=0;
float framerate=0;
int maxW,maxH;
PImage img;// = new PImage();
//PImage cpy = new PImage();
byte[] ledData;


void setup() {
  maxW=maxH=0;
  String[] list = Serial.list();
  delay(20);
  println("Serial Ports List:");
  println(list);
  //serialConfigure("/dev/tty.usbmodem315451");  // change these to your port names
  serialConfigure(list[list.length-1]);
  if (errorCount > 0) exit();
  
  int size=(maxW * maxH * 3);
  ledData =  new byte[size+3];
  ledData[size+0]=0;
  ledData[size+1]=0;
  ledData[size+2]=0;
    
  size(640,480);  // create the window
  simpleScreenCapture = new SimpleScreenCapture();
}

 
// runs for each new frame of movie data
void movieEvent() {
  // read the movie's next frame
  img = simpleScreenCapture.get();
  
  //cpy.copy(img,0,0,img.width,img.height,0,0,img.width,img.height);
  
  for (int i=0; i < numPorts; i++) {    
    // copy a portion of the movie's image to the LED image
    int xoffset = percentage(img.width, ledArea[i].x);
    int yoffset = percentage(img.height, ledArea[i].y);
    int xwidth =  percentage(img.width, ledArea[i].width);
    int yheight = percentage(img.height, ledArea[i].height);
    ledImage[i].copy(img, xoffset, yoffset, xwidth, yheight,
                     0, 0, ledImage[i].width, ledImage[i].height);
    // convert the LED image to raw data
    image2data(ledImage[i], ledData, ledLayout[i]);/*
    if (i == 0) {
      ledData[0] = '*';  // first Teensy is the frame sync master
      int usec = (int)((1000000.0 / framerate) * 0.75);
      ledData[1] = (byte)(usec);   // request the frame sync pulse
      ledData[2] = (byte)(usec >> 8); // at 75% of the frame time
    } else {
      ledData[0] = '%';  // others sync to the master board
      ledData[1] = 0;
      ledData[2] = 0;
    }*/
    // send the raw data to the LEDs  :-)
    ledSerial[i].write(ledData); 
  }
}


int led_map(int input) {
  int row = input / LEDS_PER_STRIP;
  input %= LEDS_PER_STRIP;
  
  int y = input / ( SCREEN_WIDTH );
  int x = input % ( SCREEN_WIDTH );
  
  if((x%2)==1) {
    y = 7-y;
  }
  
  int output = row * LEDS_PER_STRIP
             + x * PANEL_HEIGHT
             + y;
  return output;
}


int led_map(int x,int y) {
  return led_map(y * SCREEN_WIDTH + x);
}



// image2data converts an image to OctoWS2811's raw data format.
// The number of vertical pixels in the image must be a multiple
// of 8.  The data array must be the proper size for the image.
void image2data(PImage image, byte[] data, boolean layout) {
  int offset, x, y, mask, pixel, i=0;
  int size = image.height * image.width;

  for (y = 0; y < image.height; y++) {
    for (x = 0; x < image.width; x++) {
      pixel = image.pixels[i++];
      int r = ( pixel & 0xFF0000 ) >> 16; 
      int g = ( pixel & 0x00FF00 ) >>  8; 
      int b = ( pixel & 0x0000FF );
      
      if( r==0 ) r=1;
      if( g==0 ) g=1;
      if( b==0 ) b=1;
      
      offset = led_map(x,y)*3;
      data[offset++] = (byte)(r);
      data[offset++] = (byte)(g);
      data[offset++] = (byte)(b);
    }
  }
}


// ask a Teensy board for its LED configuration, and set up the info for it.
void serialConfigure(String portName) {
  if (numPorts >= maxPorts) {
    println("too many serial ports, please increase maxPorts");
    errorCount++;
    return;
  }
  try {
    ledSerial[numPorts] = new Serial(this, portName);
    if (ledSerial[numPorts] == null) throw new NullPointerException();
    ledSerial[numPorts].write('?');
  } catch (Throwable e) {
    println("Serial port " + portName + " does not exist or is non-functional");
    errorCount++;
    return;
  }
  delay(50);/*
  String line = ledSerial[numPorts].readStringUntil(10);
  if (line == null) {
    println("Serial port " + portName + " is not responding.");
    println("Is it really a Teensy 3.0 running VideoDisplay?");
    errorCount++;
    return;
  }
  */
  print("port "+numPorts+": ");
  String line = "32,24,0,0,0,0,0,100,100,0,0,0";
  String param[] = line.split(",");
  if (param.length != 12) {
    println("Error: port " + portName + " did not respond to LED config query");
    errorCount++;
    return;
  }
  int w = Integer.parseInt(param[0]);
  int h = Integer.parseInt(param[1]);
  // only store the info and increase numPorts if Teensy responds properly
  ledImage[numPorts] = new PImage(w, h, RGB);
  ledArea[numPorts] = new Rectangle(Integer.parseInt(param[5]), Integer.parseInt(param[6]),
                     Integer.parseInt(param[7]), Integer.parseInt(param[8]));
  ledLayout[numPorts] = (Integer.parseInt(param[5]) == 0);
  numPorts++;
  if(maxW < w) maxW = w;
  if(maxH < h) maxH = h;
}


// draw runs every time the screen is redrawn - show the movie...
void draw() {
  movieEvent();
   
  // show the original video
  //image(img, 0,80,640,415);
  //image(img, 0,80,640,415);
  image(ledImage[0], 0,80,640,415);
  
  // then try to show what was most recently sent to the LEDs
  // by displaying all the images for each port.
  for (int i=0; i < numPorts; i++) {
    // compute the intended size of the entire LED array
    int xsize = percentageInverse(ledImage[i].width, ledArea[i].width);
    int ysize = percentageInverse(ledImage[i].height, ledArea[i].height);
    // computer this image's position within it
    int xloc =  percentage(xsize, ledArea[i].x);
    int yloc =  percentage(ysize, ledArea[i].y);
    // show what should appear on the LEDs
    image(ledImage[i], 240 - xsize / 2 + xloc, 10 + yloc);
  }
}


// scale a number by a percentage, from 0 to 100
int percentage(int num, int percent) {
  double mult = percentageFloat(percent);
  double output = num * mult;
  return (int)output;
}


// scale a number by the inverse of a percentage, from 0 to 100
int percentageInverse(int num, int percent) {
  double div = percentageFloat(percent);
  double output = num / div;
  return (int)output;
}


// convert an integer from 0 to 100 to a float percentage
// from 0.0 to 1.0.  Special cases for 1/3, 1/6, 1/7, etc
// are handled automatically to fix integer rounding.
double percentageFloat(int percent) {
  if (percent == 33) return 1.0 / 3.0;
  if (percent == 17) return 1.0 / 6.0;
  if (percent == 14) return 1.0 / 7.0;
  if (percent == 13) return 1.0 / 8.0;
  if (percent == 11) return 1.0 / 9.0;
  if (percent ==  9) return 1.0 / 11.0;
  if (percent ==  8) return 1.0 / 12.0;
  return (double)percent / 100.0;
}
