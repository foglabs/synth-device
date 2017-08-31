#include <synth.h>
//#include "notes.h"

#include <Adafruit_DotStar.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#include <Wire.h>
#include <Adafruit_AMG88xx.h>

// RGB
#define NUMPIXELS 64 // Number of LEDs in strip
#define DATAPIN    10
#define CLOCKPIN   9

#define ARCADE0 0
#define ARCADE1 1
#define ARCADE2 2
#define ARCADE3 4
#define ARCADE4 5
#define ARCADE5 6
#define ARCADE6 7
#define ARCADE7 8

#define MODEPIN 12

#define GREENPIN A3
#define REDPIN A1
#define BLUEPIN A0
#define KNOBPIN A2

#define QUADRANT1 1
#define QUADRANT2 0
#define QUADRANT3 3
#define QUADRANT4 2

#define MONOMODE 0
#define POLYMODE 8
#define THERMODE 15

#define MAXNOTELENGTH 3000

// sets of three chords         Cmaj       Emaj       Fmaj       Amin
uint8_t thermchords [4][12] = {
                                // {24,28,31,  28,32,35,  29,33,36,  21,24,28},
                                // // Bmaj     Dmaj       Emin       Bmin I guess
                                // {47,51,54,  50,54,57,  52,55,59,  47,50,54},
                                // //C# Eish   Eish       A fifths   Emaj
                                // {25,28,30,  28,33,36,  33,38,43,  28,30,28},

                                // {24,28,31,  28,32,35,  29,33,36,  21,24,28}
                                {36, 40, 43, 40, 44, 47, 41, 45, 48, 33, 36, 40},
                                {59, 63, 66, 62, 66, 69, 64, 67, 71, 59, 62, 66},
                                {37, 40, 42, 40, 45, 48, 45, 50, 55, 40, 42, 40},
                                {36, 40, 43, 40, 44, 47, 41, 45, 48, 33, 36, 40},
                              };

// current quad were playing from
uint8_t current_quad=0;
// index of  thermal note currently playing
uint8_t tplaying=0;

// RGB
Adafruit_DotStar rgbsquare = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

// Thermal
Adafruit_AMG88xx amg;

// ze synth
synth soul;

bool change = true;
int fade = 100;
unsigned long fadespeed = 2000;

//////////////////GLOBALS/////////////////
uint8_t arcades [8] {1,1,1,1,1,1,1,1};
uint8_t arcadepins [8] {ARCADE0,ARCADE1,ARCADE2,ARCADE3,ARCADE4,ARCADE5,ARCADE6,ARCADE7};
int gotinput = -1;

// poly voice timeouts
unsigned long voicetimes [4];
// to be played
int voicestaged [4];
// playing now
int voicenotes [4];

uint8_t octave=12;
int mode=MONOMODE;

unsigned long time;
unsigned long time_oct;
unsigned long time_mod;
unsigned long time_modhold;
unsigned long time_rgb;
unsigned long time_thr;
unsigned long time_gre;
unsigned long time_not;
unsigned long time_kno;
unsigned long time_tstart;
unsigned long time_tplay;

int modeblink = 500;

float thermal[AMG88xx_PIXEL_ARRAY_SIZE];

bool holdingmode = false;
bool modechanged = false;
bool first = true;
bool blink = true;

int knob = 0;
int lastknob = 0;

///////////////////////////////////////////

void setup() {
  // Serial.begin(9600);

  // 0,1,2,4,5,6,7,8 arcade pins (input)
  // 3,11 pwm pins
  // 9,10 data/clock for rgb
  // A0 fx knob
  // A4,A5 thermal pins
  
  // setup arcades
  pinMode( ARCADE0, INPUT_PULLUP );
  pinMode( ARCADE1, INPUT_PULLUP );
  pinMode( ARCADE2, INPUT_PULLUP );
  pinMode( ARCADE3, INPUT_PULLUP );
  pinMode( ARCADE4, INPUT_PULLUP );
  pinMode( ARCADE5, INPUT_PULLUP );
  pinMode( ARCADE6, INPUT_PULLUP );
  pinMode( ARCADE7, INPUT_PULLUP );

  // pinMode( GREENPIN, OUTPUT );
  // pinMode( REDPIN, OUTPUT );
  // pinMode( BLUEPIN, OUTPUT );

  // pinMode( KNOBPIN, INPUT );
  // mode

  // RGB object
  rgbsquare.begin(); // Initialize pins for output
  rgbsquare.show();  // Turn all LEDs off ASAP
  rgbsquare.setBrightness(10);  // lower max brightness

  // start synth instance
  soul.begin(DIFF);

  // begin thermal
  amg.begin();

      soul.setupVoice(0,SINE,60,ENVELOPE0,80,64);
      soul.setupVoice(1,SINE,60,ENVELOPE0,100,64);
      soul.setupVoice(2,SINE,60,ENVELOPE2,110,64);
      soul.setupVoice(3,SINE,60,ENVELOPE0,110,64);

  // initialize
  
  // main time
  time = millis();
  // timeouts
  time_oct = time;
  time_mod = time;
  time_modhold = time;
  time_rgb = time;
  time_thr = time;
  time_gre = time;
  time_kno = time;
  time_tstart = time;
  time_tplay = time;

  for(int i=0; i<4;i++){
    voicetimes[i]=time;
    voicenotes[i]=-1;
    voicestaged[i]=-2;
  }
}

void loop() {
  // Serial.println(mode);
  time = millis();

  getArcades();
  modechanged = checkMode();
  // lightMode();

  // Serial.print("Mode is now ");
  // Serial.println(mode);
  // Serial.println(modechanged);

  // // SETUP
  // // voice, wave, pitchm, envelope, length, mod

  // setup my shit breh
  if(modechanged || first){

    if(!first){
      clearVoices();
    }
    
    if(mode == MONOMODE){
      soul.setUpVoice(0,SINE,ENVELOPE0,60,64);
      soul.setUpVoice(1,SAW,ENVELOPE0,60,64);
      soul.setUpVoice(2,SINE,ENVELOPE0,60,64);
      soul.setUpVoice(3,SINE,ENVELOPE0,60,64);
    } else if(mode == 1){
      soul.setWave(0,SQUARE);
      soul.setWave(1,TRIANGLE);
      soul.setWave(2,RAMP);
      soul.setWave(3,TRIANGLE);
      soul.setLength(0,20);
      soul.setLength(1,40);
      soul.setLength(2,75);
      soul.setLength(3,60);
    } else if(mode == 2){
      soul.setWave(0,SAW);
      soul.setWave(1,TRIANGLE);
      soul.setWave(2,SAW);
      soul.setWave(3,SINE);
      soul.setLength(0,18);
      soul.setLength(1,18);
      soul.setLength(2,18);
      soul.setLength(3,18);
    } else if(mode == 3){
      soul.setWave(0,SINE);
      soul.setWave(1,TRIANGLE);
      soul.setWave(2,SINE);
      soul.setWave(3,TRIANGLE);
      soul.setLength(0,82);
      soul.setLength(1,80);
      soul.setLength(2,45);
      soul.setLength(3,70);
    } else if(mode == 4){
      soul.setWave(0,SINE);
      soul.setWave(1,TRIANGLE);
      soul.setWave(2,SINE);
      soul.setWave(3,SAW);
      soul.setLength(0,42);
      soul.setLength(1,82);
      soul.setLength(2,45);
      soul.setLength(3,80);


      soul.setEnvelope(0,ENVELOPE0);
      // soul.setEnvelope(1,ENVELOPE2);
      soul.setEnvelope(2,ENVELOPE0);
      soul.setEnvelope(3,ENVELOPE1);
    } else if(mode == 5){
      soul.setWave(0,SQUARE);
      soul.setWave(1,SAW);
      soul.setWave(2,SINE);
      soul.setWave(3,SAW);
      soul.setLength(0,22);
      soul.setLength(1,40);
      soul.setLength(2,45);
      soul.setLength(3,90);

      soul.setEnvelope(0,ENVELOPE3);
      soul.setEnvelope(1,ENVELOPE3);
      soul.setEnvelope(2,ENVELOPE3);
      soul.setEnvelope(3,ENVELOPE0);
    } else if(mode == 6){
      soul.setWave(0,SAW);
      soul.setWave(1,SAW);
      soul.setWave(2,SAW);
      soul.setWave(3,SAW);
      soul.setLength(0,24);
      soul.setLength(1,48);
      soul.setLength(2,64);
      soul.setLength(3,96);

      soul.setEnvelope(0,ENVELOPE1);
      soul.setEnvelope(1,ENVELOPE1);
      soul.setEnvelope(2,ENVELOPE1);
      soul.setEnvelope(3,ENVELOPE1);
    } else if(mode == 7){
      soul.setWave(0,SQUARE);
      soul.setWave(1,SQUARE);
      soul.setWave(2,SINE);
      soul.setWave(3,TRIANGLE);
      soul.setLength(0,52);
      soul.setLength(1,50);
      soul.setLength(2,95);
      soul.setLength(3,80);


    } else if(mode == POLYMODE){

      soul.setWave(0,TRIANGLE);
      soul.setWave(1,SINE);
      soul.setWave(2,TRIANGLE);
      soul.setWave(3,SINE);

      soul.setEnvelope(0,ENVELOPE2);
      soul.setEnvelope(1,ENVELOPE2);
      soul.setEnvelope(2,ENVELOPE3);
      soul.setEnvelope(3,ENVELOPE1);

      soul.setLength(0,70);
      soul.setLength(1,80);
      soul.setLength(2,65);
      soul.setLength(3,70);
      Serial.println("Set Waves Poly");
    } else if(mode == 9){

      soul.setWave(0,SQUARE);
      soul.setWave(1,SQUARE);
      soul.setWave(2,SQUARE);
      soul.setWave(3,SINE);

      soul.setEnvelope(0,ENVELOPE0);
      soul.setEnvelope(1,ENVELOPE0);
      soul.setEnvelope(2,ENVELOPE0);
      soul.setEnvelope(3,ENVELOPE0);

      soul.setLength(0,30);
      soul.setLength(1,40);
      soul.setLength(2,50);
      soul.setLength(3,10);
    } else if(mode == 10){

      soul.setWave(0,SAW);
      soul.setWave(1,TRIANGLE);
      soul.setWave(2,SAW);
      soul.setWave(3,RAMP);

      soul.setEnvelope(0,ENVELOPE3);
      soul.setEnvelope(1,ENVELOPE2);
      soul.setEnvelope(2,ENVELOPE1);
      soul.setEnvelope(3,ENVELOPE1);

      soul.setLength(0,70);
      soul.setLength(1,60);
      soul.setLength(2,60);
      soul.setLength(3,85);
    } else if(mode == 11){

      soul.setWave(0,RAMP);
      soul.setWave(1,RAMP);
      soul.setWave(2,SINE);
      soul.setWave(3,SINE);

      soul.setEnvelope(0,ENVELOPE3);
      soul.setEnvelope(1,ENVELOPE2);
      soul.setEnvelope(2,ENVELOPE3);
      soul.setEnvelope(3,ENVELOPE0);

      soul.setLength(0,35);
      soul.setLength(1,23);
      soul.setLength(2,67);
      soul.setLength(3,70);
    } else if(mode == 12){

      soul.setWave(0,SINE);
      soul.setWave(1,SINE);
      soul.setWave(2,SINE);
      soul.setWave(3,SINE);

      soul.setEnvelope(0,ENVELOPE0);
      soul.setEnvelope(1,ENVELOPE0);
      soul.setEnvelope(2,ENVELOPE0);
      soul.setEnvelope(3,ENVELOPE0);

      soul.setLength(0,68);
      soul.setLength(1,72);
      soul.setLength(2,64);
      soul.setLength(3,76);
    } else if(mode == 13){

      soul.setWave(0,SQUARE);
      soul.setWave(1,SAW);
      soul.setWave(2,TRIANGLE);
      soul.setWave(3,SAW);

      soul.setEnvelope(0,ENVELOPE3);
      soul.setEnvelope(1,ENVELOPE3);
      soul.setEnvelope(2,ENVELOPE3);
      soul.setEnvelope(3,ENVELOPE2);

      soul.setLength(0,80);
      soul.setLength(1,20);
      soul.setLength(2,80);
      soul.setLength(3,80);
    } else if(mode == 14){

      soul.setWave(0,NOISE);
      soul.setWave(1,NOISE);
      soul.setWave(2,SINE);
      soul.setWave(3,SINE);

      soul.setEnvelope(0,ENVELOPE3);
      soul.setEnvelope(1,ENVELOPE3);
      soul.setEnvelope(2,ENVELOPE0);
      soul.setEnvelope(3,ENVELOPE0);

      soul.setLength(0,85);
      soul.setLength(1,23);
      soul.setLength(2,37);
      soul.setLength(3,70);

    } else if(mode == THERMODE){

      soul.setWave(0,SINE);
      soul.setWave(1,SINE);
      soul.setWave(2,SINE);
      soul.setWave(3,SINE);

      soul.setEnvelope(0,ENVELOPE0);
      soul.setEnvelope(1,ENVELOPE2);
      soul.setEnvelope(2,ENVELOPE0);
      soul.setEnvelope(3,ENVELOPE0);

      soul.setLength(0,95);
      soul.setLength(1,93);
      soul.setLength(2,83);
      soul.setLength(3,90);
    }

    modechanged = false;
    first = false;
  }

  // if( (time - time_gre) >= modeblink ){

  //   if(blink){
  //     digitalWrite(GREENPIN, HIGH);
  //   } else {
  //     digitalWrite(GREENPIN, LOW);
  //   }
  //   blink = !blink;


  //   time_gre = time;
  // }

  // Serial.println(analogRead(KNOBPIN));  
  
  // // do my mode boy
  switch(mode){
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7: monoMode();
            break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14: polyMode();
            break;
    case 15: thermalMode();
            break;            
  }


  // doKnob();
}

void doKnob(){

  // if( (time-time_kno)>=30 ){
    // knob = analogRead(KNOBPIN);
    // Serial.println(knob);
  //   time_kno = time;

  //   if(abs(lastknob-knob)>= 40){
  //     int length;

  //     length = 0 + ((128 - 0) / (1023 - 0)) * (knob - 0);

  //     for(uint8_t i=0; i<4; i++){
  //       soul.setLength(i, length);
  //     }
  //   }

  //   lastknob = knob;

  // }

}

////////////////// CHECKS  /////////////////
bool checkMode(){

  if( (time - time_mod) >= 800 ){

    if(digitalRead(MODEPIN) == LOW){

      // if(holdingmode == false){
      //   holdingmode = true;
      //   time_modhold = time;
      // }

      // if( holdingmode && (time-time_modhold) >= 800 ){

        mode++;
      //   holdingmode = false;
      // }

      // if(gotinput>0){
      //   mode = gotinput;
      // } else {
        // increment mode...

      // }

      // gotinput=-1;

      if(mode>15){
        mode = MONOMODE;
      }

      return true;
    }

    time_mod = time;

  }

  return false;
}



void clearVoices(){
  for(uint8_t i=0; i<4; i++){
    voicetimes[i] = time;
    voicenotes[i] = -1;
    soul.setLength(i,0);
  }
}

void cleanUpNotes(){
  for(uint8_t i=0; i<4; i++){

    // must be finished by now?
    if( (time-voicetimes[i]) >= 2000 ){
      voicetimes[i] = time;
      voicenotes[i] = -1;  
    }

    // unstage
    voicestaged[i] = -2;
  } 
}

uint8_t getAvailVoice(){
  uint8_t i;

  for(i=0; i<4; i++){
    // no note playing on this voice?
    if(voicenotes[i] == -1){
      // Serial.print("AH yeas found ");
      // Serial.println(i);
      return i;
    }
  }

  for(i=0; i<4; i++){
    // ready to be interupted?
    if( (time - voicetimes[i]) >= 700 ){
      // Serial.print("vocietimes goes ");
      // Serial.println(i);
      // Serial.println(voicetimes[i]);
      // Serial.println(time);
      return i;
    }
  }

  return 5;
}

void lightMode(){
  if(mode>=0 && mode < 8){
    // mono
    modeblink = 500;
  } else if(mode>=8 && mode < 15){
    // poly
    modeblink = 1000;
  } else {
    // thermal
    modeblink = 2000;
  }

  if((time-time_gre) >= modeblink){
    time_gre = time;

    switch(mode){
      case 0: digitalWrite(REDPIN, blink);
              digitalWrite(GREENPIN, !blink);
              digitalWrite(BLUEPIN, !blink);
              break;
      case 1: digitalWrite(REDPIN, !blink);
              digitalWrite(GREENPIN, blink);
              digitalWrite(BLUEPIN, !blink);
              break;
      case 2: digitalWrite(REDPIN, !blink);
              digitalWrite(GREENPIN, !blink);
              digitalWrite(BLUEPIN, blink);
              break;
      case 3: digitalWrite(REDPIN, blink);
              digitalWrite(GREENPIN, blink);
              digitalWrite(BLUEPIN, !blink);
              break;
              // POLY
      case 4: digitalWrite(REDPIN, blink);
              digitalWrite(GREENPIN, blink);
              digitalWrite(BLUEPIN, !blink);
              break;
      case 5: digitalWrite(REDPIN, !blink);
              digitalWrite(GREENPIN, blink);
              digitalWrite(BLUEPIN, !blink);
              break;
      case 6: digitalWrite(REDPIN, !blink);
              digitalWrite(GREENPIN, blink);
              digitalWrite(BLUEPIN, blink);
              break;
      case 7: digitalWrite(REDPIN, blink);
              digitalWrite(GREENPIN, blink);
              digitalWrite(BLUEPIN, !blink);
              break;
              // THERMAL?
      case 8: digitalWrite(REDPIN, blink);
              digitalWrite(GREENPIN, blink);
              digitalWrite(BLUEPIN, blink);
              break;
              // CHORD
      case 9: digitalWrite(REDPIN, blink);
              digitalWrite(GREENPIN, !blink);
              digitalWrite(BLUEPIN, blink);
              break;

    }

    blink = !blink;
  }
}

void getArcades(){
  // collect inputted notes in inputs array
  for(uint8_t i=0; i<8; i++){
    arcades[i] = getArcade( arcadepins[i] );
    // Serial.println(arcades[i]);
  }
}

////////// MODES //////////////
void monoMode() {
  char tex [3];
////// [8] inputs, int note_playing

  // get inputs
    // get octave from knob

  handleMNotes();
  // kill note
    //  if no inputs
      // set note length to 0
      // zero out note_playing var 
  // play note
    // every 10 ms, check for note change
      // if inputs include note_playing, re-trigger note_playing
      // if not, trigger new first note (if multi, choose random?)

  if( (time - time_thr) >= 200){
    getThermal();
    // Serial.println(thermal[0]);
    time_thr = time;
  }
  // set modulation (thermal or whateva)

  // light lights
  if( (time-time_rgb) >= 200 ){
    lightRGB(tex);
    time_rgb = time;
  }

  // cleanup?
}


//+++/ option to randomly switch waveform?
  // setupVoice on that number whenever needed
void polyMode() {
////// [8] inputs, [4] notes_playing
  // noteplaying -> [nil,nil,nil,128] if truthy, it's a note that is playing, otherwise voice is available

  char tex [3];

////// SETUP
  // setup 4 voices w/ length 64 or something

  // kill notes
    // for each note in note_playing
      // if inputs has this note
        // retrigger dat note
      // else
        // set dat length 0
        // find note's voice and make voice available (nil)

  // play notes
  handlePNotes();
    // trigger first 4 notes of input

  // set modulation (thermal or whateva)
  if( (time - time_thr) >= 200){
    getThermal();
    // Serial.println(thermal[0]);
    time_thr = time;
  }

  // light lights
  if( (time-time_rgb) >= 200 ){
    lightRGB(tex);
    time_rgb = time;
  }
}

void thermalMode() {
  char tex [3];
  // wtf bruh!
  uint8_t avg_global;
  uint8_t avg_quad [4];
  uint8_t quadindex = 0;
  uint8_t quadmax = 0;
  uint8_t i = 0;


  if( (time - time_thr) >= 60){
    getThermal();
    // Serial.println(thermal[0]);
    time_thr = time;
  }

  // distribute thermal values over quadrants
  for(i; i<NUMPIXELS; i++){
    avg_global += thermal[i];
    
    if(i<32){
      quadindex = 0;
    } else {
      quadindex = 2;
    }

    if( (i%8)>3 ){
      quadindex += 1;
    }

    avg_quad[quadindex] += thermal[i];
  }

  // calc averages
  avg_global = avg_global/64;

  // interupt
  if( (time-time_tstart) >= 3000 ){
  
    avg_quad[0] = avg_quad[0]/16;
    avg_quad[1] = avg_quad[1]/16;
    avg_quad[2] = avg_quad[2]/16;
    avg_quad[3] = avg_quad[3]/16;

    // pick hot quadrant
    for(i=0; i<4; i++){
      if( avg_quad[i] > quadmax ){

        // get highest average
        quadmax = avg_quad[i];
        // set to highest average quadrant
        current_quad = i;
      }
    }

    time_tstart = time;
  }

  // play da notes
  if( current_quad<5 && (time-time_tplay) >= 2000 ){

    playChord(current_quad, tplaying);
    tplaying += 3;

    time_tplay = time;
  }

  // played last chord, start over!
  if(tplaying>9){
    tplaying=0;
    // current_quad=5;
  }

  handleTNotes();

  // light lights
  if( (time-time_rgb) >= 60 ){
    lightRGB(tex);
    time_rgb = time;
  }

  // if quadrant changed or nothing playing
    // start new quadrant

  // else
    // continue playing quadrant
    // check note time...
    // play if ready

  // play relevant flourish notes



}

void handleTNotes(){
  int new_note = -1;
  uint8_t numheld = 0;

  for(uint8_t i=0; i<8; i++){

    // low is on baby
    if(arcades[i] == LOW){
      new_note = inputToNote(i);
      // numheld

      if(voicenotes[0]==new_note){
        // Serial.println("got keepplaying");

        if( ( time - voicetimes[0] ) >= 1600 ){
          // retrigger
          playTNote(new_note);
          voicetimes[0] = time;
        }

      } else if( new_note >= 0) { // numheld == 1 &&
        // Serial.println("playing new note");
        voicenotes[0] = new_note;
        playTNote(new_note);
     
      } else if( voicetimes[0] > 0 && ( time - voicetimes[0] ) >= 6000) {
        // Serial.println("kill note tiemrs");
        voicetimes[0] = 0;
        voicenotes[0] = -1;
        // digitalWrite(13, LOW);
        // killMNote();
      }

    }
  }

}

void playTNote(int note){
  soul.mTrigger(4,note);
}

//////////// INPUT RELATED FUNCTIONS /////////
uint8_t getArcade(uint8_t pin) {
  // return digitalRead(pin);
  uint8_t inp = digitalRead(pin);
  
  // grab for mode change
  // if(inp == LOW){
  //   gotinput = apinToNum(pin);
  // }

  return inp;
}

int inputToNote(int offset) {
  // 16 possible 'octaves' w/ 8 each
  return (octave*8)+offset;  
}

uint8_t apinToNum(uint8_t pin){
  switch(pin){
    case ARCADE0: return 0;
    case ARCADE1: return 1;
    case ARCADE2: return 2;
    case ARCADE3: return 3;
    case ARCADE4: return 4;
    case ARCADE5: return 5;
    case ARCADE6: return 6;
    case ARCADE7: return 7;
  }
}



void getThermal(){
  amg.readPixels(thermal);
}

//////////// PLAY/Kill NOTES /////////
void handleMNotes() {
  bool keepplaying = false;
  int new_note = -1;
  int noteindex = 0;

  uint8_t top = 0;
  uint8_t bottom = 0;

  // only play if one arcades
  for(uint8_t i=0; i<8; i++){

    // low is on baby
    if(arcades[i] == LOW){
      new_note = inputToNote(i);
      // Serial.println("note "+new_note);
      // digitalWrite(13, HIGH);

      // count for octave change
      // top or bottom row
      if(i<4){
        top++;
      } else {
        bottom++;
      }

      // Serial.print("got input");
      // Serial.println(i);
    }
  }

  // Serial.println(voicetimes[0]);
  // Serial.println(time);

  if(voicenotes[0]==new_note){
    // Serial.println("got keepplaying");
    if( ( time - voicetimes[0] ) >= 1600 ){

      // retrigger
      playMNote(new_note);
    }

  } else if( (top+bottom) == 1 && new_note >= 0) {

    // is dis wrong??
    // voicenotes[0] = new_note;

    playMNote(new_note);
 
  } else if( voicetimes[0] > 0 && ( time - voicetimes[0] ) >= 6000) {
    // Serial.println("kill note tiemrs");
    voicetimes[0] = 0;
    voicenotes[0] = -1;
    // digitalWrite(13, LOW);
    // killMNote();
  }

  // change octave
  if( (time - time_oct) >= 1000){

    if( top == 4 ){
      // if up (0-3)
      if(octave<16){
        octave += 1;
        time_oct = time;
      }

    } else if(bottom == 4 ) {
      // if down (4-7)
      if(octave>0){
        octave -= 1;
        time_oct = time;
      }
    }
    
  }
  
    
  if( (time-time_not) >= 1600 ){
    cleanUpNotes();
  }
  // !!!might have to wait to retrigger held note if constant retrig sounds bad
    // should be unnecessary because note will play out on its own
    // killMNote();    

}

void playMNote(int note) {
  // reset timer
  // voicetimes[0] = time;
  // voicenotes[0] = note;

  for(int i=0; i<4; i++){
    Serial.println("verrrr GOOD play "+note);
    soul.mTrigger(i,note);
  }
}

void playChord(uint8_t set, uint8_t chordstart) {
  for(uint8_t i=0; i<3; i++){
    // Serial.println("verrrr GOOD play "+note);

    // play each tone of note
    soul.mTrigger(i, thermchords[ set ][ chordstart+i ]);
  }
}

void killMNote() {
  for(int i=0; i<4; i++){
    soul.setLength(i,0);
  } 
}

void handlePNotes() {
  bool wasplaying = false;

  uint8_t top = 0;
  uint8_t bottom = 0;
  uint8_t numstaged = 0;

  // go through arcades
  for(uint8_t q=0; q<8; q++){

    // if button down - if no longer down, voice will play out its length until replaced
    if( arcades[q] == LOW ){

      if(numstaged < 4){


        if(q!= 1){
        voicestaged[numstaged] = inputToNote(q);
        numstaged++;
          
        }
        
        // Serial.print("New note ");
        // Serial.println(voicestaged[q]);
      }

      // count for oct change
      if(q<4){
        top++;
      } else {
        bottom++;
      }

    }
  }

  // change octave? -> exit befor eplaying notes
  if( (time - time_oct) >= 1000){

    if( top == 4 ){
      // if up (0-3)
      if(octave<16){
        octave += 1;
        time_oct = time;
      }

      return;
    } else if(bottom == 4 ) {
      // if down (4-7)
      if(octave>0){
        octave -= 1;
        time_oct = time;
      }

      return; 
    }
  }

  // Serial.print("NUM STAGED: ");
  // Serial.println(numstaged);

  // loop through staged notes
  for(uint8_t w=0; w<numstaged; w++){
    wasplaying = false;

    // loop through voicenotes
    for(uint8_t i=0; i<4; i++){

      // already playing?
      if(voicestaged[w] == voicenotes[i]){

        // check timer for this voice (dont disturb voice until 2000 ms)
        if( ( time - voicetimes[i] ) >= 1600 ){
          
          // retrig
          playPNote(voicestaged[w], i);
        }

        // go to next staged w
        wasplaying = true;
        break;
      }
    }

    // Serial.println(voicenotes[w]);

    if(wasplaying){
      // Serial.println("SKIP, was playing");
      // dont try to play same note on a free voice, skip to next w
      continue;
    }

    uint8_t voice = getAvailVoice();
    // Serial.println("DIDNt SKIP");
    // availvoice was found?
    if(voice < 5){
      playPNote(voicestaged[w], voice);
    }
  }
  
  if( (time-time_not) >= 1600 ){
    cleanUpNotes();
  }
}

void playPNote(int note, int voice) {
  // Serial.print("Play P NOTE");
  // Serial.println(note);
  // Serial.println(voice);
  voicenotes[voice] = note;
  voicetimes[voice] = time;
  soul.mTrigger(voice,note);
}

//////////// LIGHTS /////////
uint32_t pixelToColor(float temp){ 
  // 25-40 in a hot ass room

// def rgb(minimum, maximum, value):
//     minimum, maximum = float(minimum), float(maximum)
//     ratio = 2 * (value-minimum) / (maximum - minimum)
//     b = int(max(0, 255*(1 - ratio)))
//     r = int(max(0, 255*(ratio - 1)))
//     g = 255 - b - r
//     return r, g, b

  float min = 20;
  float max = 36;
  float ratio = 2*(temp-min)/(max-min);
  // uint8_t r = round( max(0, 255*(ratio-1)) );
  // uint8_t b = round( max(0,255*(1-ratio)) );
  // uint8_t g = round( 255 - b - r );

  uint8_t g = round( max(0, 255*(ratio-1)) );
  uint8_t b = round( max(0,255*(1-ratio)) );
  uint8_t r = round( 255 - b - r );

  // Serial.print("r");
  // Serial.println(r);
  // Serial.print("g");
  // Serial.println(g);
  // Serial.print("b");
  // Serial.println(b);    
  return r << 16 | b | g << 8;
}

void lightRGB(char * text){
  uint32_t color = 0;
  int incre = 7;
  int rowbegin = 0;

  // thermal
  for(int q=0; q<64; q++){
    // get next r,g,b

    if((rowbegin+incre)==rowbegin){
      incre=7;
      rowbegin+=8;
    }

    // Serial.println("HEAR HERE ");
    // Serial.println(rowbegin+incre);

    // invert lr because whoopsie!
    color = pixelToColor(thermal[ rowbegin+incre ]);
    incre-=1;

    rgbsquare.setPixelColor(q, color); // 'On' pixel at head
    // delay(20);                        // Pause 20 milliseconds (~50 FPS)
  }

  rgbsquare.show();                     // Refresh strip
}
  
