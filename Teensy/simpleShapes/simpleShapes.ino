// 2016-02-02 dan@marginallyclever.com
// simple shapes line marquee, lines, triangles.
// basic animations, too.

#include <OctoWS2811.h>

//OctoWS2811 Defn. Stuff
#define PANEL_WIDTH 8
#define PANEL_HEIGHT 8
#define COLS_LEDs 8*4 // all of the following params need to be adjusted for screen size
#define ROWS_LEDs 8*3  // LED_LAYOUT assumed 0 if ROWS_LEDs > 8
#define LEDS_PER_STRIP (8*8*4)//(COLS_LEDs * (ROWS_LEDs / 6))
#define PANELS_PER_PIN 4

#define PW (COLS_LEDs)
#define PH (ROWS_LEDs)

DMAMEM int displayMemory[LEDS_PER_STRIP*6];
int drawingMemory[LEDS_PER_STRIP*6];
const int config = WS2811_RGB | WS2811_800kHz;
OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, config);

//Byte val 2PI Cosine Wave, offset by 1 PI 
//supports fast trig calcs and smooth LED fading/pulsing.
uint8_t const cos_wave[256] PROGMEM =  
{0,0,0,0,1,1,1,2,2,3,4,5,6,6,8,9,10,11,12,14,15,17,18,20,22,23,25,27,29,31,33,35,38,40,42,
45,47,49,52,54,57,60,62,65,68,71,73,76,79,82,85,88,91,94,97,100,103,106,109,113,116,119,
122,125,128,131,135,138,141,144,147,150,153,156,159,162,165,168,171,174,177,180,183,186,
189,191,194,197,199,202,204,207,209,212,214,216,218,221,223,225,227,229,231,232,234,236,
238,239,241,242,243,245,246,247,248,249,250,251,252,252,253,253,254,254,255,255,255,255,
255,255,255,255,254,254,253,253,252,252,251,250,249,248,247,246,245,243,242,241,239,238,
236,234,232,231,229,227,225,223,221,218,216,214,212,209,207,204,202,199,197,194,191,189,
186,183,180,177,174,171,168,165,162,159,156,153,150,147,144,141,138,135,131,128,125,122,
119,116,113,109,106,103,100,97,94,91,88,85,82,79,76,73,71,68,65,62,60,57,54,52,49,47,45,
42,40,38,35,33,31,29,27,25,23,22,20,18,17,15,14,12,11,10,9,8,6,6,5,4,3,2,2,1,1,1,0,0,0,0
};


//Gamma Correction Curve
uint8_t const exp_gamma[256] PROGMEM =
{0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,3,3,3,3,3,
4,4,4,4,4,5,5,5,5,5,6,6,6,7,7,7,7,8,8,8,9,9,9,10,10,10,11,11,12,12,12,13,13,14,14,14,15,15,
16,16,17,17,18,18,19,19,20,20,21,21,22,23,23,24,24,25,26,26,27,28,28,29,30,30,31,32,32,33,
34,35,35,36,37,38,39,39,40,41,42,43,44,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,
61,62,63,64,65,66,67,68,70,71,72,73,74,75,77,78,79,80,82,83,84,85,87,89,91,92,93,95,96,98,
99,100,101,102,105,106,108,109,111,112,114,115,117,118,120,121,123,125,126,128,130,131,133,
135,136,138,140,142,143,145,147,149,151,152,154,156,158,160,162,164,165,167,169,171,173,175,
177,179,181,183,185,187,190,192,194,196,198,200,202,204,207,209,211,213,216,218,220,222,225,
227,229,232,234,236,239,241,244,246,249,251,253,254,255
};

char makeTimeMask[] = {
0b01000010, 0b00011000, 0b01000010, 0b01111110,
0b01100110, 0b00100100, 0b01000100, 0b01000000,
0b01011010, 0b00100100, 0b01001000, 0b01000000,
0b01000010, 0b01000010, 0b01010000, 0b01000000,
0b01000010, 0b01000010, 0b01110000, 0b01110000,
0b01000010, 0b01111110, 0b01001000, 0b01000000,
0b01000010, 0b01000010, 0b01000100, 0b01000000,
0b01000010, 0b01000010, 0b01000010, 0b01111110,

0b00000000, 0b00000000, 0b00000000, 0b00000000,
0b00000000, 0b00000000, 0b00000000, 0b00000000,
0b00000000, 0b00000000, 0b00000000, 0b00000000,
0b00000000, 0b00000000, 0b00000000, 0b00000000,
0b00000000, 0b00000000, 0b00000000, 0b00000000,
0b00000000, 0b00000000, 0b00000000, 0b00000000,
0b00000000, 0b00000000, 0b00000000, 0b00000000,
0b00000000, 0b00000000, 0b00000000, 0b00000000,

0b11111110, 0b00010000, 0b01000010, 0b01111110,
0b00010000, 0b00010000, 0b01100110, 0b01000000,
0b00010000, 0b00010000, 0b01011010, 0b01000000,
0b00010000, 0b00010000, 0b01000010, 0b01000000,
0b00010000, 0b00010000, 0b01000010, 0b01110000,
0b00010000, 0b00010000, 0b01000010, 0b01000000,
0b00010000, 0b00010000, 0b01000010, 0b01000000,
0b00010000, 0b00010000, 0b01000010, 0b01111110,
};

#define RAINBOW_LEN   (11)
char rainbow[] = {
  255,  0,  0,
  255,128,  0,
  255,255,  0,
  128,255,  0,
    0,255,  0,
    0,255,128,
    0,255,255,
    0,128,255,
    0,  0,255,
  128,  0,255,
  255,  0,128,
  255,  0,  0,
};


// location of each point on the triangle
float px1=PW/2.0, py1=PH/2.0;
float px2=px1, py2=py1;
float px3=px1, py3=py1;

// velocity of each point on the triangle
float xv1=0.02, yv1=0.023;
float xv2=0.04, yv2=-0.01;
float xv3=-0.05, yv3=-0.08;



void setup() {
  pinMode(13, OUTPUT);
  leds.begin();
  leds.show();
}


int led_map(int input) {
  int row = input / LEDS_PER_STRIP;
  input %= LEDS_PER_STRIP;
  
  int y = input / ( PANEL_WIDTH * PANELS_PER_PIN );
  int x = input % ( PANEL_WIDTH * PANELS_PER_PIN );
  
  if(x%2) {
    y = 7-y;
  }
  
  int output = row * LEDS_PER_STRIP
             + x * PANEL_HEIGHT
             + y;
  return output;
}


int makeColor(int red, int green, int blue) {
  red = exp_gamma[red];
  green = exp_gamma[green];
  blue = exp_gamma[blue];
  return ((red&0xff)<<16) + ((green&0xff) << 8) + (blue&0xff);
}


inline float linearInterpolation(float a,float b,float c) {
  return (b-a)*c + a;
}


// i from [0...1).  never reach 1.
int getRainbow(float i) {
  if(i< 0) i=0;
  if(i>=1) i=1;
  float k = i*(RAINBOW_LEN+1);
  int a = floor(k);
  int b = ceil(k);
  float c = k - floor(k);
  if(b>= RAINBOW_LEN) b = 0;
  
  int red   = linearInterpolation(rainbow[a*3+0],rainbow[(b*3+0)],c);
  int green = linearInterpolation(rainbow[a*3+1],rainbow[(b*3+1)],c);
  int blue  = linearInterpolation(rainbow[a*3+2],rainbow[(b*3+2)],c);

  return makeColor(red,green,blue);
}


int led_map(int x,int y) {
  return led_map(mask(x, y));
}


int mask(int x,int y) {
  return y * PANEL_WIDTH * PANELS_PER_PIN + x;
}


void fillScreen(int color) {
  for(int i=0;i<PW*PH;++i) {
    leds.setPixel(i, color );
  }
}


// prevent drawing off edge of screen
void clip(int x,int y,int color) {
  if(x <   0) return;
  if(x >= PW) return;
  if(y <   0) return;
  if(y >= PH) return;
  leds.setPixel(led_map(x,y), color);
}


// See https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void line(int x1,int y1, int x2,int y2,int color) {
  int dx = x2-x1;
  int dy = y2-y1;
  int dirX = dx>0 ? 1 : -1;
  int dirY = dy>0 ? 1 : -1;
  dx *= dirX;
  dy *= dirY;
  int i;
  int sum = 0;

  int x = x1;
  int y = y1;
  
  if(dx > dy) {
    sum=dy;
    for(i=0;i<dx;++i) {
      clip(x,y, color);
      sum+=dy;
      x+=dirX;
      if(sum>=dx) {
        y+=dirY;
        sum -= dx;
      }
    }
  } else {
    sum=dx;
    for(i=0;i<dy;++i) {
      clip(x,y, color);
      sum+=dx;
      y+=dirY;
      if(sum>=dy) {
        x+=dirX;
        sum -= dy;
      }
    }
  }
}

// See https://en.wikipedia.org/wiki/Midpoint_circle_algorithm#Variant_with_Integer-Based_Arithmetic
void circle(int x0, int y0, int radius,int color) {
  int x = radius;
  int y = 0;
  int decisionOver2 = 1 - x;   // Decision criterion divided by 2 evaluated at x=r, y=0

  while( y <= x ) {
    clip( x + x0,  y + y0,color); // Octant 1
    clip( y + x0,  x + y0,color); // Octant 2
    clip(-x + x0,  y + y0,color); // Octant 4
    clip(-y + x0,  x + y0,color); // Octant 3
    clip(-x + x0, -y + y0,color); // Octant 5
    clip(-y + x0, -x + y0,color); // Octant 6
    clip( x + x0, -y + y0,color); // Octant 7
    clip( y + x0, -x + y0,color); // Octant 8
    y++;
    if (decisionOver2<=0) {
      decisionOver2 += 2 * y + 1;   // Change in decision criterion for y -> y+1
    } else {
      x--;
      decisionOver2 += 2 * (y - x) + 1;   // Change for y -> y+1, x -> x-1
    }
  }
}


void loop() {
  // clear screen
  fillScreen(0);

  // m = seconds since start
  long m = millis() * 0.001;
  // m = time until demos loop and restart
  m %= 50;

  // each animation
  if(m < 10) {
    clockHands();
  } else if(m<20) {
    rectangleTunnel();
  } else if(m<30) {
    bouncingTriangle();
    marquee();
  } else if(m<40) {
    circleTunnel();
  } else {
    nyanCat();
  }
  
  // show the work
  leds.show();  // not sure if this function is needed  to update each frame
}


void marquee() {
  int colorOn = makeColor(255,200,0);  // blue
  int colorOff = makeColor(0,0,0);  // black

  int i;
  int j = millis() * 6.0 * 0.001;

  // top row, left to right
  for( i=0;i<PW;++i) {
    leds.setPixel(led_map(i,0), ((j++)%3)==0 ? colorOn:colorOff );
  }
  // right column, top to bottom
  for( i=0;i<PH;++i) { 
    leds.setPixel(led_map(PW-1,i), ((j++)%3)==0 ? colorOn:colorOff );
  }
  // bottom row, right to left
  for( i=0;i<PW;++i) {
    leds.setPixel(led_map(PW-1-i,PH-1), ((j++)%3)==0 ? colorOn:colorOff );
  } 
  // left column, top to bottom
  for( i=0;i<PH;++i) { 
    leds.setPixel(led_map(0,PH-1-i), ((j++)%3)==0 ? colorOn:colorOff );
  }
}


void maskTime() {
  int x,y;
  float f = ( millis() * 0.001 );
  int d = getRainbow( f - floor(f) );
  
  for(y=0;y<PH;++y) {
    for(x=0;x<PW;++x) {
      
      int c = makeTimeMask[(y*4)+(x/8)];
      
      if( (c & (1<<(7-(x%8)))) !=0 ) {
        clip(x,y,d);
      }
    }
  }
}


void clockHands() {
  double j = millis() * 5.0 * 0.001;

  int minuteHand = makeColor(127,127,127);
  int hourHand = makeColor(64,64,64);
  int x1 = PW/2;
  int y1 = PH/2;
  int x2 = cos(j) * 10.0 + x1;
  int y2 = sin(j) * 10.0 + y1;
  line( x1,y1, x2,y2, minuteHand );
  x2 = cos(j/20.0) * 7.0 + x1;
  y2 = sin(j/20.0) * 7.0 + y1;
  line( x1,y1, x2,y2, hourHand );

  circle(x1,y1,10,makeColor(255,0,0));

  //if( (int)(j/2) % 2 ) 
  {
    maskTime();
  }
}


void bouncingTriangle() {
  // draw triangle
  line(px1,py1,px2,py2, makeColor(255,0,0));
  line(px2,py2,px3,py3, makeColor(0,255,0));
  line(px3,py3,px1,py1, makeColor(0,0,255));

  // move points of triangle
  px1 += xv1;
  py1 += yv1;
  
  px2 += xv2;
  py2 += yv2;
  
  px3 += xv3;
  py3 += yv3;

  // bounce them off the edges
  if(px1>=PW || px1<0) xv1*=-1;
  if(px2>=PW || px2<0) xv2*=-1;
  if(px3>=PW || px3<0) xv3*=-1;

  if(py1>=PH || py1<0) yv1*=-1;
  if(py2>=PH || py2<0) yv2*=-1;
  if(py3>=PH || py3<0) yv3*=-1;
}


void rectangleTunnel() {
  // move points of triangle
  px1 -= xv1*3.0;
  py1 += yv1*3.0;
  
  if(px1>=PW || px1<0) xv1*=-1;
  if(py1>=PH || py1<0) yv1*=-1;

  double j = millis() * 20.0 * 0.001;
  int i, c;
  
  //Colour the center pixel
  clip(px1, py1, getRainbow((float)((int)(j)%PW)/(float)PW));
  for(i=1;i<PW;++i) {
    c = getRainbow((float)((int)(j-i)%PW)/(float)PW);
    
    line(px1-i,py1-i,px1+i,py1-i,c);
    line(px1+i,py1-i,px1+i,py1+i,c);
    line(px1+i,py1+i,px1-i,py1+i,c);
    line(px1-i,py1+i,px1-i,py1-i,c);
  }
}


void circleTunnel1() {
  // move points of triangle
  px2 -= xv2*3.0;
  py2 += yv2*3.0;
  
  if(px2>=PW || px2<0) xv2*=-1;
  if(py2>=PH || py2<0) yv2*=-1;

  double j = millis() * 30.0 * 0.001;
  
  int i, c;
  
  for(i=0;i<PW*1.2;++i) {
    c = getRainbow((float)((int)(i+j)%PW)/(float)PW);
    
    circle(px2,py2,i,c);
  }
}


#define CTLEN  (int)(PW*1.2)

int cx[CTLEN];//XY coords of circle buffer
int cy[CTLEN];//XY coords of circle buffer
int cMax = 1;
float tMark = 0;
int cN=0;

void circleTunnel() {
  // move points of triangle
  px2 += xv3;
  py2 += yv3;
  
  if(px2>=PW*3/4 || px2<PW*1/4) xv3*=-1;
  if(py2>=PH*3/4 || py2<PH*1/4) yv3*=-1;
  
  cx[0] = px2;
  cy[0] = py2;
  if(cMax < CTLEN-1) cMax++;
  
  int i, c;

  float t = millis() * 0.001f;
  if( (t-tMark) > 0.05f ) {
    for(i=cMax;i>0;--i) {
      cx[i] = cx[i-1];
      cy[i] = cy[i-1];
    }
    tMark = t;
    cN++;
    //xv3 = ((( rand() % 100 ) - 50 ) / 50.0 ) * 0.50;
    //yv3 = ((( rand() % 100 ) - 50 ) / 50.0 ) * 0.50;
  }
    
  for(i=0;i<=cMax;++i) {
    c = getRainbow( (float)( (((int)(cMax-i+cN)%cMax) )) / (float)cMax);
    circle(cx[i],cy[i],i,c);
    circle(cx[i],cy[i],i+1,c);
  }
}


#define colorBlue   makeColor(0x0f,0x4d,0x8f)
#define colorBlack  makeColor(0,0,0)
#define colorTan    makeColor(0xF8,0xCD,0xA0)
#define colorPink   makeColor(0xFC,0x9B,0xFD)
#define colorRose   makeColor(0xFF,0x2E,0x9A)
#define colorGrey   makeColor(0x99,0x99,0x99)
#define colorBlush  makeColor(0xF6,0x9C,0xA1)
#define colorWhite  makeColor(0xFF,0xFF,0xFF)

int catColors[] = {
  colorBlue,  // 0 is clear
  colorBlack,  // 1
  colorTan,    // 2
  colorPink,   // 3
  colorRose,   // 4
  colorGrey,   // 5
  colorBlush,  // 6
  colorWhite,  // 7
};

char catMask[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0,
0,0,0,0,0,0,1,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,1,0,0,0,0,0,
0,0,0,0,0,0,1,2,2,3,3,3,3,3,3,4,3,3,4,3,3,3,3,3,2,2,1,0,0,0,0,0,
0,0,0,0,0,0,1,2,3,3,4,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,1,0,0,0,0,0,
0,0,0,0,0,0,1,2,3,3,3,3,3,3,3,3,3,3,1,1,3,3,4,3,3,2,1,0,1,1,0,0,

0,0,0,0,0,0,1,2,3,3,3,3,3,3,3,3,3,1,5,5,1,3,3,3,3,2,1,1,5,5,1,0,
1,1,1,1,0,0,1,2,3,3,3,3,3,3,4,3,3,1,5,5,5,1,3,3,3,2,1,5,5,5,1,0,
1,5,5,1,1,0,1,2,3,3,3,3,3,3,3,3,3,1,5,5,5,5,1,1,1,1,5,5,5,5,1,0,
1,1,5,5,1,1,1,2,3,3,3,4,3,3,3,3,3,1,5,5,5,5,5,5,5,5,5,5,5,5,1,0,
0,1,1,5,5,1,1,2,3,3,3,3,3,3,3,4,1,5,5,5,5,5,5,5,5,5,5,5,5,5,5,1,
0,0,1,1,5,5,1,2,3,4,3,3,3,3,3,3,1,5,5,5,7,1,5,5,5,5,5,7,1,5,5,1,
0,0,0,1,1,1,1,2,3,3,3,3,3,3,3,3,1,5,5,5,1,1,5,5,5,1,5,1,1,5,5,1,
0,0,0,0,0,1,1,2,3,3,3,3,3,4,3,3,1,5,6,6,5,5,5,5,5,5,5,5,5,6,6,1,

0,0,0,0,0,0,1,2,2,3,4,3,3,3,3,3,1,5,6,6,5,1,5,5,1,5,5,1,5,6,6,1,
0,0,0,0,0,0,1,2,2,2,3,3,3,3,3,3,3,1,5,5,5,1,1,1,1,1,1,1,5,5,1,0,
0,0,0,0,0,1,1,1,2,2,2,2,2,2,2,2,2,2,1,5,5,5,5,5,5,5,5,5,5,1,0,0,
0,0,0,0,1,5,5,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
0,0,0,0,1,5,5,1,1,0,1,5,5,1,0,0,0,0,0,1,5,5,1,0,1,5,5,1,0,0,0,0,
0,0,0,0,1,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,1,1,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};


void nyanCat() {
  int x,y,c;
  int f = (int)(millis() * 0.003)%2;
  
  fillScreen(colorBlue);
  
  for(y=0;y<PH;++y) {
    for(x=0;x<PW;++x) {
      if(y+f<PH) {
        c = catMask[(y+f)*PW+x];
        if(c!=0) {
          clip(x,y,catColors[c]);
        }
      }
    }
  }
}


inline uint8_t fastCosineCalc( uint16_t preWrapVal) {
  uint8_t wrapVal = (preWrapVal % 255);
  if (wrapVal<0) wrapVal=255+wrapVal;
  return (pgm_read_byte_near(cos_wave+wrapVal)); 
}
