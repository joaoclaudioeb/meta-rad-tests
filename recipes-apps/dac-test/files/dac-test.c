#include "dac81408.h"

#include <stdio.h>
int main() {
  dac81408_t dev = {
      .spidev_path = "/dev/spidev2.0",
  };

  dac81408_init(&dev, 1U, 0U, 0U, 0U, 0U, 0U);

  dac81408_write_register(&dev, DAC81408_REG_SPICONFIG, 0x0084);

  uint16_t dev_id = dac81408_read_register(&dev, DAC81408_REG_DEVICEID);

  printf("Device ID: %#x", dev_id);

  return 0;
}
