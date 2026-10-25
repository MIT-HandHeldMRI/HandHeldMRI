#include "ltc2632.h"
#include "bitbang_spi.h"
#include <SPI.h>
#include <Arduino.h>

void DAC_Setup(){
  pinMode(DAC_CLK, OUTPUT);
  pinMode(DAC_SDI, OUTPUT);
  pinMode(DAC_NCS, OUTPUT);
  digitalWrite(DAC_NCS, HIGH);

  dac_write(DAC_PWR_UP_REF, DAC_ALL, DAC_NODATA);
}

void dac_write(uint16_t cmd, uint16_t addr, uint8_t amplitude){
  uint32_t data = (((uint32_t) cmd | addr) << 16) | (amplitude << 8);
  digitalWrite(DAC_NCS, LOW);
  spi_write(DAC_SDI, DAC_CLK, LTC2632_SPI_FREQ, MSBFIRST, SPI_MODE3, 24, data);
  digitalWrite(DAC_NCS, HIGH);
}