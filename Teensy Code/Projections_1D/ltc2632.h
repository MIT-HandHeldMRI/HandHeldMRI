// ltc2632.h
#include <stdint.h>
#pragma once

#define DACA 0
#define DACB 1

#define DAC_CLK 3
#define DAC_NCS 4
#define DAC_SDI 2

#define LTC2632_SPI_FREQ 2000000

#define DAC_NODATA 0

typedef enum ltc2632_cmd {
	DAC_WRITE_REG = (0b0000 << 4),
	DAC_UPDATE_REG = (0b0001 << 4),
	DAC_WRITE_REG_UPDATE_ALL = (0b0010 << 4),
	DAC_WRITE_UPDATE_REG = (0b0011 << 4),
	DAC_PWR_DOWN_N = (0b0100 << 4),
	DAC_PRW_DWN = (0b0101 << 4),
	DAC_PWR_UP_REF = (0b0110 << 4),
	DAC_PR_DWN_REF = (0b0111 << 4),
	DAC_NOOP = (0b1000 << 4)
} ltc2632_cmd;

typedef enum ltc2632_addr {
	DAC_A = (0b0000),
	DAC_B = (0b0001),
	DAC_ALL = (0b1111)
} ltc2632_addr;

void DAC_Setup();
void dac_write(uint16_t cmd, uint16_t addr, uint8_t amplitude);

