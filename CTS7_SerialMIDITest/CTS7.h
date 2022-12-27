#ifndef _CTS7_H_
#define _CTS7_H_

#define FROM_PROGMEM(address_short) __LPM((uint16_t)(address_short))

namespace CTS7{
  /* SETTINGS CONSTANTS — Change these to fit yours needs */
  /* must be a multiple of 2 */
  const byte VOICE_COUNT = 8;
  
  
}

namespace CTS7{  
  /* CORE CONSTANTS — Results from changing these are undefined */
  const byte quarter_sine_lut[] PROGMEM = {0x80, 0x8B, 0x98, 0xA4, 0xB0, 0xBB, 0xC6, 0xD0, 0xD9, 0xE2, 0xE9, 0xEF, 0xF5, 0xF9, 0xFC, 0xFE, 0xFF};
  const byte LEFT_OUTPUT  = 11;   /* No, you can`t change this pins */
  const byte RIGHT_OUTPUT = 3;
  auto& DAC_LEFT  = OCR2A;
  auto& DAC_RIGHT = OCR2B;  
  const unsigned int HALF_REF_CLOCK = 31250;
  
  byte wave_ram[256];
  byte current_voice = 0;
  
  struct Mixer { uint16_t left, right; } mixer;
  
  struct Voice {
    uint16_t phase, tune = 0;
    uint8_t wave_select = 0;
    uint8_t wave_mask = 0x3F;
    uint8_t attenuator_l = 0;
    uint8_t attenuator_r = 0;
  } voice[VOICE_COUNT];

  void begin();
  uint16_t getFnumber();
  float getFrequency();
  uint16_t pitchToFnumber();
  byte getWaveAddress();
  void setAttenuation();
  
  void begin(){
    TCCR2A = 0xA3;
    TCCR2B = 0x01;
    TIMSK2 = 0x01;
    pinMode(LEFT_OUTPUT,  OUTPUT); 
    pinMode(RIGHT_OUTPUT, OUTPUT);
  }

  float getFrequency(float pitch){
    return 440*pow(1.059463, pitch);
  }
  uint16_t getFnumber(float frequency){
    return (frequency/((HALF_REF_CLOCK/VOICE_COUNT)/(pow(2, 14))));
  }
  uint16_t pitchToFnumber(float pitch){
    return getFnumber(getFrequency(pitch));
  }  
}

ISR(TIMER2_OVF_vect){
  using namespace CTS7;
  
  if(current_voice == VOICE_COUNT){
     current_voice = 0;     
     DAC_LEFT  = mixer.left >> 8;
     //DAC_RIGHT = mixer.right >> 8;
     mixer.left  = 0;
     mixer.right = 0;
  }
  
  byte wave_address; 
  byte out;
  voice[current_voice].phase += voice[current_voice].tune;
  wave_address = ((voice[current_voice].phase >> 8) & voice[current_voice].wave_mask) | voice[current_voice].wave_select;
  out = wave_ram[wave_address];
  mixer.left  += out*voice[current_voice].attenuator_l;
  //mixer.right += out*voice[current_voice].attenuator_r;    
  current_voice++;
  
  voice[current_voice].phase += voice[current_voice].tune;
  wave_address = ((voice[current_voice].phase >> 8) & voice[current_voice].wave_mask) | voice[current_voice].wave_select;
  out = wave_ram[wave_address];
  mixer.left  += out*voice[current_voice].attenuator_l;
  //mixer.right += out*voice[current_voice].attenuator_r;
  current_voice++;
}





#endif
