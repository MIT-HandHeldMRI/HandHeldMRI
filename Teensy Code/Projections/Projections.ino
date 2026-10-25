#include "Arduino.h"
#include <imxrt.h>
#include "teensymri.h"
#include "ltc2632.h"

extern "C" uint32_t set_arm_clock(uint32_t frequency); //These lines may not be necessary?
#define ARM_CLOCK_FREQ 600000000

void setup() {

  AFE_setup();
  GPIO_setup();
  // DAC_Setup();
  set_arm_clock(ARM_CLOCK_FREQ);
  initCycleCounter();
  Serial.begin(9600);
  Serial.println("Test");
}

void loop() {

  int line = -1;
  bool dograd = true;
  int phasetime = 0;
  bool dophase = false;

  set_gradient_amplitude(0);

  uint8_t pw = 14;

  while (true) {

    digitalWriteFast(STARTSEQ0, LOW);
    gradient(GR_Y, GR_OFF);
    gradient(GR_Z, GR_OFF);
    phasetime = abs(line) * 60;
    // float angle = line * PI/64;
    float angle = 63/64;
    // Serial.println(angle);
    int zgrad = (int) (cos(angle) * 150);
    int ygrad = (int) (sin(angle) * 180);
    int8_t zpol = 0;
    if(zgrad < 0){
      zpol = -1;
    }else{
      zpol = 1;
    }

    int8_t ypol = 0;
    if(ygrad < 0){
      ypol = -1;
    }else{
      ypol = 1;
    }
    Serial.print("Angle: ");
    Serial.println(angle);
    
    set_gradient_amplitude((uint8_t) abs(ygrad)); //For Y
    set_rf_amplitude(180);

    while(!digitalRead(STARTSEQ1)) {};
    // while(IMXRT_FLEXPWM2.SM[0].CNT) {};
    digitalWriteFast(READOUT, HIGH);
  
    // 90
    generatePulses(4.52e6, pw);
    ZZ
    delayMicroseconds(200);
    TR

    delayMicroseconds(1600); //Delay between 90 and 180

    // 180
    generatePulses(4.52e6, pw*2);
    ZZ
    set_rf_amplitude((uint8_t) abs(zgrad));
    TR
    if(dograd){
      delayMicroseconds(600);

      gradient(GR_Y, ypol); 
      gradient(GR_Z, zpol); 
      delayMicroseconds(600);
      gradient(GR_Z, -zpol); 
      gradient(GR_Y, -ypol); 
      delayMicroseconds(1200);
      gradient(GR_Z, GR_OFF); 
      gradient(GR_Y, GR_OFF); 
    }
    digitalWriteFast(READOUT, LOW);
    delay(500);
    
    // line = line + 1;
    // pw = pw + 2;

  }
}
