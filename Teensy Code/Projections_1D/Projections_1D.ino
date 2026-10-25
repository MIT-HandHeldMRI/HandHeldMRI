#include "Arduino.h"
#include <imxrt.h>
#include "teensymri.h"
#include "ltc2632.h"

extern "C" uint32_t set_arm_clock(uint32_t frequency);
#define ARM_CLOCK_FREQ 600000000

#define LARMOR_FREQUENCY 4.52e6
#define SIGN(x) ((x) > 0 ? 1 : -1)

void setup() {

  AFE_setup();
  GPIO_setup();
  // DAC_Setup();
  set_arm_clock(ARM_CLOCK_FREQ);
  initCycleCounter();
  Serial.begin(9600);
}

void loop() {

  uint8_t pw = 18;
  float angle = 0;

  while (true) {

    gradient(GR_Y, GR_OFF);
    gradient(GR_Z, GR_OFF);

    //Read a potentiometer to get adjustable gradients
    // potval = analogRead(A10) >> 2;
    // Serial.print("Gradient: ");
    // Serial.println(potval);

    int ygrad = round(sin(angle) * 180);
    int zgrad = round(cos(angle) * 150);

    set_rf_amplitude(180);  //This controls both Z grad and RF amplitude
    // while(!digitalRead(STARTSEQ1)){}
    digitalWriteFast(READOUT, HIGH);
  
    // 90
    generatePulses(LARMOR_FREQUENCY, pw);
    CLAMP
    delayMicroseconds(200);
    RELEASE

    delayMicroseconds(1600); //Delay between 90 and 180

    // 180
    generatePulses(LARMOR_FREQUENCY, pw*2);
    CLAMP
    delayMicroseconds(200);

    set_gradient_amplitude((uint8_t) abs(zgrad)); //This only controls the Y gradient
    set_rf_amplitude((uint8_t) abs(ygrad)); //This controls both Z grad and RF amplitude
    RELEASE
    if(false){ //change this to true to enable the gradients
      delayMicroseconds(400);

      gradient(GR_Z, -SIGN(zgrad)); 
      gradient(GR_Y, -SIGN(ygrad)); 
      delayMicroseconds(600);
      gradient(GR_Z, SIGN(zgrad)); 
      gradient(GR_Y, SIGN(ygrad)); 
      delayMicroseconds(1200);
      gradient(GR_Z, 0); 
      gradient(GR_Y, 0); 
    }
    digitalWriteFast(READOUT, LOW);
    delay(1000);

    // Uncomment these lines to do flip angle calibration
    // You may want to change the delay above to 2000 to slow it down
    // pw = pw + 2;
    // angle = angle + PI/20;
    // Serial.print("PW: ");
    // Serial.println(pw);

  }
}
