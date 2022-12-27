#ifndef _CTS7_H_
#define _CTS7_H_

/* SETTINGS CONSTANTS — Change these to fit yours needs */
namespace CTS7{  
  /* must be a multiple of 2, higher numbers will lower sound quality */
  const byte VOICE_COUNT = 6;
    
}

/* FUNCTION SET — In case you need to re-check which built-in functions are available */
namespace CTS7{  
  /* Initialize VixenSound */
  void begin();


  /* UTILITIES */  
    /* Creates a sine wave in the specified Wavetable RAM slot
     * Slot 0 is automatically filled with 64 samples at startup.
     * SIZE: 0 means 64 samples, 1 means 32 samples. Default is 0 (64 samples)
     * SLOT: Index of the slot to fill. 0 — 7, 64 samples will fill two slots, so writing 0 will fill slot 0 and 1 */
    void loadSineWave(byte slot, byte size = 0);
    
    /* Returns the frequency for a given pitch number (0 means A4 — 440Hz) */
    float getFrequency(float pitch);
    
    /* Returns the tuning number for a given frequency */
    uint16_t getFnumber(float frequency);
  
    /* Does the conversion directly from pitch number to tuning number */
    uint16_t pitchToFnumber(float pitch);
}

namespace CTS7{  
  /* CORE CONSTANTS — Results from changing these are undefined */
  const byte quarter_sine_lut[] PROGMEM = {0x80, 0x8B, 0x98, 0xA4, 0xB0, 0xBB, 0xC6, 0xD0, 0xD9, 0xE2, 0xE9, 0xEF, 0xF5, 0xF9, 0xFC, 0xFE, 0xFF};
  const byte LEFT_OUTPUT  = 11;   /* No, you can`t change this pins, I only have this for consistency */
  const byte RIGHT_OUTPUT = 3;
  auto& DAC_LEFT  = OCR2A;
  auto& DAC_RIGHT = OCR2B;  
  const unsigned int HALF_REF_CLOCK = 31250;
  const unsigned int WAVE_RAM_SIZE  = 256;
  
  byte wave_ram[WAVE_RAM_SIZE];
    
  struct Voice {
    uint16_t phase, tune = 0;
    uint8_t wave_select = 0;
    uint8_t wave_mask = 0x3F;
    uint8_t attenuator_l = 0;
    uint8_t attenuator_r = 0;
  } voice[VOICE_COUNT];  
  
  byte current_voice = 0;
  
  struct Mixer { uint16_t left, right; } mixer;
  
  void begin(){
    TCCR2A = 0xA3;
    TCCR2B = 0x01;
    TIMSK2 = 0x01;
    pinMode(LEFT_OUTPUT,  OUTPUT); 
    pinMode(RIGHT_OUTPUT, OUTPUT);
    loadSineWave(0, 0);
  }

  /* Function definitions */
  /* I know some programmers don't like single-lining functions... but i don't see any reason not to do it in this specific case */
  float getFrequency(float pitch){return 440*pow(1.059463, pitch);}
  uint16_t getFnumber(float frequency){return frequency/((HALF_REF_CLOCK/VOICE_COUNT)/pow(2, 14));}
  uint16_t pitchToFnumber(float pitch){return getFnumber(getFrequency(pitch));}
  void loadSineWave(byte slot, byte size){
    size+=1;
    slot*=32;
    for(byte i = 0; i < 16/size; i++){
      byte i2 = i*size;
      wave_ram[i   +slot] = wave[i2];
      wave_ram[i+16+slot] = wave[16-i2];
      wave_ram[i+32+slot] = 0x100-wave[i2];
      wave_ram[i+48+slot] = 0x100-wave[16-i2];
    }
  }
}

ISR(TIMER2_OVF_vect){
  using namespace CTS7;
  
  if(current_voice == VOICE_COUNT){
     current_voice = 0;     
     DAC_LEFT  = mixer.left >> 8;
     DAC_RIGHT = mixer.right >> 8;
     mixer.left  = 0;
     mixer.right = 0;
  }
  
  byte wave_address; 
  byte out;
  voice[current_voice].phase += voice[current_voice].tune;
  wave_address = ((voice[current_voice].phase >> 8) & voice[current_voice].wave_mask) | voice[current_voice].wave_select;
  out = wave_ram[wave_address];
  mixer.left  += out*voice[current_voice].attenuator_l;
  mixer.right += out*voice[current_voice].attenuator_r;    
  current_voice++;
  
  voice[current_voice].phase += voice[current_voice].tune;
  wave_address = ((voice[current_voice].phase >> 8) & voice[current_voice].wave_mask) | voice[current_voice].wave_select;
  out = wave_ram[wave_address];
  mixer.left  += out*voice[current_voice].attenuator_l;
  mixer.right += out*voice[current_voice].attenuator_r;
  current_voice++;
}





#endif
