/**
@file dac81408.h
@brief Header file for controlling TI's 16-bit DAC, the DAC81408.
@details This code was based on FelipeJuliano24's one
(https://github.com/FelipeJuliano24/DAC81408_lib/blob/main/dac81408.h).
@author João Cláudio Elsen Barcellos
@version 0.0.1
@date 04/04/2026
*/

#ifndef DAC81408_H_
#define DAC81408_H_

#include <cstdint>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <gpiod.h>

#define DAC81408_REG_NOP 0x00
#define DAC81408_REG_DEVICEID 0x01
#define DAC81408_REG_STATUS 0x02
#define DAC81408_REG_SPICONFIG 0x03
#define DAC81408_REG_GENCONFIG 0x04
#define DAC81408_REG_BRDCONFIG 0x05
#define DAC81408_REG_SYNCCONFIG 0x06
#define DAC81408_REG_TOGGCONFIG0 0x07
#define DAC81408_REG_TOGGCONFIG1 0x08
#define DAC81408_REG_DACPWDWN 0x09
#define DAC81408_REG_DACRANGE0 0x0B
#define DAC81408_REG_DACRANGE1 0x0C
#define DAC81408_REG_TRIGGER 0x0E
#define DAC81408_REG_BRDCAST 0x0F
#define DAC81408_REG_DAC0 0x14
#define DAC81408_REG_DAC1 0x15
#define DAC81408_REG_DAC2 0x16
#define DAC81408_REG_DAC3 0x17
#define DAC81408_REG_DAC4 0x18
#define DAC81408_REG_DAC5 0x19
#define DAC81408_REG_DAC6 0x1A
#define DAC81408_REG_DAC7 0x1B
#define DAC81408_REG_OFFSET0 0x21
#define DAC81408_REG_OFFSET1 0x22

/* Bits 15-12 are reserved (pg. 35, table 12) */
#define DAC81408_TEMPALM_EN(x) ((x) << 11)
#define DAC81408_DACBUSY_EN(x) ((x) << 10)
#define DAC81408_CRCALM_EN(x) ((x) << 9)
/* Bits 8-7 are reserved (pg. 35, table 12) */
#define DAC81408_SOFTTOGGLE_EN(x) ((x) << 6)
#define DAC81408_DEV_PWDWN(x) ((x) << 5)
#define DAC81408_CRC_EN(x) ((x) << 4)
#define DAC81408_STR_EN(x) ((x) << 3)
#define DAC81408_SDO_EN(x) ((x) << 2)
#define DAC81408_FSDO(x) ((x) << 1)
/* Bit 0 is reserved (pg. 35, table 12) */

#define DAC81408_REF_PWDWN(x) ((x) << 14)
#define DAC81408_DIFF_EN(x, ch) ((x) << (ch + 2))

typedef enum { DAC81408_WREG = 0, DAC81408_RREG = 1 } dac81408_cmd_t;

typedef enum {
  DAC81408_RANGE_0_5V = 0x0,        // 0000: 0 to 5 V
  DAC81408_RANGE_0_10V = 0x1,       // 0001: 0 to 10 V
  DAC81408_RANGE_0_20V = 0x2,       // 0010: 0 to 20 V
  DAC81408_RANGE_0_40V = 0x4,       // 0100: 0 to 40 V
  DAC81408_RANGE_BIPOLAR_5V = 0x9,  // 1001: -5 V to +5 V
  DAC81408_RANGE_BIPOLAR_10V = 0xA, // 1010: -10 V to +10 V
  DAC81408_RANGE_BIPOLAR_20V = 0xC, // 1100: -20 V to +20 V
  DAC81408_RANGE_BIPOLAR_2V5 = 0xE  // 1110: -2.5 V to +2.5 V
} dac81408_range_t;

typedef enum {
  DAC81408_SYNC_ASYNC = 0, // Asynchronous mode (immediate update)
  DAC81408_SYNC_LDAC = 1   // Synchronous mode (LDAC trigger)
} dac81408_sync_t;

typedef enum {
  DAC81408_POWER_DOWN = 0,
  DAC81408_POWER_UP = 1
} dac81408_power_t;

typedef enum { DAC81408_REF_OFF = 0, DAC81408_REF_ON = 1 } dac81408_ref_state_t;

typedef enum { DAC81408_CH_OFF = 0, DAC81408_CH_ON = 1 } dac81408_ch_state_t;

struct dac81408 {
  // GPIOs and SPI pins
  uint8_t cs_pin;
  uint8_t reset_pin;
  uint8_t ldac_pin;
  uint8_t miso_pin;
  uint8_t sclk_pin;
  uint8_t mosi_pin;

  // SPI instance (spi0 or spi1)
  // void *spi_instance;
  const char *spidev_path;
  struct gpiod_chip *chip;
  struct gpiod_line *rst_line;

  // Register cache
  uint16_t spiconfig_cache;
  uint16_t genconfig_cache;
  uint16_t dacrange0_cache;
  uint16_t dacrange1_cache;
  uint16_t syncconfig_cache;
  uint16_t dacpwdwn_cache;

  // Internal state
  bool is_initialized;
};

typedef struct dac81408 dac81408_t;

void dac81408_init(dac81408_t *dev, uint8_t ldac_pin, uint8_t reset_pin,
                   uint8_t cs_pin, uint8_t miso_pin, uint8_t sclk_pin,
                   uint8_t mosi_pin);

int dac81408_config(dac81408_t *dev);
void dac81408_write_register(dac81408_t *dev, uint8_t reg, uint16_t wdata);
uint16_t dac81408_read_register(dac81408_t *dev, uint8_t reg);
void dac81408_set_ch_enabled(dac81408_t *dev, int ch, bool state);
bool dac81408_get_ch_enabled(dac81408_t *dev, int ch);
void dac81408_set_int_reference(dac81408_t *dev, dac81408_ref_state_t state);
int dac81408_get_int_reference(dac81408_t *dev);
void dac81408_set_range(dac81408_t *dev, int ch, dac81408_range_t range);
int dac81408_get_range(dac81408_t *dev, int ch);
void dac81408_set_out(dac81408_t *dev, int ch, uint16_t val);
uint16_t dac81408_get_out(dac81408_t *dev, uint8_t reg);
void dac81408_set_sync(dac81408_t *dev, int ch, dac81408_sync_t mode);
void dac81408_trigger_ldac(dac81408_t *dev);

static inline int rst_init(dac81408_t *config) {
  config->chip = gpiod_chip_open_by_name("gpiochip0");

  if (!config->chip) {
    perror("open gpiod chip rst");
    return -1;
  }

  config->rst_line = gpiod_chip_get_line(config->chip, config->reset_pin);

  if (!config->rst_line) {
    perror("get gpiod line rst");
    gpiod_chip_close(config->chip);
    return -1;
  }

  /* Request line as output, default LOW */
  int ret = gpiod_line_request_output(config->rst_line, "dac-rst", 1);
  if (ret < 0) {
    perror("request gpiod line output rst");
    gpiod_chip_close(config->chip);
    return -1;
  }

  return 0;
}

static inline int rst_set(dac81408_t *config) {
  if (!config->chip || !config->rst_line)
    return -1;

  gpiod_line_set_value(config->rst_line, 1);

  return 0;
}

static inline int rst_clear(dac81408_t *config) {
  if (!config->chip || !config->rst_line)
    return -1;

  gpiod_line_set_value(config->rst_line, 0);

  return 0;
}

static inline int dac_spi_transfer(dac81408_t *config, uint8_t *wdata,
                                   uint8_t *rdata, uint16_t len) {
  int ret = 0;

  int fd = open(config->spidev_path, O_RDWR);

  if (fd < 0) {
    perror("open SPI devices");
    return -1;
  }

  int mode = SPI_MODE_0;

  ioctl(fd, SPI_IOC_WR_MODE, &mode);

  struct spi_ioc_transfer tr = {
      .tx_buf = (unsigned long)wdata,
      .rx_buf = (unsigned long)rdata,
      .len = len,
      .delay_usecs = 0,
      .speed_hz = 1000000,
      .bits_per_word = 8,
  };

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

  if (ret < 0) {
    perror("SPI tranfer!");
  }

  if (ret == len) {
    ret = 0;
  } else {
    ret = -1;
  }

  close(fd);

  return ret;
}

static inline int dac_spi_write(dac81408_t *config, uint8_t *data,
                                uint16_t len) {
  uint8_t rbuf[255] = {0};

  return dac_spi_transfer(config, data, rbuf, len);
}

static inline int dac_spi_read(dac81408_t *config, uint8_t *data,
                               uint16_t len) {
  uint8_t wbuf[255] = {0};

  return dac_spi_transfer(config, wbuf, data, len);
}

#define SPI_TXN_MAX_TRANSFERS 16

struct spi_txn {
  struct spi_ioc_transfer xfers[SPI_TXN_MAX_TRANSFERS];
  unsigned int count;
  unsigned int bound;
};

static inline int spi_txn_prepare(struct spi_txn *txn, unsigned int count) {
  if (count == 0 || count > SPI_TXN_MAX_TRANSFERS)
    return -EINVAL;

  memset(txn, 0, sizeof(*txn));
  txn->count = count;
  txn->bound = 0;

  return 0;
}

static inline int spi_txn_bind_transfer(struct spi_txn *txn, unsigned int index,
                                        const uint8_t *tx, uint8_t *rx,
                                        uint32_t len) {
  if (index >= txn->count || len == 0)
    return -EINVAL;

  txn->xfers[index].tx_buf = (unsigned long)tx;
  txn->xfers[index].rx_buf = (unsigned long)rx;
  txn->xfers[index].len = len;
  txn->xfers[index].cs_change = 0;
  txn->bound++;

  return 0;
}

static inline int spi_txn_bind_write(struct spi_txn *txn, unsigned int index,
                                     const uint8_t *tx, uint32_t len) {
  if (index >= txn->count || len == 0)
    return -EINVAL;

  txn->xfers[index].tx_buf = (unsigned long)tx;
  txn->xfers[index].rx_buf = (unsigned long)NULL;
  txn->xfers[index].len = len;
  txn->xfers[index].cs_change = 0;
  txn->bound++;

  return 0;
}

static inline int spi_txn_bind_read(struct spi_txn *txn, unsigned int index,
                                    uint8_t *rx, uint32_t len) {
  if (index >= txn->count || len == 0)
    return -EINVAL;

  txn->xfers[index].tx_buf = (unsigned long)NULL;
  txn->xfers[index].rx_buf = (unsigned long)rx;
  txn->xfers[index].len = len;
  txn->xfers[index].cs_change = 0;
  txn->bound++;

  return 0;
}

static inline int spi_txn_set_speed(struct spi_txn *txn, unsigned int index,
                                    uint32_t speed_hz) {
  if (index >= txn->count)
    return -EINVAL;

  txn->xfers[index].speed_hz = speed_hz;

  return 0;
}

static inline int spi_txn_set_delay(struct spi_txn *txn, unsigned int index,
                                    uint16_t delay_us) {
  if (index >= txn->count)
    return -EINVAL;

  txn->xfers[index].delay_usecs = delay_us;

  return 0;
}

static inline int spi_txn_set_bits(struct spi_txn *txn, unsigned int index,
                                   uint8_t bits_per_word) {
  if (index >= txn->count)
    return -EINVAL;

  txn->xfers[index].bits_per_word = bits_per_word;

  return 0;
}

static inline int spi_txn_execute(struct spi_txn *txn, int fd) {
  int ret;

  if (txn->bound != txn->count)
    return -EINVAL;

  ret = ioctl(fd, SPI_IOC_MESSAGE(txn->count), txn->xfers);
  if (ret < 0)
    return -errno;

  return ret;
}

static inline void spi_txn_finalize(struct spi_txn *txn) {
  memset(txn, 0, sizeof(*txn));
}

static inline void spi_txn_reset(struct spi_txn *txn) {
  unsigned int count = txn->count;

  memset(txn->xfers, 0, sizeof(txn->xfers[0]) * count);
  txn->bound = 0;
}

static inline int spi_txn_open(const char *path) {
  int fd;

  fd = open(path, O_RDWR);
  if (fd < 0)
    return -errno;

  return fd;
}

static inline int spi_txn_close(int fd) {
  if (close(fd) < 0)
    return -errno;

  return 0;
}

#endif /* DAC81408_H_ */
