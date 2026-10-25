#include "Arduino.h"
#include <imxrt.h>
#include "teensymri.h"
#include "ltc2632.h"

extern "C" uint32_t set_arm_clock(uint32_t frequency); //These lines may not be necessary?
#define ARM_CLOCK_FREQ 600000000

#define LARMOR_FREQUENCY 4.52e6
#define SIGN(x) ((x) > 0 ? 1 : -1)
#define NLINES 59

void setup() {

  AFE_setup();
  GPIO_setup();
  // DAC_Setup();
  set_arm_clock(ARM_CLOCK_FREQ);
  initCycleCounter();
  Serial.begin(9600);
}

void loop() {

  int line = -NLINES/2;
  int phasetime = 0;

  set_gradient_amplitude(0);

  uint8_t pw = 18;

  while (true) {

    digitalWriteFast(STARTSEQ0, LOW);
    gradient(GR_Y, GR_OFF);
    gradient(GR_Z, GR_OFF);
    phasetime =  abs(line) * 600/NLINES;
    Serial.print("PE Time: ");
    Serial.println(phasetime);
    
    set_gradient_amplitude(240);
    set_rf_amplitude(180);

    // while(!digitalRead(STARTSEQ1)) {};
    digitalWriteFast(READOUT, HIGH);
  
    // 90
    generatePulses(LARMOR_FREQUENCY, pw);
    CLAMP
    delayMicroseconds(200);
    RELEASE

    delayMicroseconds(11600); //Delay between 90 and 180

    // 180
    generatePulses(LARMOR_FREQUENCY, pw*2);
    CLAMP
    delayMicroseconds(100);
    RELEASE
    delayMicroseconds(10000);
    if(false){
      gradient(GR_Z, SIGN(line));
      delayMicroseconds(phasetime);
      gradient(GR_Z, GR_OFF);
      delayMicroseconds(500 - phasetime);

      gradient(GR_Y, GR_NEG); 
      delayMicroseconds(600);
      gradient(GR_Y, GR_POS); 
      delayMicroseconds(1200);
      gradient(GR_Y, GR_OFF); 
    }

    digitalWriteFast(READOUT, LOW);
    delay(1000);
    line = line + 1;
  }
}
