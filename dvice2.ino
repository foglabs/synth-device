#include <synth.h>
//#include "notes.h"

#include <Adafruit_DotStar.h>
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

#define MONOMODE 0
#define POLYMODE 4
#define THERMODE 8

#define MAXNOTELENGTH 3000

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
unsigned long time_rgb;
unsigned long time_thr;
unsigned long time_gre;
unsigned long time_not;
unsigned long time_kno;

int modeblink = 0;

float thermal[AMG88xx_PIXEL_ARRAY_SIZE];

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

  pinMode( GREENPIN, OUTPUT );
  pinMode( REDPIN, OUTPUT );
  pinMode( BLUEPIN, OUTPUT );

  pinMode( KNOBPIN, INPUT );
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
  time_rgb = time;
  time_thr = time;
  time_gre = time;
  time_kno = time;

  for(int i=0; i<4;i++){
    voicetimes[i]=time;
    voicenotes[i]=-1;
    voicestaged[i]=-2;
  }
}

void loop() {

  time = millis();
  modechanged = checkMode();
  lightMode();

  // Serial.print("Mode is now ");
  // Serial.println(mode);
  // Serial.println(modechanged);

  // modeLights();
  // // SETUP
  // // voice, wave, pitchm, envelope, length, mod

  // setup my shit breh
  if(modechanged || first){

    if(!first){
      clearVoices();
    }
    
      modeblink = 0;

      if(mode == MONOMODE){
        soul.setWave(0,SINE);
        soul.setWave(1,SAW);
        soul.setWave(2,SINE);
        soul.setWave(3,SINE);
        soul.setLength(0,60);
        soul.setLength(1,60);
        soul.setLength(2,60);
        soul.setLength(3,60);  
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
        soul.setMod(0, 50);
      } else if(mode == 3){
        soul.setWave(0,SINE);
        soul.setWave(1,TRIANGLE);
        soul.setWave(2,SINE);
        soul.setWave(3,TRIANGLE);
        soul.setLength(0,82);
        soul.setLength(1,120);
        soul.setLength(2,45);
        soul.setLength(3,100);
      } else if(mode == POLYMODE){
        modeblink = 2000;

        soul.setWave(0,TRIANGLE);
        soul.setWave(1,SINE);
        soul.setWave(2,TRIANGLE);
        soul.setWave(3,SINE);

        soul.setEnvelope(0,ENVELOPE2);
        soul.setEnvelope(1,ENVELOPE2);
        soul.setEnvelope(2,ENVELOPE3);
        soul.setEnvelope(3,ENVELOPE1);

        soul.setLength(0,100);
        soul.setLength(1,80);
        soul.setLength(2,110);
        soul.setLength(3,70);
        Serial.println("Set Waves Poly");
      } else if(mode == 5){
        modeblink = 2000;

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
      } else if(mode == 6){
        modeblink = 2000;

        soul.setWave(0,SAW);
        soul.setWave(1,TRIANGLE);
        soul.setWave(2,SAW);
        soul.setWave(3,RAMP);

        soul.setEnvelope(0,ENVELOPE3);
        soul.setEnvelope(1,ENVELOPE2);
        soul.setEnvelope(2,ENVELOPE1);
        soul.setEnvelope(3,ENVELOPE1);

        soul.setLength(0,70);
        soul.setLength(1,110);
        soul.setLength(2,110);
        soul.setLength(3,130);
      } else if(mode == 7){
        modeblink = 2000;

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
      }
    // } else if(mode == THERMODE){

    modechanged = false;
    first = false;
  }

  // if( (time - time_gre) >= fadespeed ){
  //   if(change){
  //     fade+=1;
  //     if(fade>255){
  //       fade=0;
  //     }

  //     change = false;
  //   } else {
  //     change = true;
  //   }

  //   time_gre = time;
  // }

  // analogWrite(GREENPIN, fade);
  
  // Serial.println(analogRead(KNOBPIN));  
  
  // // do my mode boy
  switch(mode){
    case 0:
    case 1:
    case 2:
    case 3: monoMode();
            break;
    case 4:
    case 5:
    case 6:
    case 7: polyMode();
            break;
    // case 2: thermalMode();
    //         break;            
  }


  doKnob();
}

void doKnob(){

  // if( (time-time_kno)>=30 ){
  //   knob = analogRead(KNOBPIN);
  //   Serial.println(knob);
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

  if( (time - time_mod) >= 2000 ){
    // rgb button


    if(digitalRead(MODEPIN) == LOW){

      // increment mode...
      mode++;

      if(mode>9){
        mode = MONOMODE;
      }

      time_mod = time;
      return true;
    }
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
    if( (time-voicetimes[i]) >= 700 ){
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
  if(mode>=0 && mode < 4){
    // mono
    modeblink = 0;
  } else if(mode>=4 && mode < 8){
    // poly
    modeblink = 100;
  } else {
    // thermal
    modeblink = 3000;
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

  //   if(blink == true){
  //     blink = false;
  //     digitalWrite(GREENPIN, HIGH);
  //     digitalWrite(REDPIN, LOW);
  //     digitalWrite(BLUEPIN, HIGH);

  //   } else {
  //     blink=true;
  //     digitalWrite(GREENPIN, LOW);
  //     digitalWrite(REDPIN, HIGH);
  //     digitalWrite(BLUEPIN, HIGH);
  //   }
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
    // Serial.println(arcades[i]);
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

  if( (time - time_thr) >= 500){
    getThermal();
    // Serial.println(thermal[0]);
    time_thr = time;
  }
  // set modulation (thermal or whateva)

  // light lights
  if( (time-time_rgb) >= 1000 ){
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

  // get inputs
    // collect inputted notes in inputs array
    // 
    // collect inputted notes in inputs array
  for(uint8_t i=0; i<8; i++){
    arcades[i] = getArcade( arcadepins[i] );
    // Serial.println(arcades[i]);
  }

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
  if( (time - time_thr) >= 1000){
    getThermal();
    // Serial.println(thermal[0]);
    time_thr = time;
  }

  // light lights
  if( (time-time_rgb) >= 1000 ){
    lightRGB(tex);
    time_rgb = time;
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
uint8_t getArcade(uint8_t pin) {
  return digitalRead(pin);
}

int inputToNote(int offset) {
  // 16 possible 'octaves' w/ 8 each
  return (octave*8)+offset;  
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
    if( ( time - voicetimes[0] ) >= 700 ){

      // retrigger
      playMNote(new_note);
    }

  } else if( (top+bottom) == 1 && new_note >= 0) {
    // Serial.println("playing new note");
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
  
  // !!!might have to wait to retrigger held note if constant retrig sounds bad
    // should be unnecessary because note will play out on its own
    // killMNote();    

}

void playMNote(int note) {
  // reset timer
  // voicetimes[0] = time;
  // voicenotes[0] = note;

  for(int i=0; i<4; i++){
    // Serial.println("verrrr GOOD play "+note);
    soul.mTrigger(i,note);
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
        if( ( time - voicetimes[i] ) >= 3000 ){
          
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
  
  if( (time-time_not) >= 1200 ){
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

  float min = 25;
  float max = 40;
  float ratio = 2*(temp-min)/(max-min);
  uint8_t r = round( max(0, 255*(ratio-1)) );
  uint8_t b = round( max(0,255*(1-ratio)) );
  uint8_t g = round( 255 - b - r );

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

Serial.println("HEAR HERE ");
    Serial.println(rowbegin+incre);
    // invert lr because whoopsie!
    color = pixelToColor(thermal[ rowbegin+incre ]);
    incre-=1;

    rgbsquare.setPixelColor(q, color); // 'On' pixel at head
    // delay(20);                        // Pause 20 milliseconds (~50 FPS)
  }

  rgbsquare.show();                     // Refresh strip
}
  
