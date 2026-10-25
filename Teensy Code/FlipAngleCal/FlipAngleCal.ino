#include "Arduino.h"
#include <imxrt.h>
#include "teensymri.h"
#include "ltc2632.h"

#define LARMOR_FREQUENCY 4.485e6

void setup() {
  //Set up the MRI hardware
  AFE_setup();
  GPIO_setup();

  Serial.begin(9600);
  Serial.println("Test");
}

void loop() {
  set_gradient_amplitude(0);

  uint8_t pw = 15;

  //Turn off the gradients for this lab
  set_gradient_amplitude(0);
  gradient(GR_Y, GR_OFF);
  gradient(GR_Z, GR_OFF);

  //RF amplitude is 0-180
  set_rf_amplitude(180);

  while (true) {

    //Wait for the FPGA to trigger a sequence
    while(!digitalRead(STARTSEQ1)) {};
  
    // 90
    generatePulses(LARMOR_FREQUENCY, pw);
    CLAMP //Don't let the ringing coil affect the sample
    delayMicroseconds(200);
    RELEASE

    delayMicroseconds(1600); //Delay between 90 and 180

    // 180
    generatePulses(LARMOR_FREQUENCY, pw*2);
    CLAMP
    delayMicroseconds(200);
    RELEASE

    delay(1000);

    //Increment the pulse width to the next setting
    // pw = pw + 2;
  }
}
