
/*
    Basic Pin setup:
    ------------                                  ---u----
    ARDUINO   13|-> SCLK (pin 25)           OUT1 |1     28| OUT channel 0
              12|                           OUT2 |2     27|-> GND (VPRG)
              11|-> SIN (pin 26)            OUT3 |3     26|-> SIN (pin 11)
              10|-> BLANK (pin 23)          OUT4 |4     25|-> SCLK (pin 13)
               9|-> XLAT (pin 24)             .  |5     24|-> XLAT (pin 9)
               8|                             .  |6     23|-> BLANK (pin 10)
               7|                             .  |7     22|-> GND
               6|                             .  |8     21|-> VCC (+5V)
               5|                             .  |9     20|-> 2K Resistor -> GND
               4|                             .  |10    19|-> +5V (DCPRG)
               3|-> GSCLK (pin 18)            .  |11    18|-> GSCLK (pin 3)
               2|                             .  |12    17|-> SOUT
               1|                             .  |13    16|-> XERR
               0|                           OUT14|14    15| OUT channel 15
    ------------                                  --------

    -  Put the longer leg (anode) of the LEDs in the +5V and the shorter leg
         (cathode) in OUT(0-15).
    -  +5V from Arduino -> TLC pin 21 and 19     (VCC and DCPRG)
    -  GND from Arduino -> TLC pin 22 and 27     (GND and VPRG)
    -  digital 3        -> TLC pin 18            (GSCLK)
    -  digital 9        -> TLC pin 24            (XLAT)
    -  digital 10       -> TLC pin 23            (BLANK)
    -  digital 11       -> TLC pin 26            (SIN)
    -  digital 13       -> TLC pin 25            (SCLK)
    -  The 2K resistor between TLC pin 20 and GND will let ~20mA through each
       LED.  To be precise, it's I = 39.06 / R (in ohms).  This doesn't depend
       on the LED driving voltage.
    - (Optional): put a pull-up resistor (~10k) between +5V and BLANK so that
                  all the LEDs will turn off when the Arduino is reset.

    If you are daisy-chaining more than one TLC, connect the SOUT of the first
    TLC to the SIN of the next.  All the other pins should just be connected
    together:
        BLANK on Arduino -> BLANK of TLC1 -> BLANK of TLC2 -> ...
        XLAT on Arduino  -> XLAT of TLC1  -> XLAT of TLC2  -> ...
    The one exception is that each TLC needs it's own resistor between pin 20
    and GND.

    This library uses the PWM output ability of digital pins 3, 9, 10, and 11.
    Do not use analogWrite(...) on these pins.

    This sketch does the Knight Rider strobe across a line of LEDs.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03 */
#include "Tlc5940.h"
#include "FastShiftOut.h"
/* #include "SparkFun_Tlc5940.h"*/

#define _74HC595_LATCH_PIN   A4
#define _74HC595_CLOCK_PIN   A5
#define _74HC595_DATA_PIN    A3
 
#define MAX_GREYSCALES      16
#define MAX_LEDS            NUM_TLCS * 16
#define TIME_TRANSITION     70

#define LEVEL_0 0b01110000
#define LEVEL_1 0b10110000
#define LEVEL_2 0b11010000
#define LEVEL_3 0b11100000
#define LEVEL_ALL 0b00000000
#define LEVEL_NONE 0b11110000
#define LEVEL_TOPBOTTOM LEVEL_0 & LEVEL_3
#define LEVEL_MIDDLES LEVEL_1 & LEVEL_2

#define nb_leds 7
const byte dataArray[nb_leds] =  { LEVEL_0, LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_ALL, LEVEL_TOPBOTTOM, LEVEL_MIDDLES };

 
int greyscales[MAX_GREYSCALES] = {0, 10, 25, 50, 75, 100, 150, 200, 300, 400, 500, 750, 1000, 2000, 3000, 4095 };
byte valeur_data = LEVEL_ALL;// all mosfets high
bool up_going = true;

FastShiftOut FSO(_74HC595_DATA_PIN, _74HC595_CLOCK_PIN, MSBFIRST);

void setup()
{
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  Tlc.init();
  Tlc.clear();
  Tlc.update();

  
  pinMode( _74HC595_LATCH_PIN, OUTPUT);
//  pinMode( _74HC595_CLOCK_PIN, OUTPUT);
  
//  pinMode( _74HC595_DATA_PIN,  OUTPUT);
//  Serial.begin(9600);
}

void SetLEDsLevel( byte lvl1, byte lvl2, byte lvl3, byte lvl4 )
{
  byte i = ~(lvl1 << 7 | lvl2 << 6 | lvl3 << 5 | lvl4 << 4 | 0b00001111);  
  digitalWrite( _74HC595_LATCH_PIN, LOW);
  
  shiftOut( _74HC595_DATA_PIN, _74HC595_CLOCK_PIN, MSBFIRST, i );
  
  //FSO.write(i);
  
  digitalWrite( _74HC595_LATCH_PIN, HIGH);
}

void SetLEDsByteLevel( byte lvl )
{
  PORTC &= ~(1 << PORTC4);
  //digitalWrite( _74HC595_LATCH_PIN, LOW);
  //shiftOut( _74HC595_DATA_PIN, _74HC595_CLOCK_PIN, MSBFIRST, lvl );
  FSO.write(lvl);
  PORTC |= (1 << PORTC4);
  //digitalWrite( _74HC595_LATCH_PIN, HIGH);
}

void KnightRiderEffect()
{
  /* This loop will create a Knight Rider-like effect if you have LEDs plugged
   into all the TLC outputs.  NUM_TLCS is defined in "tlc_config.h" in the
   library folder.  After editing tlc_config.h for your setup, delete the
   Tlc5940.o file to save the changes. */

 
 int direction_lux = 1;
 int direction_led = 1;
 int  greyscale =0;
 int channel = 0;


  while (true) {
    
 
  Tlc.clear();
  Tlc.set(channel, greyscales[greyscale]);
  Tlc.update();

  delay(10);

  switch (greyscale) {
    case MAX_GREYSCALES - 1 :
      direction_lux = -1;
      break;
    case 0 :
      if (direction_lux == -1) {
        channel += direction_led;
      }
      direction_lux = 1;
      break;
   default :
     break;     
  }

  greyscale += direction_lux;
  
  if (channel >= MAX_LEDS - 1) {
    direction_led = -1;
  } else {
    if (channel <= 0) {
      direction_led = 1;
    }
  }
  
  /*if (valeur_data == 255)
  {
    valeur_data = 0;
  }
  else
    {
      valeur_data = 255;
    }*/
    
  digitalWrite( _74HC595_LATCH_PIN, LOW);
  shiftOut( _74HC595_DATA_PIN, _74HC595_CLOCK_PIN, MSBFIRST, valeur_data );
  digitalWrite( _74HC595_LATCH_PIN, HIGH);
  }
}

void FastAsAShark()
{
  //byte valeur_data = 0;// all mosfets high
  int channel = 0;
  int previous = 0;
  byte data = 0;
  byte i = 0;
  byte valeur_data = LEVEL_ALL;// all mosfets high
  
  SetLEDsByteLevel( valeur_data );
  
  while (true) {
    //Tlc.clear();
    Tlc.set(previous, 0);
    Tlc.set(channel, greyscales[MAX_GREYSCALES-1]);
    Tlc.update();   
    previous = channel; 
    delay(0);
    channel ++;
    if (channel >= MAX_LEDS) {
      channel = 0;
      i++;
     if (i >= nb_leds) i = 0; 
    }

    data = dataArray[i] ;
    //ground latchPin and hold low for as long as you are transmitting
    //digitalWrite(_74HC595_LATCH_PIN, LOW);
    //move 'em out
    //shiftOut(_74HC595_DATA_PIN, _74HC595_CLOCK_PIN, MSBFIRST, data);
    //return the latch pin high to signal chip that it 
    //no longer needs to listen for information
    //digitalWrite(_74HC595_LATCH_PIN, HIGH);    
    delayMicroseconds(3);
  }

  
}

void LoopOneAfterTheOther()
{
  byte looplevel = 0;
  byte WalkArray[MAX_LEDS] = {0, 1, 2, 3,    4, 11, 12, 13,    14, 15, 8, 7,    6, 5, 10, 9};
  byte compteur = 0;
  
  while (compteur < 4) {
    SetLEDsByteLevel( dataArray[looplevel] );
    looplevel++;
    if (looplevel >= 4) {
      looplevel = 0;
    }
  
    for (int i = 0; i <= MAX_LEDS-1; i++)
    {
      Tlc.clear();
      Tlc.set(WalkArray[i], greyscales[MAX_GREYSCALES-1]);
      Tlc.update();
      delay(TIME_TRANSITION);
    }
    compteur++;
  }  
}

void Beating() {
  byte j = 0;
  byte compteur = 0;
  int incre = 1;
  
  SetLEDsByteLevel( LEVEL_ALL );
  
  while (compteur < 110) {
    Tlc.clear();    
    for (int i = 0; i <= MAX_LEDS-1; i++)
    {
      Tlc.set(i, greyscales[j]);
    } 
    Tlc.update();
    delay(TIME_TRANSITION);
    
    j = j + incre;
    if (j >= MAX_GREYSCALES-1) {
      incre = -1;
      j = j-1;
    } else {
      if (j <= 0) {
        incre = 1;
        j = 1;
      }
    }
    compteur ++;
  }
}

void BarresVerticales()
{
  const byte MAX_WALKARRAY = MAX_LEDS-4;
  byte WalkArray[MAX_WALKARRAY] = {0, 1, 2, 3,    4, 11, 12, 13,    14, 15, 8, 7};
  byte i = 0;
  byte compteur = 0;
  
  SetLEDsByteLevel( LEVEL_ALL );

  while (compteur <= MAX_WALKARRAY*5) {
    Tlc.clear();    
    Tlc.set(WalkArray[i], greyscales[MAX_GREYSCALES-1]);
    Tlc.update();
    delay(TIME_TRANSITION);
    i++;
    if (i >= MAX_WALKARRAY) i = 0;
    compteur++;
  }
  
}

void BarresHorizontales( bool top_or_sides )
{
  const byte LEDS_PER_FLOOR = MAX_LEDS  / 4;
  const byte MAX_WALKARRAY = MAX_LEDS-4;
  byte WalkArray[MAX_WALKARRAY * 4] = { 12, 11, 4, 3,  12, 11, 4, 3,  12, 11, 4, 3,
                                        12, 11, 4, 3,  13, 10, 5, 2,  14,  9, 6, 1,
                                        15,  8, 7, 0,  15,  8, 7, 0,  15,  8, 7, 0,
                                        15,  8, 7, 0,  14,  9, 6, 1,  13, 10, 5, 2};
  
byte WalkArray2[MAX_WALKARRAY * 4] =  {  0,  1,  2,  3,   0,  1,  2,  3,   0,  1,  2,  3,
                                         0,  1,  2,  3,   4,  5,  6,  7,  11, 10,  9,  8,
                                        12, 13, 14, 15,  12, 13, 14, 15,  12, 13, 14, 15,
                                        12, 13, 14, 15,  11, 10,  9,  8,   4,  5,  6,  7};

  byte WalkFloor[MAX_WALKARRAY] = { LEVEL_0, LEVEL_1, LEVEL_2,
                                    LEVEL_3, LEVEL_3, LEVEL_3,
                                    LEVEL_3, LEVEL_2, LEVEL_1,
                                    LEVEL_0, LEVEL_0, LEVEL_0 };
  byte i = 0; // walking the floors
  byte j = 0; // walking the leds
  byte nextbar = 0; // reme,bers last bar visited
  byte compteur = 0;
  
  while (compteur <= 5) {
    nextbar=0;
    for (i = 0; i <= MAX_WALKARRAY-1; i++) {
      SetLEDsByteLevel( WalkFloor[i] );
    
      Tlc.clear();    
      for (j = nextbar; j < (nextbar+4); j++) { 
        if (top_or_sides) {
          Tlc.set(WalkArray[j], greyscales[MAX_GREYSCALES-1]);
        } else {
          Tlc.set(WalkArray2[j], greyscales[MAX_GREYSCALES-1]);  
        }
      }  
      Tlc.update();
      nextbar = j;
      delay(TIME_TRANSITION);
    }
    compteur++;
  }
}

void Opposites()
{
  bool reached = false;
  const byte MAX_STEPS = 6;
  const byte COORD_CNT = 2;
  const byte WalkArray[MAX_STEPS][COORD_CNT] = 
  { {  0, 12},
    {  1, 13},
    {  2, 14},
    {  3, 15},
    {  4, 8},
    { 11, 7}
  };
  
  int compteur = 0;

  while (compteur >= 0) {
    SetLEDsByteLevel(   dataArray[compteur] );
    
    for (byte i = 0; i < MAX_STEPS; i++) {
      Tlc.clear();    
      Tlc.set(WalkArray[i][0], greyscales[MAX_GREYSCALES-1]);
      Tlc.set(WalkArray[i][1], greyscales[MAX_GREYSCALES-1]);  
      Tlc.update();
      delay(TIME_TRANSITION);
    }
    if (compteur == 3) reached = true;
    if (reached) compteur--; else compteur ++;
  }
}

void OppositesVerticaux( bool sideway  )
{
  #define MAX_STEPS 6
  #define HALF_STEP MAX_STEPS/2
  #define MAX_ROWCOORD 2
  #define MAX_PROG 4

  int Motion[2][3] = { {0, MAX_PROG, 1},
                       {MAX_PROG-2, 0, -1}
                     };
                       
  byte Rows[HALF_STEP][MAX_ROWCOORD] =   { {LEVEL_0, LEVEL_3},
                                           {LEVEL_1, LEVEL_2},
                                           {LEVEL_2, LEVEL_1}};
  byte Cols[MAX_PROG][MAX_STEPS-2][MAX_ROWCOORD] = { 
                                                   {{ 0, 3}, { 1, 2}, { 2, 1}, { 3, 0}},
                                                   {{ 7, 4}, { 6, 5}, { 5, 6}, { 4, 7}},
                                                   {{ 8,11}, { 9,10}, {10, 9}, {11, 8}},
                                                   {{15,12}, {14,13}, {13,14}, {12,15}}
                                                 };
  byte compteur = 0;
  byte pigeon_hole = 0;
  byte previous = 0;

  Tlc.clear();    
  while (compteur < 2) {
    // multi plexing each pair
    for (byte looping = 0; looping < 2; looping++) {
      
      for (int x = Motion[looping][0]; looping == 1 ? (x > Motion[looping][1]) : (x < Motion[looping][1]) ; x = x + Motion[looping][2]) {
        // step rows
        for (byte i = 0; i < MAX_STEPS; i++) {
          for (byte looping = 0; looping < 10; looping++) {
            for (byte y = 0; y < MAX_ROWCOORD; y++) {
              SetLEDsByteLevel(  LEVEL_NONE );
              Tlc.set(previous, greyscales[0]);  
              previous = Cols[x][i<4 ? i : 3][y]; /* there are 6 stages, but only 4 different data, so do a little trick*/
              Tlc.set(previous, greyscales[MAX_GREYSCALES-1]);  
              Tlc.update();
              delayMicroseconds(3);/*delay(1);*/  /* important, otherwise there will be ghosts - maybe gooing microseconds?*/
              SetLEDsByteLevel(  Rows[ i<4 ? 0 : i-3 ][y] ); /* there are 6 stages, but only 3 different data, so do a little trick*/
              delay(3);
            }  
          }
        }
      }
    }
   compteur++;
  }
}                                               

void CubeAQuentin()
{
const byte Etages[5] = { LEVEL_ALL, LEVEL_1, LEVEL_2, LEVEL_0 , LEVEL_3 };
/* this defines which levels will be used. 
   level_all is for the outer vertical bars of the outer cube
   Level_1 and 2 are for the inner cube
   level0 and 3 for the side bars of the outer cube*/

/*Now comes the definition of the two cubes.  The inner one is only 4 coords using levels 1 and 2*/   
const byte InnerCube[4] = {10,9, 6, 5};

/*while the outer one is quite bigger.  It consists of vertical bars using all levels*/
const byte OuterBars[4] = {15, 12, 3, 0};

/*and vertical bars using LEVEL_0 and LEVEL_3*/
const byte OuterSides[8] = {14, 13, 11, 8, 7, 4, 2, 1};

int compteur = 0;
byte previous = 0;
int innerscale = 0;
int outerscale = 0;
int inner_inc = 1;
/*So now, we are going to multiplex the outer part*/

  while (compteur < 5000) {

    for (byte i = 0; i < 4; i++) {
      for (byte j = 1; j < 3; j++) {
        SetLEDsByteLevel(  Etages[j] );
        Tlc.clear();
        Tlc.set(InnerCube[i], greyscales[innerscale]);
        Tlc.update();
        //delayMicroseconds(3);/*delay(1);*/  /* important, otherwise there will be ghosts - maybe gooing microseconds?*/
        delayMicroseconds(700);
      }
    }
    innerscale  = innerscale + inner_inc;
    if (innerscale >= MAX_GREYSCALES) {
      inner_inc = -1;
      outerscale = MAX_GREYSCALES-1;
    };
    if (innerscale < 0) {inner_inc = 1;}; 

    //outerscale

    SetLEDsByteLevel(  Etages[0] );
    for (byte i = 0; i < 4; i++) {
      Tlc.clear();
//      Tlc.set(OuterBars[i], greyscales[outerrscale]);
      Tlc.update();
      //delayMicroseconds(3);/*delay(1);*/  /* important, otherwise there will be ghosts - maybe gooing microseconds?*/
      delayMicroseconds(700);
     
    }

    compteur++;
  }
}

void CubeAQuentin2(int fromain)
{
  /* x =  coordinate horizontal on the cube, 
     y is the layer
     scale is the entry in the 2 byte array that will control the inner and outer grey scales values */
  #define MK_ARR(x, y, scale) ((x << 4) | (y << 1) | (scale))
  #define GET_X(arr) (arr >> 4)
  #define GET_Y(arr) ((arr & B00001110) >> 1)
  #define GET_SCALE(arr) ((arr & B00000001))
  #define ARR_END B11111111
  #define microDelay 7000
  byte Scales[2] = {MAX_GREYSCALES -1, 10  };
  //byte Scales[2] = {10,MAX_GREYSCALES -1   };
  int compteur = 0;
  const byte WalkArray[] = { /*this defines the inner cube*/
                             MK_ARR ( 5, 6, 0),
                             MK_ARR ( 6, 6, 0),
                             MK_ARR ( 9, 6, 0),
                             MK_ARR (10, 6, 0),

                             /* this defines the outer cube */
                             /* here come the 4 corners*/
                           
                             MK_ARR (12, 4, 1),
     ARR_END,   
                             MK_ARR ( 3, 4, 1),
                             MK_ARR ( 0, 4, 1),
                             MK_ARR (15, 4, 1),

    
                             /*and now, all the bars*/
                             MK_ARR( 11, 5, 1 ),
                             MK_ARR(  4, 5, 1 ),
                             MK_ARR(  2, 5, 1 ),
                             MK_ARR(  1, 5, 1 ),

                             MK_ARR(  7, 5, 1 ),
                             MK_ARR(  8, 5, 1 ),
                             MK_ARR( 14, 5, 1 ),
                             MK_ARR( 13, 5, 1 ),

                             ARR_END
                           };



while (compteur < 700) {                           
  byte i = 0;
  SetLEDsByteLevel(  LEVEL_NONE );
  while (WalkArray[i] != ARR_END) {
    
    //SetLEDsByteLevel(  LEVEL_NONE );
    //delayMicroseconds(800);
    //delay(1);
    Tlc.clear();
    //Tlc.update();
    //delayMicroseconds(microDelay);
    Tlc.set(GET_X(WalkArray[i]), greyscales[Scales[GET_SCALE(WalkArray[i])]]);
    SetLEDsByteLevel(  dataArray[GET_Y(WalkArray[i])] );
    Tlc.update();
    
    //delayMicroseconds(microDelay);
    //digitalWrite(PB2, HIGH);
    
    delay(5);
    //digitalWrite(PB2, LOW);
    //delay(1);
    
    //delay(500);
    
    i++;

  }
  compteur ++;
}
SetLEDsByteLevel(  LEVEL_NONE );
delay(500);
}

void loop()
{
  byte i =0;
// KnightRiderEffect();
//FastAsAShark();
 
 Beating();
  BarresHorizontales(true);
 BarresHorizontales(false);
BarresVerticales();

 Opposites();
 Opposites();
 OppositesVerticaux(true);
 LoopOneAfterTheOther();
 
 //CubeAQuentin2(i);
 i = i == 0 ? 1 : 0;
}
                 
