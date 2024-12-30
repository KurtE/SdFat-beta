/**
 * Copyright (c) 2011-2022 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "SdSpiDriver.h"
#if defined(SD_USE_CUSTOM_SPI) && defined(ARDUINO_ARCH_ZEPHYR)
#define USE_BLOCK_TRANSFER 1

class wrapped_SPI : public arduino::ZephyrSPI {
  public:
    inline const struct device *SPIDevice() {return spi_dev;}
    inline struct spi_config *getConfig()  {return &config;}
    inline struct spi_config *getConfig16() {return &config16;}
};

//------------------------------------------------------------------------------
void SdSpiArduinoDriver::activate() { m_spi->beginTransaction(m_spiSettings); }
//------------------------------------------------------------------------------
void SdSpiArduinoDriver::begin(SdSpiConfig spiConfig) {
  if (spiConfig.spiPort) {
    m_spi = spiConfig.spiPort;
  } else {
    m_spi = &SPI;
  }
  m_spi->begin();
}
//------------------------------------------------------------------------------
void SdSpiArduinoDriver::deactivate() { m_spi->endTransaction(); }
//------------------------------------------------------------------------------
void SdSpiArduinoDriver::end() { m_spi->end(); }
//------------------------------------------------------------------------------
uint8_t SdSpiArduinoDriver::receive() { return m_spi->transfer(0XFF); }
//------------------------------------------------------------------------------
uint8_t SdSpiArduinoDriver::receive(uint8_t* buf, size_t count) {
#if USE_BLOCK_TRANSFER
  #if 1
    wrapped_SPI *pspi = (wrapped_SPI *)m_spi;
    struct spi_buf sbuf = {.buf = (void*)buf, .len=count};
    struct spi_buf_set buf_set = { .buffers = &sbuf, .count = 1 };

    spi_transceive(pspi->SPIDevice(), pspi->getConfig(), nullptr, &buf_set);
  #else  
  memset(buf, 0XFF, count);
  m_spi->transfer(buf, count);
  #endif
#else   // USE_BLOCK_TRANSFER
  for (size_t i = 0; i < count; i++) {
    buf[i] = m_spi->transfer(0XFF);
  }
#endif  // USE_BLOCK_TRANSFER
  return 0;
}
//------------------------------------------------------------------------------
void SdSpiArduinoDriver::send(uint8_t data) { m_spi->transfer(data); }
//------------------------------------------------------------------------------
void SdSpiArduinoDriver::send(const uint8_t* buf, size_t count) {
#if USE_BLOCK_TRANSFER
#if 1
    wrapped_SPI *pspi = (wrapped_SPI *)m_spi;
    struct spi_buf sbuf = {.buf = (void*)buf, .len=count};
    struct spi_buf_set buf_set = { .buffers = &sbuf, .count = 1 };

    spi_transceive(pspi->SPIDevice(), pspi->getConfig(), &buf_set, nullptr);

#else
  uint32_t tmp[128];
  if (0 < count && count <= 512) {
    memcpy(tmp, buf, count);
    m_spi->transfer(tmp, count);
    return;
  }
#endif  
#endif  // USE_BLOCK_TRANSFER
  for (size_t i = 0; i < count; i++) {
    m_spi->transfer(buf[i]);
  }
}
#endif  // defined(SD_USE_CUSTOM_SPI) && defined(__arm__) &&defined(CORE_TEENSY)
