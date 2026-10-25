#include "bitbang_spi.h"
#include <Arduino.h>
#include <SPI.h>

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
      while (micros() - start_time < period);
    } else {
      while (micros() - start_time < period);
      digitalWrite(clock_pin, sck);
    }

    // Toggle clock trailing edge
    start_time = micros();
    sck = !sck;
    if (cpha) {
      digitalWrite(clock_pin, sck);
      while (micros() - start_time < period);
    } else {
      while (micros() - start_time < period);
      digitalWrite(clock_pin, sck);
    }
  }
}