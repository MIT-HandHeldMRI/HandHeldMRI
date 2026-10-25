#include "Arduino.h"
#include <imxrt.h>
#include "teensymri.h"
#include "ltc2632.h"

extern "C" uint32_t set_arm_clock(uint32_t frequency); //These lines may not be necessary?
#define ARM_CLOCK_FREQ 600000000

#define LARMOR_FREQUENCY 4.51e6
#define NUM_PROJECTIONS 8
int zgrads[NUM_PROJECTIONS];
int ygrads[NUM_PROJECTIONS];
#define SIGN(x) ((x) > 0 ? 1 : -1)

#define CLAMP_DUR 200 //microseconds
#define TE 3600 //microseconds
#define LOBE 600 //Gradient prewinder/rewinder width, microseconds
const uint64_t CYCLES_PER_ECHO = (ARM_CLOCK_FREQ/((uint64_t) 1000000) * TE);
const uint64_t CYCLES_PER_HALF_LOBE = (ARM_CLOCK_FREQ/((uint64_t) 1000000) * LOBE);
uint64_t cycle_count;

void setup() {

  AFE_setup();
  GPIO_setup();
  // DAC_Setup();
  set_arm_clock(ARM_CLOCK_FREQ);
  initCycleCounter();
  Serial.begin(9600);

  Serial.println("\n\nMultiecho Projections");
  Serial.print("PROJECTIONS: ");
  Serial.println(NUM_PROJECTIONS);
  Serial.print("TE (ms): ");
  Serial.println(TE);
  Serial.print("GRADIENT HALF LOBE (ms): ");
  Serial.println(LOBE);
  Serial.print("ARM CYCLES PER ECHO: ");
  Serial.println(CYCLES_PER_ECHO);

  //Calculate each of the amplitudes, and store them in an array for fast access
  for(int i = 0; i < NUM_PROJECTIONS; i++){
    float angle = i * PI/NUM_PROJECTIONS;
    zgrads[i] = (int) (cos(angle) * 160);
    ygrads[i] = (int) (sin(angle) * 160);
  }
  
  Serial.print("ZGRADS: [");
  for(int i = 0; i < NUM_PROJECTIONS; i++){
    Serial.print(zgrads[i]);
    Serial.print(", ");
  }
  Serial.println("]");
    Serial.print("YGRADS: [");
  for(int i = 0; i < NUM_PROJECTIONS; i++){
    Serial.print(ygrads[i]);
    Serial.print(", ");
  }
  Serial.println("]");
  
}

void loop() {

  set_gradient_amplitude(0);
  uint8_t pw = 18;

  while (true) {

    digitalWriteFast(READOUT, LOW);
    gradient(GR_Y, GR_OFF);
    gradient(GR_Z, GR_OFF);
    
    set_rf_amplitude(180);

    while(!digitalRead(STARTSEQ1)) {};
    digitalWriteFast(READOUT, HIGH); //Trigger the scope
  
    // 90
    generatePulses(LARMOR_FREQUENCY, pw);
    cycle_count = ARM_DWT_CYCCNT - (pw*300);
    CLAMP
    delayMicroseconds(CLAMP_DUR);
    RELEASE

    while((uint64_t)ARM_DWT_CYCCNT < cycle_count + CYCLES_PER_ECHO/2 - pw*600){} //Delay between 90 and 180

    for(int i = 0; i < NUM_PROJECTIONS; i++){
        // 180
      cycle_count = ARM_DWT_CYCCNT;
      generatePulses(LARMOR_FREQUENCY, pw*2);
      CLAMP
      set_rf_amplitude((uint8_t) abs(zgrads[i]));
      set_gradient_amplitude((uint8_t) abs(ygrads[i])); //For Y
      delayMicroseconds(CLAMP_DUR);
      RELEASE
      if(true){
        while((uint64_t)ARM_DWT_CYCCNT < cycle_count + CYCLES_PER_HALF_LOBE){} //Wait to start gradients

        gradient(GR_Y, SIGN(ygrads[i])); 
        gradient(GR_Z, SIGN(zgrads[i])); 
        delayMicroseconds(600);
        gradient(GR_Y, -SIGN(ygrads[i])); 
        gradient(GR_Z, -SIGN(zgrads[i])); 
        delayMicroseconds(1200);        
        gradient(GR_Y, SIGN(ygrads[i])); 
        gradient(GR_Z, SIGN(zgrads[i])); 
        delayMicroseconds(600);
        gradient(GR_Y, GR_OFF); 
        gradient(GR_Z, GR_OFF); 

        // delayMicroseconds(600);
        set_rf_amplitude(180);
      }
      while((uint64_t)ARM_DWT_CYCCNT < cycle_count + CYCLES_PER_ECHO){} //precise cycle count per echo
    }
    
    
    // Uncomment these lines to do flip angle calibration
    // delay(1000);
    // You may want to change the delay above to 2000 to slow it down
    // pw = pw + 2;
    // Serial.print("PW: ");
    // Serial.println(pw);

  }
}
