#include <synth.h>
//#include "notes.h"

#include <Adafruit_DotStar.h>
#include <SPI.h>

#include <Wire.h>
#include <Adafruit_AMG88xx.h>

// RGB
#define NUMPIXELS 64 // Number of LEDs in strip
#define DATAPIN    9
#define CLOCKPIN   10

#define ARCADE0 0
#define ARCADE1 1
#define ARCADE2 2
#define ARCADE3 4
#define ARCADE4 5
#define ARCADE5 6
#define ARCADE6 7
#define ARCADE7 8

#define MODEPIN 12

#define MONOMODE 0
#define POLYMODE 1
#define THERMODE 2

// RGB
Adafruit_DotStar rgbsquare = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

// Thermal
Adafruit_AMG88xx amg;

// ze synth
synth soul;

//////////////////GLOBALS/////////////////
int arcades [8] {-1,-1,-1,-1,-1,-1,-1,-1};
int arcadepins [8] {ARCADE0,ARCADE1,ARCADE2,ARCADE3,ARCADE4,ARCADE5,ARCADE6,ARCADE7};

// poly voice timeouts
unsigned long voicetimes [4];
int voicenotes [4];

uint8_t octave=12;
uint8_t mode=MONOMODE;

unsigned long time;
unsigned long time_oct;
unsigned long time_mod;
unsigned long time_rgb;

float thermal[AMG88xx_PIXEL_ARRAY_SIZE];

bool modechanged = false;

///////////////////////////////////////////

void setup() {
  Serial.begin(9600);

  // 0,1,2,4,5,6,7,8 arcade pins (input)
  // 3,11 pwm pins
  // 9,10 data/clock for rgb
  // A0 fx knob
  // A4,A5 thermal pins
  
  // setup arcades
  pinMode(13, OUTPUT);
  pinMode( ARCADE0, INPUT_PULLUP );
  pinMode( ARCADE1, INPUT_PULLUP );
  pinMode( ARCADE2, INPUT_PULLUP );
  pinMode( ARCADE3, INPUT_PULLUP );
  pinMode( ARCADE4, INPUT_PULLUP );
  pinMode( ARCADE5, INPUT_PULLUP );
  pinMode( ARCADE6, INPUT_PULLUP );
  pinMode( ARCADE7, INPUT_PULLUP );

  // mode
  pinMode( MODEPIN, INPUT_PULLUP );

  // RGB object
  rgbsquare.begin(); // Initialize pins for output
  rgbsquare.show();  // Turn all LEDs off ASAP
  rgbsquare.setBrightness(100);  // lower max brightness

  // start synth instance
  soul.begin(DIFF);

  // begin thermal
  // amg.begin();

      soul.setupVoice(0,SINE,60,ENVELOPE0,80,64);
      soul.setupVoice(1,SINE,60,ENVELOPE0,100,64);
      soul.setupVoice(2,SINE,60,ENVELOPE2,110,64);
      soul.setupVoice(3,SINE,60,ENVELOPE0,110,64);


  // initialize
  for(int i=0; i<4;i++){
    voicetimes[i]=millis();
    voicenotes[i]=-2;
  }
 
  // main time
  time = millis();
  // timeouts
  time_oct = time;

}


void loop() {

  time = millis();
  monoMode();
  // modechanged = checkMode();

  // // SETUP
  // // voice, wave, pitchm, envelope, length, mod

  // // setup my shit breh
  // if(modechanged){
  //   if(mode == MONOMODE){

  //     soul.setupVoice(0,SINE,60,ENVELOPE0,80,64);
  //     soul.setupVoice(1,SINE,60,ENVELOPE0,100,64);
  //     soul.setupVoice(2,SINE,60,ENVELOPE2,110,64);
  //     soul.setupVoice(3,SINE,60,ENVELOPE0,110,64);
  //   } else if(mode == POLYMODE){

  //     soul.setupVoice(0,SINE,60,ENVELOPE0,80,64);
  //     soul.setupVoice(1,SAW,60,ENVELOPE0,80,64);
  //     soul.setupVoice(2,TRIANGLE,60,ENVELOPE2,80,64);
  //     soul.setupVoice(3,SINE,60,ENVELOPE0,80,64);
  //   } else {

  //   }

  // }

  // // do my mode boy
  // switch(mode){
  //   case 0: monoMode();
  //           break;
  //   case 1: polyMode();
  //           break;
  //   case 2: thermalMode();
  //           break;            
  // }

}

//////////////////MODE CHECKS/////////////////
bool checkMode(){
  if( (time - time_mod) >= 1000 ){
    // rgb button
    if(digitalRead(MODEPIN) == LOW){
      // increment mode...
      mode++;

      if(mode>2){
        mode = 0;
      }

      time_mod = time;
      return true;
    }
  } else {
    return false;
  }
}

void modeLights(){
  switch(mode){
    case 0: //something;
            break;
    case 1: break;
    
    case 2: break;
  }
}


////////// MODES //////////////
void monoMode() {
  char tex [3];
////// [8] inputs, int note_playing

  // get inputs
    // get octave from knob

    // collect inputted notes in inputs array
  for(uint8_t i=0; i<8; i++){
    arcades[i] = getArcade( arcadepins[i] );
  }

  handleMNotes();
  // kill note
    //  if no inputs
      // set note length to 0
      // zero out note_playing var 
  // play note
    // every 10 ms, check for note change
      // if inputs include note_playing, re-trigger note_playing
      // if not, trigger new first note (if multi, choose random?)

  // getThermal();
  // set modulation (thermal or whateva)

  // light lights
  if( (time-time_rgb) >= 500 ){
    lightRGB(tex);
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

  // get inputs
    // collect inputted notes in inputs array
    // 

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
  getThermal();

  // light lights
  if( (time-time_rgb) >= 500 ){
    lightRGB(tex);
  }
}

void thermalMode() {
  // wtf!
  getThermal();
  uint8_t avg_global;
  uint8_t avg_quad [4];
  uint8_t quadindex = 0;
  uint8_t quadmax = 0;
  uint8_t i = 0;

  // distribute thermal values
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
  avg_quad[0] = avg_quad[0]/16;
  avg_quad[1] = avg_quad[1]/16;
  avg_quad[2] = avg_quad[2]/16;
  avg_quad[3] = avg_quad[3]/16;

  // pick hot quadrant
  for(i=0; i<4; i++){
    if( avg_quad[i] > quadmax ){
      quadmax = avg_quad[i];
    }
  }

  // if quadrant changed or nothing playing
    // start new quadrant

  // else
    // continue playing quadrant
    // check note time...
    // play if ready

  // play relevant flourish notes



}

//////////// INPUT RELATED FUNCTIONS /////////
int getArcade(int pin) {
  return digitalRead(pin);
}

int inputToNote(int oct, int offset) {
  // 16 possible 'octaves' w/ 8 each
  return (oct*8)+offset;
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
  for(int i=0; i<8; i++){

    // low is on baby
    if(arcades[i] == LOW){
      new_note = inputToNote(octave,i);
      digitalWrite(13, HIGH);

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
    Serial.println("got keepplaying");
    if( ( time - voicetimes[0] ) >= 6000){
        // set to new time for new note
      voicetimes[0] = time;
      // retrigger
      playMNote(new_note);
    }

  } else if( (top+bottom) == 1 && new_note >= 0) {
    Serial.println("playing new note");
    playMNote(new_note);
    voicenotes[0] = new_note;
    voicetimes[0] = time;
 
  } else if( voicetimes[0] > 0 && ( time - voicetimes[0] ) >= 6000) {
    Serial.println("kill note tiemrs");
    voicetimes[0] = -1;
    voicenotes[0] = -2;
    digitalWrite(13, LOW);
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
  
  // !!!might have to wait to retrigger held note if constant retrig sounds bad
    // should be unnecessary because note will play out on its own
    // killMNote();    

}

void playMNote(int note) {
  for(int i=0; i<4; i++){
    Serial.println("verrrr GOOD");
    soul.mTrigger(i,note);
  }
}

void killMNote() {
  for(int i=0; i<4; i++){
    soul.setLength(i,0);
  } 
}

void handlePNotes() {
  bool keepplaying = false;
  int this_note = -1;
  int noteindex = 0;
  unsigned long time = millis();

  uint8_t top = 0;
  uint8_t bottom = 0;

  // go through arcades
  for(int q=0; q<8; q++){

    // if button down - if no longer down, voice will play out its length until replaced
    if( arcades[q] == LOW ){
      Serial.println("Pressed "+q);
      this_note = inputToNote(octave,q);

      // count for oct change
      if(q<4){
        top++;
      } else {
        bottom++;
      }      

      // are we already playing this note?
      for(int i=0; i<4; i++){

        if(voicenotes[i]==this_note){
          keepplaying = true;
          noteindex = i;
        }
      }

      if(keepplaying){
        
        // wait for retrigger if held
        if( ( time - voicetimes[noteindex] ) >= 6000){
            // set to new time for new note
          voicetimes[noteindex] = time;
          // retrigger
          playPNote(this_note, noteindex);
        }

      } else {
        // choose random voice to overwrite
        noteindex = rand()*3;
        voicetimes[noteindex] = time;
        voicenotes[noteindex] = this_note;
        playPNote(this_note, noteindex);
      }
    }

    // reset for next input check
    keepplaying = false;
  }

  // clean up note time for (no longer held) note
  // note will play out on its own
  for(int i=0; i<4; i++){
    if( voicetimes[i]>0 && ( time - voicetimes[i] ) >= 6000){
      voicenotes[i] = -1;
      voicetimes[i] = -1;
    }
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
}

void playPNote(int note, int voice) {
  soul.mTrigger(voice,note);
}

//////////// LIGHTS /////////
uint8_t pixelToColor(uint8_t temp){
  // 25-40 in a hot ass room

  const int NUM_COLORS = 4;
  static float color[NUM_COLORS][3] = { {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0} };
  // temp - min / max - min
  int tempmag = 1 - ( (temp - 25)/(40-25) );
  int idx1;        // |-- Our desired color will be between these two indexes in "color".
  int idx2;        // |
  float fractBetween = 0; 

  if(tempmag <= 0) {
    idx1 = idx2 = 0;
  } else if(tempmag >= 1) {
    idx1 = idx2 = NUM_COLORS-1;    // accounts for an input >=0
  } else {
    tempmag = tempmag * (NUM_COLORS-1);        // Will multiply tempmag by 3.
    idx1  = floor(tempmag);                  // Our desired color will be after this index.
    idx2  = idx1+1;                        // ... and before this index (inclusive).
    fractBetween = tempmag - float(idx1);    // Distance between the two indexes (0-1).
  }

  uint8_t r = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
  uint8_t g = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
  uint8_t b  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
  return r << 16 | b | g << 8;
}

void lightRGB(char * text){
  uint8_t color = 0;

  if(thermal){
  // thermal
    for(int q=0; q<64; q++){
      // get next r,g,b
      color = pixelToColor(thermal[q]);
      rgbsquare.setPixelColor(q, color); // 'On' pixel at head
      // rgbsquare.setPixelColor(tail, 0);     // 'Off' pixel at tail
      rgbsquare.show();                     // Refresh strip
      // delay(20);                        // Pause 20 milliseconds (~50 FPS)

      // if(++head >= NUMPIXELS) {         // Increment head index.  Off end of strip?
      //   head = 0;                       //  Yes, reset head index to start
      //   if((color >>= 8) == 0)          //  Next color (R->G->B) ... past blue now?
      //     color = 0xFF0000;             //   Yes, reset to red
      // }
      // if(++tail >= NUMPIXELS) tail = 0; // Increment, reset tail index  
    }

  } else {
    // draw text!!
  }
  
}
  
