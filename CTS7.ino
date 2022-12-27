#include "CTS7.h"

#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

using namespace CTS7;

byte wave[] = {0x80, 0x8B, 0x98, 0xA4, 0xB0, 0xBB, 0xC6, 0xD0, 0xD9, 0xE2, 0xE9, 0xEF, 0xF5, 0xF9, 0xFC, 0xFE, 0xFF};

void setup(){
  for(byte i = 0; i < 16; i++){
    wave_ram[i   ] = wave[i];
    wave_ram[i+16] = wave[16-i];
    wave_ram[i+32] = 0x100-wave[i];
    wave_ram[i+48] = 0x100-wave[16-i];
  }
  for(byte i = 0; i < 16; i++){
    wave_ram[i   +64] = wave[i];
    wave_ram[i+16+64] = wave[16-i];
    wave_ram[i+32+64] = wave[i];
    wave_ram[i+48+64] = wave[16-i];
  }
  for(byte i = 0; i < 16; i++){
    wave_ram[i   +128] = wave[i];
    wave_ram[i+16+128] = 0x80;
    wave_ram[i+32+128] = 0x100-wave[i];
    wave_ram[i+48+128] = 0x80;
  }
  
  MIDI.begin(MIDI_CHANNEL_OMNI);
  Serial.begin(115200);
  MIDI.setHandleNoteOn(MyHandleNoteOn);
  MIDI.setHandleNoteOff(MyHandleNoteOff);
  MIDI.turnThruOff();
  CTS7::begin();
}

float notes[VOICE_COUNT];
unsigned long env_timer = 0;
float t;
void loop(){
  MIDI.read();
  if(millis() - env_timer > 30){
    env_timer = millis();
    t += 0.5;
    //voice[0].tune = pitchToFnumber(notes[0]+sin(t)*0.5);
    
    for(byte i = 0; i < VOICE_COUNT; i++){
      if(voice[i].attenuator_l > 0) voice[i].attenuator_l--;
      if(voice[i].attenuator_r > 0) voice[i].attenuator_r--;      
    }
  }
}

byte c = 0;
void MyHandleNoteOn(byte channel, byte pitch, byte velocity) { 
  c++; 
  c = (c == VOICE_COUNT) ? 0 : c;
  voice[c].tune = pitchToFnumber(pitch-48);  
  voice[c].attenuator_l = 28;
  voice[c].wave_select = (channel-1)<<6;
}

void MyHandleNoteOff(byte channel, byte pitch, byte velocity) {
}
