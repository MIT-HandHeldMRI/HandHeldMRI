//bitbang_spi.h
#pragma once

#include <stdint.h>

void spi_write(uint8_t data_pin, uint8_t clock_pin, uint32_t freq, uint8_t bit_order,uint8_t mode, uint8_t bits, uint32_t val_out);