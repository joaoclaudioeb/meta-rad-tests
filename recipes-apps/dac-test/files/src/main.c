#include "dac81408.h"

#include <stdio.h>

int main(void) {
    dac81408_t dev = {0};

    dac81408_init(&dev,
                  "/dev/spidev2.0",
                  "gpiochip0",
                  0U,
                  1U);

    dac81408_write_register(&dev, DAC81408_REG_SPICONFIG, 0x0084);

    uint16_t dev_id = dac81408_read_register(&dev, DAC81408_REG_DEVICEID);

    printf("Device ID: %#x\n", dev_id);

    return 0;
}
