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

import processing.serial.*;
import java.awt.Rectangle;
import hypermedia.net.*;
import java.util.concurrent.*;

int numPorts=0;  // the number of serial ports in use
String[] serialPorts = {
  //"COM16", "COM19", "COM17"
  "/dev/tty.usbmodem315451"
};
int maxPorts=1; // maximum number of serial ports

Serial[] ledSerial = new Serial[maxPorts];     // each port's actual Serial port
Rectangle[] ledArea = new Rectangle[maxPorts]; // the area of the movie each port gets, in % (0-100)
boolean[] ledLayout = new boolean[maxPorts];   // layout of rows, true = even is left->right
PImage[] ledImage = new PImage[maxPorts];      // image sent to each port
int errorCount=0;
float framerate=0;

// Display configuration
final static int displayWidth = 32;
final static int displayHeight = 24;
final int ledCount = displayHeight*displayWidth;

boolean VERTICAL = true;
int FRAMERATE = 15;

int packet_length = ledCount*3 + 1;

final int displayPadding = 50;

PImage nextImage = new PImage(displayWidth, displayHeight);

Boolean demoMode = true;
BlockingQueue newImageQueue;

// domestar interaction components
DemoTransmitter demoTransmitter;

UDP udp;






final int PANEL_WIDTH=8;
final int PANEL_HEIGHT=8;
final int PANELS_PER_PIN = 4;
final int LEDS_PER_STRIP = PANEL_WIDTH * PANEL_HEIGHT * PANELS_PER_PIN;

int led_map(int input) {
  int row = input / LEDS_PER_STRIP;
  input %= LEDS_PER_STRIP;
  
  int y = input / ( PANEL_WIDTH * PANELS_PER_PIN );
  int x = input % ( PANEL_WIDTH * PANELS_PER_PIN );
  
  if((x%2)==1) {
    y = 7-y;
  }
  
  int output = row * LEDS_PER_STRIP
             + x * PANEL_HEIGHT
             + y;
  return output;
}


int led_map(int x,int y) {
  return led_map(y * PANEL_WIDTH * PANELS_PER_PIN + x);
}




void setup() {
  // create window
  size(60+2*displayPadding, 32+2*displayPadding);  // create the window
  //size(displayWidth, displayHeight);
  //size(480, 400);  // create the window

  // configure output
  String[] list = Serial.list();
  delay(20);
  println("Serial Ports List:");
  println(list);
  for (int i=0; i<maxPorts; i++) {
    serialConfigure(serialPorts[i]);
  }
  if (errorCount > 0) exit();

  newImageQueue = new ArrayBlockingQueue(2);

  // configure input
  udp = new UDP( this, 58082 );
  udp.listen( true );

  demoTransmitter = new DemoTransmitter();
  demoTransmitter.start();
}

int maxConvertedByte = 0;

int convertByte(byte b) {
  int c = (b<0) ? 256+b : b;

  if (c > maxConvertedByte) {
    maxConvertedByte = c;
    println("Max Converted Byte is now " + c);
  }  

  return c;
}

void receive(byte[] data, String ip, int port) {  
  //println(" new datas!");
  if (demoMode) {
    println("Started receiving data from " + ip + ". Demo mode disabled.");
    demoMode = false;
  }
/*
  if (data[0] == 2) {
    // We got a new mode, so copy it out
    String modeName = new String(data);
    return;
  }

  if (data[0] != 1) {
    println("Packet header mismatch. Expected 1, got " + data[0]);
    return;
  }
*/
  if (data.length != packet_length) {
    println("Packet size mismatch. Expected "+packet_length+", got " + data.length);
    return;
  }

  if (newImageQueue.size() > 0) {
    println("Buffer full, dropping frame!");
    return;
  }

  color[] newImage = new color[ledCount];

  for (int i=0; i< (ledCount); i++) {
    // Processing doesn't like it when you call the color function while in an event
    // go figure
    newImage[i] = (int)(0xff<<24 | convertByte(data[i*3 + 1])<<16) |
      (convertByte(data[i*3 + 2])<<8) |
      (convertByte(data[i*3 + 3]));
  }
  try {
    newImageQueue.put(newImage);
  } 
  catch( InterruptedException e ) {
    println("Interrupted Exception caught");
  }
}

// movieEvent runs for each new frame of movie data
void frameUpdate(PImage f) {
  for (int i=0; i < numPorts; i++) {    
    // copy a portion of the movie's image to the LED image
    int xoffset = percentage(f.width, ledArea[i].x);
    int yoffset = percentage(f.height, ledArea[i].y);
    int xwidth =  percentage(f.width, ledArea[i].width);
    int yheight = percentage(f.height, ledArea[i].height);

    ledImage[i].copy(f, xoffset, yoffset, xwidth, yheight, 0, 0, ledImage[i].width, ledImage[i].height);
//    println(i+": x"+xoffset+" y"+yoffset+" w"+xwidth+" h"+yheight+" "+0+" "+0+" "+ledImage[i].width+" "+ledImage[i].height);

    // convert the LED image to raw data
    byte[] ledData =  new byte[ledImage[i].width * ledImage[i].height * 3];
    
    image2data(ledImage[i], ledData, ledLayout[i]);
    ledSerial[i].write(ledData);
  }
}


// image2data converts an image to OctoWS2811's raw data format.
// The number of vertical pixels in the image must be a multiple
// of 8.  The data array must be the proper size for the image.
void image2data(PImage image, byte[] data, boolean layout) {
  int offset = 0;
  int x, y, mask;
  int pixel;
  int i=0;

  for (y = 0; y < image.height; y++) {
    for (x = 0; x < image.width; x++) {
      pixel = image.pixels[i++];
      int r = ( pixel ) >> 16; 
      int g = ( pixel ) >>  8; 
      int b = ( pixel );
      offset = led_map(x,y)*3;
      data[offset++] = (byte)(r & 0xfe);
      data[offset++] = (byte)(g & 0xfe);
      data[offset++] = (byte)(b & 0xfe);
    }
  }
  offset=0;
  data[offset++] |= (byte)0x01;
  data[offset++] |= (byte)0x01;
  data[offset++] |= (byte)0x01;
}


// image2data converts an image to OctoWS2811's raw data format.
// The number of vertical pixels in the image must be a multiple
// of 8.  The data array must be the proper size for the image.
void image2data_old(PImage image, byte[] data, boolean layout) {
  int offset = 3;
  int x, y, xbegin, xend, xinc, mask;
  int linesPerPin = image.height / 8;
  int pixel[] = new int[8];

  for (y = 0; y < linesPerPin; y++) {
    if ((y & 1) == (layout ? 0 : 1)) {
      // even numbered rows are left to right
      xbegin = 0;
      xend = image.width;
      xinc = 1;
    } 
    else {
      // odd numbered rows are right to left
      xbegin = image.width - 1;
      xend = -1;
      xinc = -1;
    }
    for (x = xbegin; x != xend; x += xinc) {
      for (int i=0; i < 8; i++) {
        // fetch 8 pixels from the image, 1 for each pin
        pixel[i] = image.pixels[x + (y + linesPerPin * i) * image.width];
        pixel[i] = colorWiring(pixel[i]);
      }
      // convert 8 pixels to 24 bytes
      for (mask = 0x800000; mask != 0; mask >>= 1) {
        byte b = 0;
        for (int i=0; i < 8; i++) {
          if ((pixel[i] & mask) != 0) b |= (1 << i);
        }
        data[offset++] = b;
      }
    }
  }
}

// translate the 24 bit color from RGB to the actual
// order used by the LED wiring.  GRB is the most common.
int colorWiring(int c) {
  return c;  // RGB
  //return ((c & 0xFF0000)) | ((c & 0x00FF00)) | (c & 0x0000FF); // GRB - most common wiring
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
  } 
  catch (Throwable e) {
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

  String param[] = line.split(",");
  if (param.length != 12) {
    println("Error: port " + portName + " did not respond to LED config query");
    errorCount++;
    return;
  }*/
  print("port "+numPorts+": ");
  String line = "32,24,0,0,0,0,0,100,100,0,0,0";
  print(line);

  String param[] = line.split(",");

  // only store the info and increase numPorts if Teensy responds properly
  int w1=Integer.parseInt(param[0]);
  int h1=Integer.parseInt(param[1]);
  int x2=Integer.parseInt(param[5]);
  int y2=Integer.parseInt(param[6]);
  int w2=Integer.parseInt(param[7]);
  int h2=Integer.parseInt(param[8]);
  /*
  print("\nw1="+w1);
  print("\th1="+h1);
  print("\tx2="+x2);
  print("\ty2="+y2);
  print("\tw2="+w2);
  print("\th2="+h2);
  print("\n");
  */
  ledImage[numPorts] = new PImage(w1, h1, RGB);
  ledArea[numPorts] = new Rectangle(x2,y2,w2,h2);
  ledLayout[numPorts] = (x2 == 0);
  numPorts++;
}


// draw runs every time the screen is redrawn - show the movie...
void draw() {
  if (newImageQueue.size() > 0) {
    color[] newImage = (color[])newImageQueue.remove();

    // now need to stuff the values into a PImage
    nextImage.loadPixels();
    for (int i=0; i<displayHeight; i++) {
      for (int j=0; j<displayWidth; j++) {
        //int loc = i*displayWidth+j;
        int loc = i+j*displayHeight;

        // Set the display pixel to the image pixel
        nextImage.pixels[loc] = color(newImage[loc]);
      }
    }
    nextImage.updatePixels();


    image(nextImage, displayPadding, displayPadding);
    /*
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
     */

    frameUpdate(nextImage);
  }
}

// respond to mouse clicks as pause/play
boolean isPlaying = true;
void mousePressed() {
  if (isPlaying) {
    isPlaying = false;
  } 
  else {
    isPlaying = true;
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

