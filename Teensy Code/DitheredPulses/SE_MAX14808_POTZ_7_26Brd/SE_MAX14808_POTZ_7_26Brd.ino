#include "Arduino.h"
#include <SPI.h>

extern "C" uint32_t set_arm_clock(uint32_t frequency);

#define ARM_CLOCK_FREQ 600000000  //594000000//600000000 //600000000 //594 000 000  //600000000 //594000000//

#define NOP_PIN 42
#define READOUT 2  
#define READOUT_OLD 32
#define RFA false  // false = RF on channel B.
#define WAITTIME 1000000  // Wait time for timed loop in microseconds.
#define WAITFORFPGA true // true=wait for FPGA to set Teensy bit, false=run once per second.
#define DOGRAD true  // Try the gradients.

// Mapping for 2026 Board
// Maxk14808
// Teensy Pin numbers start from Top right= 1, Top left = 48
// gpio6 bit         |31|30|29|28| |27|26|25|24| |23|22|21|20| |19|18|17|16|
// Teensy4.1 Dbit    |27|26|39|38| |21|20|23|22| |16|17|41|40| |15|14|18|19|
// Teensy4.1 Pin     |30|31|18|19| |06|07|04|05| |11|10|16|17| |12|13|09|08|
//  Max14808 Pins    |--|--|P4|N4| |N6|P6|P7|N7| |N1|P1|P3|N3| |P2|N2|P5|N5|

// gpio6 bit      |15|14|13|12|  |11|10|09|08|  |07|06|05|04|  |03|02|01|00|
// Teensy4.1 Dbit |--|--|25|24|  |--|--|--|--|  |--|--|--|--|  |00|01|--|--|
// Teensy4.1 Pin  |--|--|30|31|  |--|--|--|--|  |--|--|--|--|  |47|46|--|--|
// Max14808       |--|--|--|--|  |--|--|--|--|  |--|--|--|--|  |N8|P8|--|--|

// Mapping for TI DAC7564/DAC8164
// gpio8 bit         |31|30|29|28| |27|26|25|24| |23|22|21|20| |19|18|17|16|
// Teensy4.1 Dbit    |--|--|--|--| |--|--|--|--| |30|31|--|--| |--|28|--|--|
// DAC Pin                                       |CK|SY|--|--| |--|IN|--|--|
// Transfer is SYNC LOW, Data on Clk neg edge, in octal:
// First nibble: 0x1 -- Type of command (immedidate load data)
// Second nibble: 0, 2, 4, 6 in octal for DAC A,B,C,D
// 3rd, 4th and 5th Nibble: 12 bits of data MSB first.
// 6th nibble: four bits of dont care.

// Pins for talking to the AFE5808
const uint8_t AFE_RESET = 33;
const uint8_t AFE_SCK_PIN = 34;
const uint8_t AFE_MOSI_PIN = 35;
const uint8_t AFE_CE_PIN = 36;
const uint8_t AFE_MISO_PIN = 37;

// Flags for alchitry
#define STARTSEQ0 0
#define STARTSEQ1 1


// RF definitions!!!!!!!!!!!!!!!!!!!!!!!
// RF Output A (not currently used) channels 3+4 RF for 2026 board
#if RFA
#define UP GPIO6_DR = 0x10200000;  // P4=N3=0 N4=P3=1
#define DN GPIO6_DR = 0x20100000;  // N4=P3=1 P4=N3=0
#define ZZ GPIO6_DR = 0x00000000;  // TR4, TR3, Clamped
#define ZT GPIO6_DR = 0x00300000;  // 4 Clamped, 3 Open
#define TR GPIO6_DR = 0x30000000;  // 4 Open 3 Clamped
// Gradients Max14808:  3,4 for RF, 3,4 and 7,8 for grad
#define YGN GPIO6_DR = 0x31000004;  // N7=N8=1, P4,3=TR
#define YGP GPIO6_DR = 0x32000008;  // P7=P8=1 P4,3=TR
#define ZGN GPIO6_DR = 0x30480000;  // N1==P2=1, P4,3=TR
#define ZGP GPIO6_DR = 0x30840000;  // P1=N2=1, P4,3=TR
#else
// WATCH OUT FOR OUT OF DATE COMMENTS!!!!!!!
// RF Output B (currently used) For channels 5+6 RF for 2026 board
#define UP GPIO6_DR =  0x08020000;   // P6=N5=1
#define DN GPIO6_DR =  0x04010000;   // N6=P5=1
#define ZZ GPIO6_DR =  0x00000000;   // TR5, TR6, Clamped
#define ZT GPIO6_DR =  0x00000000;   // Both 5 and 6 Open
#define TR GPIO6_DR =  0x00030000;   // 5 Open 6Clamped
// Gradients Max14808:  5,6 for RF, 1,2 for grad
#define YGN GPIO6_DR = 0x01030004;  // P1=N2=1, P4=6,7=TR
#define YGP GPIO6_DR = 0x02030008;  // N1=P2=1 P4,3=TR
#define ZGN GPIO6_DR = 0x008B0000;  // P1=N2=1, P4=6,7=TR
#define ZGP GPIO6_DR = 0x00470000;  // N1=P2=1 P4,3=TR
#endif

#define UW UP  // For the include file RF pulse.
#define DW DN

//#define ZGN GPIO6_DR   = 0x2C830000;  // P4=N1=1 P7,8=TR
//#define ZGP GPIO6_DR   = 0x1C430000;  // N4=P1=1 P7,8=TR//
//#define ZGN GPIO6_DR   = 0x20430000;  // P4=P1=1 P7,8=TR
//#define ZGP GPIO6_DR   = 0x10830000;  // N4=N1=1 P7,8=TR

#define UP21 UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP UP
#define DN21 DN DN DN DN DN DN DN DN DN DN DN DN DN DN DN DN DN DN DN DN DN
#define ZZ10 ZZ     ZZ    ZZ    ZZ    ZZ    ZZ    ZZ    ZZ    ZZ    ZZ
#define ZZ100 ZZ10  ZZ10  ZZ10  ZZ10  ZZ10  ZZ10  ZZ10  ZZ10  ZZ10  ZZ10
#define ZZ1000 ZZ100 ZZ100 ZZ100 ZZ100 ZZ100 ZZ100 ZZ100 ZZ100 ZZ100 ZZ100
#define CLAMP_STDM ZZ1000 ZZ1000 ZZ1000 ZZ1000 ZZ1000 ZZ1000 ZZ1000 ZZ1000

#define ZT10 ZT ZT ZT ZT ZT ZT ZT ZT ZT ZT
#define ZT100 ZT10 ZT10 ZT10 ZT10 ZT10 ZT10 ZT10 ZT10 ZT10 ZT10
#define ZT1000 ZT100 ZT100 ZT100 ZT100 ZT100 ZT100 ZT100 ZT100 ZT100 ZT100
#define CLAMP_STD_T ZT1000 ZT1000 ZT1000 ZT1000 ZT1000 ZT1000 ZT1000 ZT1000

#define CLAMP_STD for(int i=8; i > 0; i--) { ZZ1000 }


#define TR10 TR TR TR TR TR TR TR TR TR TR
#define TR100 TR10 TR10 TR10 TR10 TR10 TR10 TR10 TR10 TR10 TR10
#define TR1000 TR100 TR100 TR100 TR100 TR100 TR100 TR100 TR100 TR100 TR100

#define ZGN10 ZGN ZGN ZGN ZGN ZGN ZGN ZGN ZGN ZGN ZGN
#define ZGN100 ZGN10 ZGN10 ZGN10 ZGN10 ZGN10 ZGN10 ZGN10 ZGN10 ZGN10 ZGN10
#define ZGN1000 ZGN100 ZGN100 ZGN100 ZGN100 ZGN100 ZGN100 ZGN100 ZGN100 ZGN100 ZGN100

#define ZGP10 ZGP ZGP ZGP ZGP ZGP ZGP ZGP ZGP ZGP ZGP
#define ZGP100 ZGP10 ZGP10 ZGP10 ZGP10 ZGP10 ZGP10 ZGP10 ZGP10 ZGP10 ZGP10
#define ZGP1000 ZGP100 ZGP100 ZGP100 ZGP100 ZGP100 ZGP100 ZGP100 ZGP100 ZGP100 ZGP100

#define YGN10 YGN YGN YGN YGN YGN YGN YGN YGN YGN YGN
#define YGN100 YGN10 YGN10 YGN10 YGN10 YGN10 YGN10 YGN10 YGN10 YGN10 YGN10
#define YGN1000 YGN100 YGN100 YGN100 YGN100 YGN100 YGN100 YGN100 YGN100 YGN100 YGN100

#define YGP10 YGP YGP YGP YGP YGP YGP YGP YGP YGP YGP
#define YGP100 YGP10 YGP10 YGP10 YGP10 YGP10 YGP10 YGP10 YGP10 YGP10 YGP10
#define YGP1000 YGP100 YGP100 YGP100 YGP100 YGP100 YGP100 YGP100 YGP100 YGP100 YGP100

#define NOP digitalWriteFast(NOP_PIN, false);
#define NOP40 NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP
#define NOP800 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40 NOP40

#include "dac_defs_7bit.h"

// ****END OF RF VARIABLE DEFINITIONS**** //

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

//Joe's SPI bit-bang code
/* Shift a byte out serially with the given frequency in Hz (<= 500kHz) */
void spi_write(uint8_t data_pin, uint8_t clock_pin, uint32_t freq, uint8_t bit_order, uint8_t mode, uint8_t bits, uint32_t val_out) {
  uint32_t period = (freq >= 500000) ? 1 : (500000 / freq);  // Half clock period in uS
  uint8_t cpol = (mode == SPI_MODE2 || mode == SPI_MODE3);
  uint8_t cpha = (mode == SPI_MODE1 || mode == SPI_MODE3);
  uint8_t sck = cpol ? HIGH : LOW;

  uint8_t i;
  uint32_t start_time;

  // Set clock idle for 2 periods
  digitalWrite(clock_pin, sck);
  delayMicroseconds(period * 4);

  for (i = 0; i < bits; i++) {
    start_time = micros();

    // Shift bit out
    if (bit_order == LSBFIRST)
      digitalWrite(data_pin, !!(val_out & (1 << i)));
    else
      digitalWrite(data_pin, !!(val_out & (1 << ((bits - 1) - i))));

    // Toggle clock leading edge
    sck = !sck;
    if (cpha) {
      digitalWrite(clock_pin, sck);
      while (micros() - start_time < period)
        ;
    } else {
      while (micros() - start_time < period)
        ;
      digitalWrite(clock_pin, sck);
    }

    // Toggle clock trailing edge
    start_time = micros();
    sck = !sck;
    if (cpha) {
      digitalWrite(clock_pin, sck);
      while (micros() - start_time < period)
        ;
    } else {
      while (micros() - start_time < period)
        ;
      digitalWrite(clock_pin, sck);
    }
  }
}


// Setup routine for the AFE
void AFE_setup() {

  //extremely low frequency (can increase...just low since this is one time programming:)
  int freq = 10000;  //hz
  
  //pages 24 and 25 of data sheet have relevant info:
  //SOFTWARE RESET (brings up all channels):
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x000001);
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);

  // Comms*** Reg (0x4)
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x040018);
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);

  // channel_enable bits: 87654321 1=on, 0=off 
  int channel_enable = 0b11100011;  // Channels 8,7,6,2,1

  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, (0x0207F8 ^ (channel_enable << 3)));
  digitalWrite(AFE_CE_PIN, 1);

  // Register 1(0x01)
  // shut down ADCs that aren't being used:
  // Bit 1: LVDS output disable (set to 0!)
  // Bits 9->2: ADC Shutdowns
  // Bit 10: partial power down (don't)
  // Bit 11: low-freq noise suppression
  // Bit 15: Single-Ended Clock (set to 1!)
  delay(5);
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x0183FC ^ (channel_enable << 2));  //shut down ADCs. Channel 1 on: 0x0183F8. 1&3: 0x0183E8
  digitalWrite(AFE_CE_PIN, 1);


  // Power*** Reg53(0x35)
  // shut down LNAs not being used (big power savings):
  // Bits 7->0: channels 8 through 1 (Bit 7 = 1, chan 8 Disabled)
  delay(5);
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x3500FF ^ (channel_enable << 0));  //shut down LNAs.
  digitalWrite(AFE_CE_PIN, 1);

  // PGA** Reg50(0x32)
  // Set PGA CLamp (bit 10=1)
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24,0x320000);
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);

 // PGA**  Reg 51(0x33): 
 // Bit 13:Gain (1 24db,0 30db) 
 // Bits 7->5:Clamp (!see above!) 
 // Bit 4:Disable Int. Removal (Disable this integral removal!!!)
 // Bits 3->1:LPF (100=10Mhz, 011=30Mhz)
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x330018); 
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);

  // LNA** Reg 52 (0x34)
  // Bits 5->0: Channel by Channel active termination (don't use!)
  // Bits 7->6: All channel active termination(00->50ohms,11->400ohms).
  // Bit 8: Enable all-channel Active Termination (don't use!)
  // Bits 10->9:Clamp  00: auto, 11:0.6Vpp
  // Bit 11: Must be zero
  // Bit 12: Disable Integrated DC Removal (not a good idea!)
  // Bits 14->13:Gain  00:18 dB  01:24dB 10:12dB 
  // Bit 15: Enable channel-by-channel LNA Gain control (See Reg 57)
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x342000); 
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);

  // LNA** Reg 59 (0x3B hex)
  // Bits 3->2: LNA HPF (01 = 50Khz with Cbypass 15nf)
  // Bits 6->4: Digital Attenuator, 000: 0-dB, 001: -6-dB 111: -42dB
  // Bit 7: Enable dig. atten.
  digitalWrite(AFE_CE_PIN, 0);
  spi_write(AFE_MOSI_PIN, AFE_SCK_PIN, freq, MSBFIRST, SPI_MODE3, 24, 0x3B00C4);  
  digitalWrite(AFE_CE_PIN, 1);
  delay(5);
}


// TE is the sum of RF pulse time and a programmed delaytime.  Sets the delay
// as a multiple the time to complete 1000 Teensy I/0 instrucions.  
// 1000*3.3ns = 3.3us if using 600mhz Teensy clk.
#define SEHALF 100 // 2*400*3.3us =~ 2.7milliseconds

void setup() {

  pinMode(STARTSEQ0, INPUT);
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
  //Serial.begin(115200); //value doesn't matter
  AFE_setup();

  ARM_DEMCR |= ARM_DEMCR_TRCENA;  // info from teensy forum, access cycle counter
  ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;

  pinMode(NOP_PIN, OUTPUT);

  // Simultanously switched Driver pins on GPIO6
  // 16 Pins in use.
  //GPIO6_GDIR = 0x3FFF000C; // GPIO Register 1

  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);

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

  pinMode(38, OUTPUT);
  pinMode(39, OUTPUT);
  pinMode(40, OUTPUT);
  pinMode(41, OUTPUT);

  //GPIO8_GDIR = 0x00C40000;  //GPIO Register 3
  pinMode(28, OUTPUT);  //GPIO8-18  DIN
  pinMode(30, OUTPUT);  //GPIO8-23  DCLK
  pinMode(31, OUTPUT);  //GPIO8-22  NSYNC

  pinMode(READOUT_OLD, OUTPUT);
  pinMode(READOUT, OUTPUT);
  digitalWriteFast(READOUT, false);
  digitalWriteFast(READOUT_OLD, false);

  // Set up clock for AFE
  //pinMode(13, OUTPUT);
  analogWriteResolution(7);
  analogWriteFrequency(13, 50000000);
  analogWrite(13, 64);

  // // Set up the analog inputs for the potentiometer
  analogReadResolution(12);
  pinMode(24, INPUT);
  pinMode(25, INPUT);
  pinMode(26, INPUT);
  pinMode(27, INPUT);
  pinMode(A10, INPUT);
  pinMode(A11, INPUT);
  pinMode(A12, INPUT);
  pinMode(A13, INPUT);

  set_arm_clock(ARM_CLOCK_FREQ);
}

void yield(){};
int gpick = 0;
int count = 0;
int gcount = 0;
//#define CLAMPNUM 100

elapsedMicros loopTimer = 0;  // Loop runs every second.

FASTRUN void loop() {

  //noInterrupts();

  while (true) {

  // Read potentiometers for the next cycle (0->256)
  // And set DAC
    int potRFA = analogRead(27) / 32;    // RF A 0-128
    SETA
    dacVal128(potRFA)
    ENDM
    SETB
    dacVal128(potRFA)
    ENDM

    int potZG = analogRead(26) / 16;     // Z grad.
    boolean negZGrad = (potZG < 128);
    potZG -= 128;
    potZG = abs(potZG);

    int potYG = analogRead(25) / 16;  // Y shares power with RF.
    boolean negYGrad = (potYG < 128);
    potYG -= 128;
    potYG = abs(potYG);

    int potExtra = analogRead(24) / 32;  // Op-amps
    int potExtra2 = analogRead(25) / 32;  // Op-amps
    SETC
    dacVal128(potExtra); // Op-amp
    ENDM
    SETD
    dacVal128(potExtra2); // Op-amp
    ENDM

    CLAMP_STD

  // Decide which scan (gpick == 0: no grad, gpick==1:zgrad, gpick==2:ygrad)
    if (gpick >= 2) gpick = 1;
    else gpick++;

  // Wait to start loop
  #if WAITFORFPGA
      for(volatile int outpin = digitalReadFast(10); outpin == 0; outpin = digitalReadFast(10)){}
  #else
    while (loopTimer <  WAITTIME) {}; // Start loop every second.
    loopTimer = 0;
  #endif

    // Open the write window.
    digitalWriteFast(READOUT, true);
    digitalWriteFast(READOUT_OLD, true);

    CLAMP_STD

    // 90
    //#include "pulsecmd4550126.h"
//#include "pulsecmd453.h"
//TR100
//#include "pulsecmd453.h"
//#include "pulsecmd447.h"
// #include "pulsecmd453.h"
 //#include "pulsecmd453.h"
// //TR1000
//#include "pulsecmd447.h"
//#include "pulsecmd455.h"
#define numP 2
for(int i = numP; i > 0; i--) {
#include "pulsecmd453.h"
}

//#include "pulsecmd455.h"
//#include "pulsecmd455.h"
//#include "pulsecmd455.h"

    CLAMP_STD

    // Delay between 90 and 180
    for (int k = 0; k < 2 * SEHALF; k++) {
      TR1000
    }

    CLAMP_STD
// 180
//nclude "pulsecmd4550126.h"
//#include "pulsecmd453.h"
//TR100
//#include "pulsecmd453.h"
//R100
//#include "pulsecmd453.h"
//TR100
//#include "pulsecmd453.h"
//#include "pulsecmd447.h"
//#include "pulsecmd447.h"
for(int i = numP; i > 0; i--) {
#include "pulsecmd453.h"
}
for(int i = numP; i > 0; i--) {
#include "pulsecmd453.h"
}
//#include "pulsecmd453.h"
//#include "pulsecmd453.h"
//#include "pulsecmd455.h"
//#include "pulsecmd455.h"
////TR1000
// #include "pulsecmd453.h"
// #include "pulsecmd453.h"
// #include "pulsecmd453.h"
// #include "pulsecmd453.h"
//#include "pulsecmd455.h"
//TR1000
//#include "pulsecmd455.h"
//TR1000
//#include "pulsecmd455.h"
//#include "pulsecmd455.h"
//#include "pulsecmd455.h"
//#include "pulsecmd455.h"
//#include "pulsecmd455.h"
// #include "pulsecmd455.h"
// #include "pulsecmd455.h"
// #include "pulsecmd455.h"
//#include "pulsecmd455.h"
//#include "pulsecmd455.h"

// Change the RF supply voltage for Max14808
    if(DOGRAD && (gpick > 0)) {
      SETA
      dacVal128(potZG)
      ENDM
      CLAMP_STD
      SETB
      dacVal128(potYG)
      ENDM
    }
    CLAMP_STD

    int sehalfH = SEHALF;
    if ((!DOGRAD) || (gpick == 0)) {
      for (int k = 0; k < sehalfH; k++) {  // Half of Spin Echo Delay
        TR1000
      }
      for (int k = 0; k < 2 * sehalfH; k++) {  // Spin echo +/- Half delay
        TR1000
      }
      for (int k = 0; k < sehalfH; k++) {  // Half Spin Echo Delay
        TR1000
      }
    } else if (gpick == 1) {  // Zgrad assume in Vout7+Vout8
      if (negZGrad) {
        for (int k = 0; k < sehalfH; k++) {  // Spin Echo Delay
          ZGN1000
        }
        for (int k = 0; k < 2 * sehalfH; k++) {
          ZGP1000
        }
        for (int k = 0; k < sehalfH; k++) {  // Spin Echo Delay
          ZGN1000
        }
      } else {
        for (int k = 0; k < sehalfH; k++) {  // Spin Echo Delay
          ZGP1000
        }
        for (int k = 0; k < 2 * sehalfH; k++) {
          ZGN1000
        }
        for (int k = 0; k < sehalfH; k++) {  // Spin Echo Delay
          ZGP1000
        }
      }
    } else {  // gpick == 2, do y grad.
      if (negYGrad) {
        for (int k = 0; k < sehalfH; k++) {  // Spin Echo Delay
          YGN1000
        }
        for (int k = 0; k < 2 * sehalfH; k++) {
          YGP1000
        }
        for (int k = 0; k < sehalfH; k++) {  // Spin Echo Delay
          YGN1000
        }
      } else {
        for (int k = 0; k < sehalfH; k++) {  // Spin Echo Delay
          YGP1000
        }
        for (int k = 0; k < 2 * sehalfH; k++) {
          YGN1000
        }
        for (int k = 0; k < sehalfH; k++) {  // Spin Echo Delay
          YGP1000
        }
      }
    }  // End of Y gradient

    // Close Readout window
    digitalWriteFast(READOUT, false);
    digitalWriteFast(READOUT_OLD, false);

    //  Post readout clamp
    CLAMP_STD

    // Don't allow loop to repeat faster than once a second.
    delayMicroseconds(900000);
  }

  interrupts();
}
