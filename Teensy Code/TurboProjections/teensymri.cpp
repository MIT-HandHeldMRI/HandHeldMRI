#include "teensymri.h"
#include "bitbang_spi.h"
#include "ltc2632.h"
#include <Arduino.h>
#include <SPI.h>

void GPIO_setup(){
  ARM_DEMCR |= ARM_DEMCR_TRCENA;  // info from teensy forum, access cycle counter
  ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
  pinMode(NOP_PIN, OUTPUT);

  // Simultanously switched Driver pins
  GPIO6_GDIR |= 0xFFFF0000;

  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);

  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(20, OUTPUT);
  pinMode(21, OUTPUT);

  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);

  pinMode(38, OUTPUT);
  pinMode(39, OUTPUT);
  pinMode(40, OUTPUT);
  pinMode(41, OUTPUT);

  pinMode(24, INPUT);
  pinMode(25, INPUT);
 
  GPIO9_GDIR |= 0x0000FFFF;
  pinMode(02, OUTPUT); //GPIO9-04
  pinMode(03, OUTPUT); //GPIO9-05
  pinMode(04, OUTPUT); //GPIO9-06`

  pinMode(READOUT, OUTPUT);
  digitalWriteFast(READOUT, false);
}

void AFE_setup() {   

  pinMode(11, OUTPUT);
  analogWriteResolution(8);  
  analogWriteFrequency(11, 50000000);  
  analogWrite(11, 128); 

  pinMode(STARTSEQ0, OUTPUT);
  pinMode(STARTSEQ1, INPUT);
  pinMode(AFE_MISO_PIN, INPUT);
  pinMode(AFE_MOSI_PIN, OUTPUT);
  pinMode(AFE_SCK_PIN, OUTPUT);
  pinMode(AFE_CE_PIN, OUTPUT);
  pinMode(AFE_RESET, OUTPUT);

  digitalWrite(AFE_SCK_PIN, 1);
  digitalWrite(AFE_RESET, 0);
  digitalWrite(AFE_CE_PIN, 1);
  delay(1);
  digitalWrite(AFE_RESET, 1);
  delay(1);
  digitalWrite(AFE_RESET, 0);

  delay(10);
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  //Serial.begin(115200); //value doesn't matter

  //extremely low frequency (can increase...just low since this is one time programming:)
  int freq = 10000;  //hz
  //
  // digitalWrite(AFE_CE_PIN, 0);
  // spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x031100);
  // digitalWrite(AFE_CE_PIN, 1);
  // delay(5);
  // //write to enable reads
  // digitalWrite(AFE_CE_PIN, 0);
  // spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x000002);
  // digitalWrite(AFE_CE_PIN, 1);
  // delay(5);
  // digitalWrite(AFE_CE_PIN, 0);
  // spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x031100);
  // digitalWrite(AFE_CE_PIN, 1);
  // delay(5);
  // //write to enable writes
  // digitalWrite(AFE_CE_PIN, 0);
  // spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x000000);
  // digitalWrite(AFE_CE_PIN, 1);
  //real config

  //pages 24 and 25 of data sheet have relevant info:
  //SOFTWARE RESET (brings up all channels):
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x000001);
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);
  //sets LVDS to be MSB first and offset binary:
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x040018);
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);

  int channel_enable = 0b11000000; //1=on, 0=off (channels 7 and 8 on)
  //int channel_enable = 0b11000011; //1=on, 0=off (channels 7 and 8 on and 1 and 2)

  // LNA Gain
  // 2 bits
  // 00: 18 dB
  // 01: 24 dB
  // 10: 15 dB
  // 11: Reserved
  int lna_gain = 0b00;

  // PGA Gain
  // 1 bit
  // 0:24 dB
  // 1:30 dB
  int pga_gain = 0b0;

  // Active Termination
  // 1 bit
  // 0: Disable
  // 1: Enable active termination
  int act_term = 0b0;


  // VCA Attenuation
  // 3 bits
  // 000: 0-dB attenuation
  // 001: 6-dB attenuation
  // N: About N * 6 dB attenuation
  int vca_attn = 0b000;

  // Test Pattern
  // 3 Bits
  // 000: Normal operation
  // 001: Sync
  // 010: De-skew
  // 011: Custom
  // 2[15:13] 0x2[15:13] 0 TEST_PATTERN_MODES 100:All 1's
  // 101: Toggle
  // 110: All 0's
  // 111: Ramp. Note: Reg.0x16 should be set as 1, i.e. demodulator is disabled.
  //int test_pattern = 0b101;
  //int test_pattern = 0b111;  //on ramp...set to 000 for normal operation.
  int test_pattern = 0b000;  //on ramp...set to 000 for normal operation.


  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, (0x0207F8 ^ (channel_enable << 3)) | (test_pattern << 13));
  digitalWrite(AFE_CE_PIN, 1);


  //LVDS shut down 10:3
  /*
  7F8 all channels

  1 2 3 4
  0 0 0 0   0x02E7F8
  1 0 0 0   0x02E7F0
  0 1 0 0   0x02E7E8
  1 1 0 0   0x02E7E0
  0 0 1 0   0x02E7D8
  1 0 1 0   0x02E7D0
  0 1 1 0   0x02E7C8
  1 1 1 0   0x02E7C0
  0 0 0 1   0x02E7B8
  1 0 0 1   0x02E7B0
  0 1 0 1   0x02E7A8
  1 1 0 1   0x02E7A0
  0 0 1 1   0x02E798
  1 0 1 1   0x02E790
  0 1 1 1   0x02E788
  1 1 1 1   0x02E780


  */
  //shut down ADCs that aren't being used:
  delay(5);
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x0183FC ^ (channel_enable << 2)); //shut down ADCs. Channel 1 on: 0x0183F8. 1&3: 0x0183E8
  digitalWrite(AFE_CE_PIN, 1);


  delay(5);
  //shut down LNAs not being used (big power savings):
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x3500FF ^ (channel_enable << 0)); //shut down LNAs.
  digitalWrite(AFE_CE_PIN, 1);

  delay(5);
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x030000); //0x030000 no inversion
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);

  //CONSIDER TURNING THESE ON....I'll need to experiment (joe 6/11/2025)
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x340000 | (act_term << 8) | (lna_gain << 13)); //active termination 0x342100 vs not 0x342000
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);

  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x330008 | (pga_gain << 13)); //PGA gain 30 db: 0x332008, 24db: 0x330008
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);

  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x3B0088 | (vca_attn << 4)); //Attenuation, 0 db 0x3B0088, -30 db 0x3B00D8
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);



  //// AFE5812 only (useless on 5803)
  //digitalWrite(AFE_CE_PIN, 0);
  //spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x160001); //turn off demodulator
  //digitalWrite(AFE_CE_PIN, 1);
  //delay(5);

}

void initCycleCounter() {
    ARM_DEMCR    |= ARM_DEMCR_TRCENA;         // enable debug/trace
    ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;   // enable cycle counter
}

void generatePulses(float freq, uint32_t duration_us) {

    const float cpu_hz = 600e6;       // adjust if not at 600 MHz
    float half_period_cycles = cpu_hz / (2.0f * freq);

    uint64_t now = ARM_DWT_CYCCNT;
    uint64_t end_time = now + (uint64_t)(cpu_hz * duration_us / 1000000.0f);

    float accum = 0;
    uint64_t start = ARM_DWT_CYCCNT;
    GPIO6_DR |= 0x08020000; //Initial state is UP
    GPIO6_DR &= ~(0x04010000); //Clear the down pins

    while ((uint64_t)ARM_DWT_CYCCNT< end_time) {

        // integer and fractional part
        uint32_t whole = (uint32_t)half_period_cycles;
        accum += half_period_cycles - whole;

        if (accum >= 1.0f) {
            whole++;
            accum -= 1.0f;
        }

        // wait until that many cycles have passed
        start = start + whole;
        while ((uint64_t)ARM_DWT_CYCCNT < start) {
            // tight wait loop
        }

        GPIO6_DR_TOGGLE = (0x08020000 | 0x04010000);
    }
}

// void gradient(uint8_t gradient, uint8_t pol){
//   if(gradient == GR_X){
//     //not implemented
//   }else if (gradient == GR_Y){
//       if(pol == GR_POS){
//         GPIO6_DR |=  0x20430000;
//         GPIO6_DR &=  ~(0x10800000);
//       }else if (pol == GR_NEG){
//         GPIO6_DR |= 0x10830000;
//         GPIO6_DR &= ~(0x20400000);
//       }else{
//         GPIO6_DR &=  ~(0x30C00000);
//       }
//   }else if (gradient == GR_Z){
//     if(pol == GR_POS){
//       GPIO6_DR |= 0x81030000;
//       GPIO6_DR &=  ~(0x42000000);
//     }else if (pol == GR_NEG){
//       GPIO6_DR |=  0x42030000;  // P1=N3=1 
//       GPIO6_DR &=  ~(0x81000000);
//     }else{
//       GPIO6_DR &=  ~(0xC3000000);
//     }
//   }
// }

void gradient(uint8_t gradient, int8_t pol){
  if(gradient == GR_X){
    //not implemented
  }else if (gradient == GR_Y){
      if(pol == GR_POS){
        GPIO6_DR |=  0x20830000;
        GPIO6_DR &=  ~(0x10400000);
      }else if (pol == GR_NEG){
        GPIO6_DR |= 0x10430000;
        GPIO6_DR &= ~(0x20800000);
      }else{
        GPIO6_DR &=  ~(0x30C00000);
      }
  }else if (gradient == GR_Z){
    if(pol == GR_POS){
      GPIO6_DR |= 0x82030000;
      GPIO6_DR &=  ~(0x41000000);
    }else if (pol == GR_NEG){
      GPIO6_DR |=  0x41030000;  // P1=N3=1 
      GPIO6_DR &=  ~(0x82000000);
    }else{
      GPIO6_DR &=  ~(0xC3000000);
    }
  }
}

void set_gradient_amplitude(uint16_t amplitude){
  dac_write(DAC_WRITE_UPDATE_REG, DACB, amplitude);
}

void set_rf_amplitude(uint16_t amplitude){
  dac_write(DAC_WRITE_UPDATE_REG, DACA, amplitude);
}