/**
 * @file ads1256.c
 * @brief Source file for controlling TI's 24-bit ADC, the ADS1256.
 * @details This code was based on CuriousScientist0's one (https://github.com/CuriousScientist0/ADS1256/tree/main/src and https://www.youtube.com/watch?v=GBWJdyjRIdM&t=262s) and on existing kernel drivers for TI's ADCs (e.g., ti-ads124s08.c, ti-ads1015.c).
 * @author Carlos Augusto Porto Freitas
 * @author João Cláudio Elsen Barcellos
 * @version 0.0.1
 * @date 21/04/2026
 */

#include <linux/err.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/slab.h>
#include <linux/sysfs.h>

#include <linux/gpio/consumer.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/sysfs.h>

#include <asm/unaligned.h>

/* Commands */
#define ADS1256_CMD_NOP           0x00
#define ADS1256_CMD_WAKEUP        0xFF
#define ADS1256_CMD_RDATA         0x01
#define ADS1256_CMD_RDATAC        0x03
#define ADS1256_CMD_SDATAC        0x0F
#define ADS1256_CMD_RREG          0x10
#define ADS1256_CMD_WREG          0x50
#define ADS1256_CMD_SELFCAL       0xF0
#define ADS1256_CMD_SELFOCAL      0xF1
#define ADS1256_CMD_SELFGCAL      0xF2
#define ADS1256_CMD_SYSOCAL       0xF3
#define ADS1256_CMD_SYSGCAL       0xF4
#define ADS1256_CMD_SYNC          0xFC
#define ADS1256_CMD_STANDBY       0xFD
#define ADS1256_CMD_RESET         0xFE

/* Registers */
#define ADS1256_REG_STATUS        0x00
#define ADS1256_REG_MUX           0x01
#define ADS1256_REG_ADCON         0x02
#define ADS1256_REG_DRATE         0x03
#define ADS1256_REG_IO            0x04
#define ADS1256_REG_OFC0          0x05
#define ADS1256_REG_OFC1          0x06
#define ADS1256_REG_OFC2          0x07
#define ADS1256_REG_FSC0          0x08
#define ADS1256_REG_FSC1          0x09
#define ADS1256_REG_FSC2          0x0A

/* ADS1256 number of channels, considering single + diff. channels */
#define ADS1256_MAX_CHANNELS      12

enum ads1256_bitorder {
    ADS1256_BITORDER_MSB          = 0,
    ADS1256_BITORDER_LSB          = 1
};

enum ads1256_acal {
    ADS1256_ACAL_DISABLED         = 0,
    ADS1256_ACAL_ENABLED          = 1
};

enum ads1256_buffer {
    ADS1256_BUFFER_DISABLED       = 0,
    ADS1256_BUFFER_ENABLED        = 1
};

/* ADS1256 mux. options */
enum ads1256_mux {
    ADS1256_MUX_DIFF_0_1          = 0x01,
    ADS1256_MUX_DIFF_2_3          = 0x23,
    ADS1256_MUX_DIFF_4_5          = 0x45,
    ADS1256_MUX_DIFF_6_7          = 0x67,
    ADS1256_MUX_SING_0            = 0x0F,
    ADS1256_MUX_SING_1            = 0x1F,
    ADS1256_MUX_SING_2            = 0x2F,
    ADS1256_MUX_SING_3            = 0x3F,
    ADS1256_MUX_SING_4            = 0x4F,
    ADS1256_MUX_SING_5            = 0x5F,
    ADS1256_MUX_SING_6            = 0x6F,
    ADS1256_MUX_SING_7            = 0x7F,
};

/* ADS1256 channels */
enum ads1256_channels {
    ADS1256_CHANNEL_0             = 0,
    ADS1256_CHANNEL_1,
    ADS1256_CHANNEL_2,
    ADS1256_CHANNEL_3,
    ADS1256_CHANNEL_4,
    ADS1256_CHANNEL_5,
    ADS1256_CHANNEL_6,
    ADS1256_CHANNEL_7,
    ADS1256_CHANNEL_0_1,
    ADS1256_CHANNEL_2_3,
    ADS1256_CHANNEL_4_5,
    ADS1256_CHANNEL_6_7,
};

/* ADS1256 PGA */
enum ads1256_pga {
    ADS1256_PGA_1                 = 0x00,
    ADS1256_PGA_2                 = 0x01,
    ADS1256_PGA_4                 = 0x02,
    ADS1256_PGA_8                 = 0x03,
    ADS1256_PGA_16                = 0x04,
    ADS1256_PGA_32                = 0x05,
    ADS1256_PGA_64                = 0x06,
};

/* ADS1256 SPS */
enum ads1256_drate {
    ADS1256_DRATE_30000SPS        = 0xF0,
    ADS1256_DRATE_15000SPS        = 0xE0,
    ADS1256_DRATE_7500SPS         = 0xD0,
    ADS1256_DRATE_3750SPS         = 0xC0,
    ADS1256_DRATE_2000SPS         = 0xB0,
    ADS1256_DRATE_1000SPS         = 0xA1,
    ADS1256_DRATE_500SPS          = 0x92,
    ADS1256_DRATE_100SPS          = 0x82,
    ADS1256_DRATE_60SPS           = 0x72,
    ADS1256_DRATE_50SPS           = 0x63,
    ADS1256_DRATE_30SPS           = 0x53,
    ADS1256_DRATE_25SPS           = 0x43,
    ADS1256_DRATE_15SPS           = 0x33,
    ADS1256_DRATE_10SPS           = 0x23,
    ADS1256_DRATE_5SPS            = 0x13,
    ADS1256_DRATE_2SPS            = 0x03,
};

struct ads1256_private {
    struct spi_device *spi;
    struct gpio_desc *drdy_gpio;
    struct gpio_desc *reset_gpio;
    struct gpio_desc *sync_gpio;
    struct mutex lock;

    /* Cached regs. values */
    u8 status_reg;
    u8 mux_reg;
    u8 adcon_reg;
    u8 drate_reg;
    u8 gpio_reg;

    /* Ref. voltage in millivolts */
    u32 vref_mv;

    u32 buffer[ADS1256_MAX_CHANNELS + sizeof(s64)/sizeof(u32)] __aligned(8);
    u8 data[6] __aligned(IIO_DMA_MINALIGN);
};

#define ADS1256_CHAN(index) {                                                              \
        .type = IIO_VOLTAGE,                                                               \
            .indexed = 1,                                                                  \
            .channel = index,                                                              \
            .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE),       \
            .scan_index = index,                                                           \
            .scan_type = {                                                                 \
                .sign = 's',                                                               \
                .realbits = 24,                                                            \
                .storagebits = 32,                                                         \
            },                                                                             \
            }

#define ADS1256_DIFF_CHAN(index, pos, neg) {                                               \
    .type = IIO_VOLTAGE,                                                                   \
        .indexed = 1,                                                                      \
        .differential = 1,                                                                 \
        .channel = pos,                                                                    \
        .channel2 = neg,                                                                   \
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_SCALE),           \
        .scan_index = index,                                                               \
        .scan_type = {                                                                     \
            .sign = 's',                                                                   \
            .realbits = 24,                                                                \
            .storagebits = 32,                                                             \
        },                                                                                 \
        }

static const struct iio_chan_spec ads1256_channels[] = {
    ADS1256_DIFF_CHAN(ADS1256_CHANNEL_0_1, 0, 1),
    ADS1256_DIFF_CHAN(ADS1256_CHANNEL_2_3, 2, 3),
    ADS1256_DIFF_CHAN(ADS1256_CHANNEL_4_5, 4, 5),
    ADS1256_DIFF_CHAN(ADS1256_CHANNEL_6_7, 6, 7),
    ADS1256_CHAN(ADS1256_CHANNEL_0),
    ADS1256_CHAN(ADS1256_CHANNEL_1),
    ADS1256_CHAN(ADS1256_CHANNEL_2),
    ADS1256_CHAN(ADS1256_CHANNEL_3),
    ADS1256_CHAN(ADS1256_CHANNEL_4),
    ADS1256_CHAN(ADS1256_CHANNEL_5),
    ADS1256_CHAN(ADS1256_CHANNEL_6),
    ADS1256_CHAN(ADS1256_CHANNEL_7),
};

static int ads1256_wait_drdy(struct ads1256_private *priv)
{
    int timeout = 1000;

    while (gpiod_get_value(priv->drdy_gpio)) {
        if (--timeout == 0)
            return -ETIMEDOUT;

        udelay(10);
    }

    return 0;
}

static int ads1256_write_cmd(struct iio_dev *indio_dev, u8 cmd)
{
    struct ads1256_private *priv = iio_priv(indio_dev);
    int ret;

    ret = ads1256_wait_drdy(priv);
    if (ret)
        return ret;

    priv->data[0] = cmd;

    ret = spi_write(priv->spi, &priv->data[0], 1);
    if (ret < 0)
        return ret;

    udelay(7); /* t6 delay (~6.5 us) */

    return 0;
}

static int ads1256_reset(struct iio_dev *indio_dev)
{
    struct ads1256_private *priv = iio_priv(indio_dev);

    if (priv->reset_gpio) {
        gpiod_set_value(priv->reset_gpio, 0);
        mdelay(200);
        gpiod_set_value(priv->reset_gpio, 1);
        mdelay(200);
    } else {
        return ads1256_write_cmd(indio_dev, ADS1256_CMD_RESET);
    }

    return 0;
}

static int ads1256_write_reg(struct iio_dev *indio_dev, u8 reg, u8 data)
{
    struct ads1256_private *priv = iio_priv(indio_dev);
    int ret;

    ret = ads1256_wait_drdy(priv);
    if (ret)
        return ret;
        
    priv->data[0] = ADS1256_CMD_WREG | reg;
    priv->data[1] = 0x0;
    priv->data[2] = data;

    ret = spi_write(priv->spi, &priv->data[0], 3);
    if (ret < 0)
        return ret;

    udelay(7); /* t6 delay (~6.5 us) */
    
    return 0;
}

static int ads1256_read(struct iio_dev *indio_dev)
{
    struct ads1256_private *priv = iio_priv(indio_dev);
    int ret;
    int32_t val;

    /* Defines one SPI message composed of two transfers */
    struct spi_transfer t[4] = {
        {
            .tx_buf = &priv->data[0],
            .len = 1,
            /* .delay_usecs = 4, */
        },
        {
            .tx_buf = &priv->data[1],
            .len = 1,
            /* .delay_usecs = 4, */
        },
        {
            .tx_buf = &priv->data[2],
            .len = 1,
            /* .delay_usecs = 7, /\* t6 delay (~6.5 us) *\/ */
        },
        {
            .tx_buf = &priv->data[3],
            .rx_buf = &priv->data[3],
            .len = 3,
        },
    };

    ret = ads1256_wait_drdy(priv);
    if (ret)
        return ret;

    priv->data[0] = ADS1256_CMD_SYNC;
    priv->data[1] = ADS1256_CMD_WAKEUP;
    priv->data[2] = ADS1256_CMD_RDATA;
    memset(&priv->data[3], ADS1256_CMD_NOP, 3);
    
    ret = spi_sync_transfer(priv->spi, t, ARRAY_SIZE(t));
    if (ret < 0)
        return ret;

    val = ((int32_t)priv->data[3] << 16) |
        ((int32_t)priv->data[4] << 8) |
        (int32_t)priv->data[5];
    if (val & 0x800000)
        val |= 0xFF000000;
               
    return val;
}

static const u8 ads1256_mux_table[] = {
    [ADS1256_CHANNEL_0]   = ADS1256_MUX_SING_0,
    [ADS1256_CHANNEL_1]   = ADS1256_MUX_SING_1,
    [ADS1256_CHANNEL_2]   = ADS1256_MUX_SING_2,
    [ADS1256_CHANNEL_3]   = ADS1256_MUX_SING_3,
    [ADS1256_CHANNEL_4]   = ADS1256_MUX_SING_4,
    [ADS1256_CHANNEL_5]   = ADS1256_MUX_SING_5,
    [ADS1256_CHANNEL_6]   = ADS1256_MUX_SING_6,
    [ADS1256_CHANNEL_7]   = ADS1256_MUX_SING_7,
    [ADS1256_CHANNEL_0_1] = ADS1256_MUX_DIFF_0_1,
    [ADS1256_CHANNEL_2_3] = ADS1256_MUX_DIFF_2_3,
    [ADS1256_CHANNEL_4_5] = ADS1256_MUX_DIFF_4_5,
    [ADS1256_CHANNEL_6_7] = ADS1256_MUX_DIFF_6_7,
};

static int ads1256_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int *val, int *val2, long mask)
{
    struct ads1256_private *priv = iio_priv(indio_dev);
    int ret;

    mutex_lock(&priv->lock);
    
    switch (mask) {
    case IIO_CHAN_INFO_RAW:
        ret = ads1256_write_reg(indio_dev, ADS1256_REG_MUX, ads1256_mux_table[chan->scan_index]);
        if (ret)
            goto out;

        ret = ads1256_read(indio_dev);
        if (ret < 0)
            goto out;

        *val = ret;

        ret = IIO_VAL_INT;
        break;
    case IIO_CHAN_INFO_SCALE:
        *val = priv->vref_mv * 2;
        *val2 = 23;
        ret = IIO_VAL_FRACTIONAL_LOG2;
        break;
    default:
        ret = -EINVAL;
        break;
    }
 out:
    mutex_unlock(&priv->lock);
    return ret;
}

static const struct iio_info ads1256_info = {
    .read_raw = &ads1256_read_raw,
};

static int ads1256_probe(struct spi_device *spi)
{
    struct iio_dev *indio_dev;
    struct ads1256_private *priv;
    int ret;

    /* Allocate iio_dev + private data */
    indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*priv));
    if (!indio_dev)
        return -ENOMEM;

    priv = iio_priv(indio_dev);
    priv->spi = spi;
    spi_set_drvdata(spi, indio_dev);
    mutex_init(&priv->lock);

    /* Request GPIOs */
    priv->drdy_gpio = devm_gpiod_get(&spi->dev, "drdy", GPIOD_IN);
    if (IS_ERR(priv->drdy_gpio))
        return PTR_ERR(priv->drdy_gpio);

    priv->reset_gpio = devm_gpiod_get_optional(&spi->dev, "reset", GPIOD_OUT_HIGH);
    if (IS_ERR(priv->reset_gpio))
        return PTR_ERR(priv->reset_gpio);

    priv->sync_gpio = devm_gpiod_get_optional(&spi->dev, "sync", GPIOD_OUT_HIGH);
    if (IS_ERR(priv->sync_gpio))
        return PTR_ERR(priv->sync_gpio);

    /* Configure VREF */
    ret = device_property_read_u32(&spi->dev, "ti,vref-mv", &priv->vref_mv);
    if (ret) {
        priv->vref_mv = 2500; /* VREF (default): 2.5V (2500mV) */
        dev_dbg(&spi->dev, "ti,vref-mv not found, using default %d\n", priv->vref_mv);
    }

    /* Configure iio_dev */
    indio_dev->name        = "ads1256";
    indio_dev->modes       = INDIO_DIRECT_MODE;
    indio_dev->channels    = ads1256_channels;
    indio_dev->num_channels = ARRAY_SIZE(ads1256_channels);
    indio_dev->info        = &ads1256_info;

    /* Reset device and initialize registers */
    ret = ads1256_reset(indio_dev);
    if (ret)
        return ret;

    /* Configure STATUS register */
    ret = device_property_read_u8(&spi->dev, "ti,status", &priv->status_reg);
    if (ret) {
        priv->status_reg = (ADS1256_BUFFER_ENABLED << 1) | (ADS1256_ACAL_ENABLED   << 2) | (ADS1256_BITORDER_MSB   << 3); /* STATUS (default): BUFEN=1, ACAL=1, ORDER=MSB */ 
        dev_dbg(&spi->dev, "ti,status not found, using default 0x%x\n", priv->status_reg);
    }
    ret = ads1256_write_reg(indio_dev, ADS1256_REG_STATUS, priv->status_reg);
    if (ret)
        return ret;
    
    /* Configure ADCON register */
    ret = device_property_read_u8(&spi->dev, "ti,pga", &priv->adcon_reg);
    if (ret) {
        priv->adcon_reg = ADS1256_PGA_1; /* ADCON (default): PGA=1, CLKOUT=off, SDCS=off */
        dev_dbg(&spi->dev, "ti,pga not found, using default 0x%x\n", priv->adcon_reg);
    }
    ret = ads1256_write_reg(indio_dev, ADS1256_REG_ADCON, priv->adcon_reg);
    if (ret)
        return ret;

    /* Configure DRATE register */
    ret = device_property_read_u8(&spi->dev, "ti,drate", &priv->drate_reg);
    if (ret) {
        priv->drate_reg = ADS1256_DRATE_10SPS; /* DRATE (default): 10 SPS */
        dev_dbg(&spi->dev, "ti,drate not found, using default 10 SPS\n");
    }
    ret = ads1256_write_reg(indio_dev, ADS1256_REG_DRATE, priv->drate_reg);
    if (ret)
        return ret;

    /* Register device */
    return devm_iio_device_register(&spi->dev, indio_dev);
}

static const struct spi_device_id ads1256_id[] = {
    { "ads1256", 0 },
    { }
};
MODULE_DEVICE_TABLE(spi, ads1256_id);

static const struct of_device_id ads1256_of_match[] = {
    { .compatible = "ti,ads1256" },
    { }
};
MODULE_DEVICE_TABLE(of, ads1256_of_match);

static struct spi_driver ads1256_driver = {
    .driver = {
        .name           = "ads1256",
        .of_match_table = ads1256_of_match,
    },
    .probe    = ads1256_probe,
    .id_table = ads1256_id,
};
module_spi_driver(ads1256_driver);

/**
 * TODO:
 * Replace busy-wait in ads1256_wait_drdy() with IRQ-based DRDY handling
 * Replace delay_usecs with delay.value + delay.unit in spi_transfer structs
 * Replace mdelay() with msleep() in ads1256_reset()
 * Emit SYNC+WAKEUP only after MUX change, not before every RDATA
 * Add IIO triggered buffer support
 * Consider mainline submission after above items are addressed
 */

MODULE_AUTHOR("Carlos A. P. Freitas <caftrabalho@gmail.com>");
MODULE_AUTHOR("João C. E. Barcellos <joaoclaudiobarcellos@gmail.com>");
MODULE_DESCRIPTION("TI ADS1256 ADC");
MODULE_LICENSE("GPL v2");
