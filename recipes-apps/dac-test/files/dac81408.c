/**
 * @file dac81408.c
 * @brief Source file for controlling TI's 16-bit DAC, the DAC81408.
 * @details This code was based on FelipeJuliano24's one
 * (https://github.com/FelipeJuliano24/DAC81408_lib/blob/main/dac81408.c).
 * @author João Cláudio Elsen Barcellos
 * @version 0.0.1
 * @date 04/04/2026
 */

#include "dac81408.h"
#include <time.h>
#include <unistd.h>

static void dac_sleep_ms(uint32_t ms) {
  struct timespec ts;

  ts.tv_sec = 0U;
  ts.tv_nsec = ms * 1000000UL;

  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
}

static void dac_sleep_us(uint32_t us) {
  struct timespec ts;

  ts.tv_sec = 0U;
  ts.tv_nsec = us * 1000UL;

  clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
}

static inline void _dac81408_tcsh_delay(void) {
  dac_sleep_us(1); // minimum tCSH from datasheet
}

static inline void _dac81408_tldac_delay(void) {
  dac_sleep_us(1); // minimum tLDAC from datasheet
}

void dac81408_init(dac81408_t *dev, uint8_t ldac_pin, uint8_t reset_pin,
                   uint8_t cs_pin, uint8_t miso_pin, uint8_t sclk_pin,
                   uint8_t mosi_pin) {
  if (reset_pin != -1) {
    dev->reset_pin = reset_pin;
    rst_init(dev);
    rst_set(dev);
  }
  /* Main registers default values */
  dev->dacrange0_cache = 0x0000;
  dev->dacrange1_cache = 0x0000;
  dev->spiconfig_cache = 0x0AA4;
  dev->genconfig_cache = 0x7F00;
  dev->syncconfig_cache = 0x0000;
  dev->dacpwdwn_cache = 0xFFFF;
}

int dac81408_config(dac81408_t *dev) {
  if (dev->reset_pin != -1) {
    rst_clear(dev);
    dac_sleep_ms(150);
    rst_set(dev);
    dac_sleep_ms(150);
  }

  uint16_t spiconfig = DAC81408_TEMPALM_EN(0) | DAC81408_DACBUSY_EN(0) |
                       DAC81408_CRCALM_EN(0) | DAC81408_SOFTTOGGLE_EN(0) |
                       DAC81408_DEV_PWDWN(0) | DAC81408_CRC_EN(0) |
                       DAC81408_STR_EN(0) | DAC81408_SDO_EN(1) |
                       DAC81408_FSDO(0) | ((0x01) << 7);
  dac81408_write_register(dev, DAC81408_REG_SPICONFIG, spiconfig);

  dev->spiconfig_cache = dac81408_read_register(dev, DAC81408_REG_SPICONFIG);

  return (dev->spiconfig_cache == spiconfig) ? 0 : -1;
}

void dac81408_write_register(dac81408_t *dev, uint8_t reg, uint16_t wdata) {
  uint8_t cmd = (DAC81408_WREG << 7) | (reg & 0x3F);
  uint8_t msb = (wdata >> 8) & 0xFF;
  uint8_t lsb = wdata & 0xFF;

  dac_spi_write(dev, &cmd, 1);
  dac_spi_write(dev, &msb, 1);
  dac_spi_write(dev, &lsb, 1);
  _dac81408_tcsh_delay();
}

uint16_t dac81408_read_register(dac81408_t *dev, uint8_t reg) {
  uint8_t cmd = (DAC81408_RREG << 7) | (reg & 0x3F);
  uint8_t dummy = 0x00;

  dac_spi_write(dev, &cmd, 1);
  dac_spi_write(dev, &dummy, 1);
  dac_spi_write(dev, &dummy, 1);
  _dac81408_tcsh_delay();

  _dac81408_tcsh_delay();

  uint8_t buf[3];
  dac_spi_read(dev, &buf[0], 1);
  dac_spi_read(dev, &buf[1], 1);
  dac_spi_read(dev, &buf[2], 1);
  _dac81408_tcsh_delay();

  return ((buf[1] << 8) | buf[2]);
}

void dac81408_set_ch_enabled(dac81408_t *dev, int ch, bool state) {
  if (ch < 0 || ch > 7)
    return;

  uint16_t dacpwdwn = dac81408_read_register(dev, DAC81408_REG_DACPWDWN);
  uint16_t mask = 1 << (ch + 4);

  if (state) {
    dac81408_write_register(dev, DAC81408_REG_DACPWDWN,
                            dacpwdwn & ~mask); // power up (bit = 0)
  } else {
    dac81408_write_register(dev, DAC81408_REG_DACPWDWN,
                            dacpwdwn | mask); // power down (bit = 1)
  }

  dev->dacpwdwn_cache = dac81408_read_register(dev, DAC81408_REG_DACPWDWN);
}

bool dac81408_get_ch_enabled(dac81408_t *dev, int ch) {
  if (ch < 0 || ch > 7)
    return -1;

  uint16_t dacpwdwn = dac81408_read_register(dev, DAC81408_REG_DACPWDWN);

  return !((dacpwdwn >> (ch + 4)) & 1);
}

void dac81408_set_range(dac81408_t *dev, int ch, dac81408_range_t range) {
  if (ch < 0 || ch > 7)
    return;

  uint8_t dacrange_ch = ch & 0x03;
  uint16_t shift = 4 * dacrange_ch;
  uint16_t mask = 0xF << shift;

  if (ch < 4) {
    dev->dacrange0_cache =
        (dev->dacrange0_cache & ~mask) | ((range << shift) & mask);
    dac81408_write_register(dev, DAC81408_REG_DACRANGE0, dev->dacrange0_cache);
  } else {
    dev->dacrange1_cache =
        (dev->dacrange1_cache & ~mask) | ((range << shift) & mask);
    dac81408_write_register(dev, DAC81408_REG_DACRANGE1, dev->dacrange1_cache);
  }
}

int dac81408_get_range(dac81408_t *dev, int ch) {
  if (ch < 0 || ch > 7)
    return -1;

  uint8_t dacrange_ch = ch & 0x3;
  uint16_t shift = 4 * dacrange_ch;

  if (ch < 4) {
    return ((dev->dacrange0_cache >> shift) & 0xF);
  } else {
    return ((dev->dacrange1_cache >> shift) & 0xF);
  }
}

void dac81408_set_out(dac81408_t *dev, int ch, uint16_t val) {
  if (ch < 0 || ch > 7)
    return;

  uint8_t ch_reg = DAC81408_REG_DAC0 + ch;

  dac81408_write_register(dev, ch_reg, val);
}

uint16_t dac81408_get_out(dac81408_t *dev, uint8_t reg) {
  uint16_t val = dac81408_read_register(dev, reg);

  return val;
}

/* Functions that do not work or were not tested yet */
void dac81408_set_int_reference(dac81408_t *dev, dac81408_ref_state_t state) {
  uint16_t mask_preserve = 0x3C7F;
  uint16_t ref_bit = (state == DAC81408_REF_OFF) ? (1 << 14) : 0;

  dev->genconfig_cache = (dev->genconfig_cache & mask_preserve) | ref_bit;
  dac81408_write_register(dev, DAC81408_REG_GENCONFIG, dev->genconfig_cache);
}

int dac81408_get_int_reference(dac81408_t *dev) {
  uint16_t res = dac81408_read_register(dev, DAC81408_REG_GENCONFIG);
  return (res & (1 << 14)) ? DAC81408_REF_OFF : DAC81408_REF_ON;
}

void dac81408_set_sync(dac81408_t *dev, int ch, dac81408_sync_t mode) {
  if (ch < 0 || ch > 7)
    return;

  if (mode == DAC81408_SYNC_LDAC) {
    dev->syncconfig_cache |= (1UL << (ch + 4));
  } else {
    dev->syncconfig_cache &= ~(1UL << (ch + 4));
  }
  dac81408_write_register(dev, DAC81408_REG_SYNCCONFIG, dev->syncconfig_cache);
}

void dac81408_trigger_ldac(dac81408_t *dev) {
  if (dev->ldac_pin == 0)
    return;

  // gpio_put(dev->ldac_pin, 0);
  _dac81408_tldac_delay();
  // pio_put(dev->ldac_pin, 1);
}
