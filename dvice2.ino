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

#define BLUEPIN A1
#define KNOBPIN A2

#define MONOMODE 0
#define POLYMODE 1
#define THERMODE 2

// RGB

Adafruit_DotStar rgbsquare = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

// Thermal
Adafruit_AMG88xx amg;

// ze synth
synth soul;


bool change = true;
int fade = 0;
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
unsigned long time_mbt;

float thermal[AMG88xx_PIXEL_ARRAY_SIZE];

bool modechanged = false;
bool first = true;

///////////////////////////////////////////

void setup() {
  Serial.begin(9600);

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

  pinMode( BLUEPIN, OUTPUT );

  // pinMode( KNOBPIN, INPUT );

  // mode
  pinMode( MODEPIN, INPUT_PULLUP );

  // RGB object
  rgbsquare.begin(); // Initialize pins for output
  rgbsquare.show();  // Turn all LEDs off ASAP
  rgbsquare.setBrightness(100);  // lower max brightness

  // start synth instance
  soul.begin(DIFF);

  // begin thermal
  amg.begin();

      // soul.setupVoice(0,SINE,60,ENVELOPE0,80,64);
      // soul.setupVoice(1,SINE,60,ENVELOPE0,100,64);
      // soul.setupVoice(2,SINE,60,ENVELOPE2,110,64);
      // soul.setupVoice(3,SINE,60,ENVELOPE0,110,64);

  // initialize
  
  // main time
  time = millis();
  // timeouts
  time_oct = time;
  time_mod = time;
  time_rgb = time;
  time_thr = time;
  time_mbt = time;

  for(int i=0; i<4;i++){
    voicetimes[i]=time;
    voicenotes[i]=-1;
    voicestaged[i]=-2;
  }
}

void loop() {

  time = millis();
  modechanged = checkMode();

  Serial.print("Mode is now ");
  Serial.println(mode);
  // modeLights();
  // Serial.print("Mode");
  // Serial.println(mode);
  // // SETUP
  // // voice, wave, pitchm, envelope, length, mod

  // Serial.println(mode);
  // setup my shit breh
  if(modechanged || first){

    clearVoices();

    if(mode == MONOMODE){


      soul.setWave(0,SINE);
      soul.setWave(1,SAW);
      soul.setWave(2,SINE);
      soul.setWave(3,SINE);
      soul.setLength(0,60);
      soul.setLength(1,60);
      soul.setLength(2,60);
      soul.setLength(3,60);
      Serial.println("Set Waves Mono");

    } else if(mode == POLYMODE){

      soul.setWave(0,SAW);
      soul.setWave(1,SAW);
      soul.setWave(2,TRIANGLE);
      soul.setWave(3,SAW);

    // } else if(mode == THERMODE){
      soul.setLength(0,40);
      soul.setLength(1,40);
      soul.setLength(2,40);
      soul.setLength(3,40);
      Serial.println("Set Waves Poly");

    }

    modechanged = false;
    first=false;
  }


  if( (time - time_mbt) >= fadespeed ){
    if(change){
      fade+=1;
      if(fade>255){
        fade=0;
      }

      change = false;
    } else {
      change = true;
    }

    time_mbt = time;
  }

  // analogWrite(BLUEPIN, fade);
  // Serial.println(analogRead(KNOBPIN));  
  
  // // do my mode boy
  switch(mode){
    case 0: monoMode();
            break;
    case 1: polyMode();
            break;
    // case 2: thermalMode();
    //         break;            
  }
}

////////////////// CHECKS  /////////////////
bool checkMode(){

  if( (time - time_mod) >= 2000 ){
    // rgb button


    if(digitalRead(MODEPIN) == LOW){
      // increment mode...
      mode++;

      if(mode>2){
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
    setLength(i,0);
  }
}

uint8_t getAvailVoice(){
  uint8_t i;

  for(i=0; i<4; i++){
    // no note playing on this voice?
    if(voicenotes[i] == -1){
      return i;
    }
  }

  for(i=0; i<4; i++){
    // ready to be interupted?
    if( (time - voicetimes[i]) >= 500 ){
      return i;
    }
  }

  return 5;
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
  if( (time - time_thr) >= 500){
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

      Serial.print("got input");
      Serial.println(i);
    }
  }

  // Serial.println(voicetimes[0]);
  // Serial.println(time);

  if(voicenotes[0]==new_note){
    // Serial.println("got keepplaying");
    if( ( time - voicetimes[0] ) >= 500){
        // set to new time for new note
      voicetimes[0] = time;
      // retrigger
      playMNote(new_note);
    }

  } else if( (top+bottom) == 1 && new_note >= 0) {
    // Serial.println("playing new note");
    playMNote(new_note);
    voicenotes[0] = new_note;
    voicetimes[0] = time;
 
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
      Serial.println("Pressed "+q);
      notetoplay = inputToNote(q);

      Serial.print("New note ");
      Serial.println(notetoplay);

      // count for oct change
      if(q<4){
        top++;
      } else {
        bottom++;
      }

      if(numstaged <= 4){
        voicestaged[numstaged] = notetoplay;
        numstaged++;
      }
    }
  }

  // change octave befor eplaying notes
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


  // loop through staged notes
  for(uint8_t w=0; w<numstaged; w++){
    wasplaying = false;

    // loop through voicenotes
    for(uint8_t i=0; i<4; i++){

      // check timer for this voice (dont disturb voice until 500ms)
      if( ( time - voicetimes[i] ) >= 500){

        // already playing?
        if(voicestaged[w] == voicenotes[i]){

          // retrig
          playPNote(voicestaged[w], i);

          // reset timer for this voice
          voicetimes[i] = time;
          
          // go to next staged w
          wasplaying = true;
          break;

        // not playing this note yet
        }
      }
    }

    if(wasplaying){
      // dont try to play same note on a free voice, skip to next w
      continue;
    }

    uint8_t voice = getAvailVoice();

    // availvoice was found?
    if(voice < 5){

      playPNote(voicestaged[w], voice);
      voicenotes[voice] = voicestaged[w];
      voicetimes[voice] = time;
    }

    // unstage
    voicestaged[w] = -2;
  }



    ////////eh

    // is this voice ready to be overwritten yet
    if( ( time - voicetimes[w] ) >= 500){

      // check if this voice is playing note that


      // voicenotes[w] is note playing on voice w
      // voicetimes[w] is timer for voice w
      // voicestaged[] is notes to take over
        // open voices
        // voices with ready timers

      // is voice w playing any staged notes
      for(uint8_t i=0; i<4; i++){

        if( voicenotes[w] == voicestaged[i] ){
          // retrigger dis
          playPNote(voicestaged[i], w);
          voicestaged[i] = -2;
          numstaged -= 1;
        }

      }

      // reset voice timer
      voicetimes[w] = time;
    }

  for(uint8_t i=0; i<numstaged; i++){


  // note to play is found
  if(voicestaged[i]>0){


    uint8_t voice = getAvailVoice();


    // availvoice was found
    if(voice < 5){
      playPNote(voicestaged[w], voice);
    }
  } else {

  }


}


   



// ////////// old
  //   if(keepplaying){
      
  //     // wait for retrigger if held
  //     if( ( time - voicetimes[noteindex] ) >= 500){
  //         // set to new time for new note
  //       voicetimes[noteindex] = time;
  //       // retrigger
  //       playPNote(this_note, noteindex);
  //     }

  //   } else {
  //     // choose random voice to overwrite
  //     noteindex = rand()*3;
  //     voicetimes[noteindex] = time;
  //     voicenotes[noteindex] = this_note;
  //     playPNote(this_note, noteindex);
  //   }

  //   // reset for next input check
  //   keepplaying = false;

  // // clean up note time for (no longer held) note
  // // note will play out on its own
  // for(int i=0; i<4; i++){
  //   if( voicetimes[i]>0 && ( time - voicetimes[i] ) >= 6000){
  //     voicenotes[i] = -1;
  //     voicetimes[i] = 0;
  //   }
  // }
//////////////////////

}

void playPNote(int note, int voice) {
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

  // thermal
  for(int q=0; q<64; q++){
    // get next r,g,b
    color = pixelToColor(thermal[q]);
    rgbsquare.setPixelColor(q, color); // 'On' pixel at head
    // delay(20);                        // Pause 20 milliseconds (~50 FPS)
  }

  rgbsquare.show();                     // Refresh strip
}
  
