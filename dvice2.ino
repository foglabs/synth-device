#include <synth.h>
#include "notes.h"

#include <Adafruit_DotStar.h>
#include <SPI.h>

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

#define MONOMODE 0
#define POLYMODE 1
#define THERMODE 2

// RGB
Adafruit_DotStar rgbsquare = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

// synth
synth soul;

void setup() {

  // 0,1,2,4,5,6,7,8 arcade pins (input)
  // 3,11 pwm pins
  // 9,10 data/clock for rgb
  // A0 fx knob
  // A4,A5 thermal pins
  
  // setup arcades
  pinMode( arcade0, INPUT_PULLUP );
  pinMode( arcade1, INPUT_PULLUP );
  pinMode( arcade2, INPUT_PULLUP );
  pinMode( arcade3, INPUT_PULLUP );
  pinMode( arcade4, INPUT_PULLUP );
  pinMode( arcade5, INPUT_PULLUP );
  pinMode( arcade6, INPUT_PULLUP );
  pinMode( arcade7, INPUT_PULLUP );

  // RGB object
  rgbsquare.begin(); // Initialize pins for output
  rgbsquare.show();  // Turn all LEDs off ASAP

  // start synth instance
  edgar.begin(DIFF);
}

int[8] arcades;
int[8] arcadepins;

unsigned long[4] voicetimes;
int[4] voicenotes;

int octave=3;
int mode=MONOMODE;
int note_playing=0;

void loop() {

  if(!!!!modechanged){
    if(mode == MONOMODE){
      // setup voices

      edgar.setupVoice(0,SINE,60,ENVELOPE0,80,64);
      edgar.setupVoice(1,SINE,60,ENVELOPE0,100,64);
      edgar.setupVoice(2,SINE,60,ENVELOPE2,110,64);
      edgar.setupVoice(3,SINE,60,ENVELOPE0,110,64);

    } else if (mode == POLYMODE){

    } else if (mode == THERMODE){

    }
  }

  // read 


}

//////////////////GLOBALS/////////////////




////////// MODES //////////////
void monoMode() {
////// [8] inputs, int note_playing

  // get inputs
    // get octave from knob

    // collect inputted notes in inputs array
  for(int i=0; i<8; i++){
    arcades[i] = getArcade( arcadepins[i] );
  }


  handleNotes( arcades );
  // kill note
    //  if no inputs
      // set note length to 0
      // zero out note_playing var 

  // play note
    // every 10 ms, check for note change
      // if inputs include note_playing, re-trigger note_playing
      // if not, trigger new first note (if multi, choose random?)

  // set modulation (thermal or whateva)

  // light lights

  // cleanup?
}


//+++/ option to randomly switch waveform?
  // setupVoice on that number whenever needed
void polyMode() {
////// [8] inputs, [4] notes_playing
  // noteplaying -> [nil,nil,nil,128] if truthy, it's a note that is playing, otherwise voice is available

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
    // trigger first 4 notes of input

  // set modulation (thermal or whateva)



  // light lights


}


void thermalMode() {

}

//////////// INPUT RELATED FUNCTIONS /////////
int getArcade(pin) {
  return digitalRead(pin);
}

int inputToNote(oct, offset) {
  // 16 possible 'octaves' w/ 8 each
  return (oct*8)+offset;
}

//////////// PLAY/Kill NOTES /////////
void handleMNotes(inputs) {
  bool keepplaying = false;
  int new_note = -1;
  int howmanyheld = 0;

  // count for octave change
  while(howmanyheld<4){
    
    // only play if one inputs
    if(howmanyheld<2){

      for(int i=0; i<8; i++){
        if(inputs[i] == HIGH){
          howmanyheld++;
          new_note = inputToNote(octave,i);
        }
      }

      // !!!might have to wait to retrigger held note if constant retrig sounds bad
      if(new_note >= 0){
        playMNote(new_note);
        note_playing=new_note;
      } else {
        // should be unnecessary because note will play out on its own
        // killMNote();    
      }
    }
  }

  // change octave
  if(howmanyheld==4){
    // if up (0-3)

    // if down (4-7)
  }
}

void playMNote(note) {
  for(int i=0; i<4; i++){
    soul.mTrigger(i,note);
  }
}

void killMNote() {
  for(int i=0; i<4; i++){
    soul.setLength(i,0);
  } 
}

void handlePNotes(inputs) {
  bool keepplaying = false;
  int this_note = -1;
  int howmanyheld = 0;
  int noteindex = 0;
  unsigned long time = millis();

  // for octave change
  while(howmanyheld<4){

    // go through inputs
    for(int q=0; q<8; q++){

      this_note = inputToNote(octave,q);

      // if button down - if no longer down, voice will play out its length until replaced
      if( inputs[q] == HIGH ){

        // are we already playing this note?
        for(int i=0; i<4; i++){

          if(voicenotes[i]==this_note){
            keepplaying = true;
            noteindex = i;
          }
        }

        if(keepplaying){
          
          // wait for retrigger if held
          if( ( time - voicetimes[i] ) >= 300){
            // set to new time for new note
            voicetimes[noteindex] = time;
            // retrigger
            playPNote(this_note, i);
          }

        } else {
          // choose random voice to overwrite
          noteindex = random(0,3);
          voicetimes[noteindex] = time;
          voicenotes[noteindex] = this_note;
          playPNote(this_note, noteindex);
        }

      } else {

        // clean up note timer for (no longer held) note
        // note will play out on its own
        for(int i=0; i<4; i++){

          if(voicenotes[i]==this_note){

            if( ( time - voicetimes[i] ) >= 300){
              voicenotes[i] = -1;
              voicetimes[i] = -1;
            }

          }
        }
        
      }

      // reset for next input check
      keepplaying = false;
    }
  }

  // exit if too many inputs
  while(howmanyheld<2){
    
    for(int i=0; i<8; i++){
      if(inputs[i] == HIGH){
        howmanyheld++;
        new_note = inputToNote(octave,i);
      }
    }

    // !!!might have to wait to retrigger held note if constant retrig sounds bad
    if(new_note >= 0){
      playMNote(new_note);
      note_playing=new_note;
    } else {
      // should be unnecessary because note will play out on its own
      // killMNote();    
    }
  }
}

void playPNote(note, voice) {
  soul.mTrigger(voice,note);
}

//////////// LIGHTS /////////

void lightRgb(colors, text){
  if(colors){
  // colors
    for(int q=0; q<64; q++){
      strip.setPixelColor(head, color); // 'On' pixel at head
      strip.setPixelColor(tail, 0);     // 'Off' pixel at tail
      strip.show();                     // Refresh strip
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
  