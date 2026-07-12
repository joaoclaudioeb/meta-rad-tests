#include <stdio.h>

#include "device-drivers/dac81408.h"

int main(void) {
  dac81408_t dev = {0};

  dac81408_init(&dev, "/dev/spidev1.0", "gpiochip0", 0U, DAC81408_PIN_UNUSED);

  dac81408_config(&dev);

  uint16_t pwdwn = dac81408_read_register(&dev, 0x09);  // DACPWDWN
  printf("DACPWDWN: 0x%04X (esperado 0x0000 se todos ativos)\n", pwdwn);

  uint16_t dev_id = dac81408_read_register(&dev, DAC81408_REG_DEVICEID);
  printf("Device ID: 0x%x\n", dev_id);

  dac81408_set_int_reference(&dev, DAC81408_REF_ON);

  dac81408_set_ch_enabled(&dev, 0, true);
  dac81408_set_ch_enabled(&dev, 1, true);
  dac81408_set_ch_enabled(&dev, 2, true);
  dac81408_set_ch_enabled(&dev, 3, true);
  dac81408_set_ch_enabled(&dev, 4, true);
  dac81408_set_ch_enabled(&dev, 5, true);
  dac81408_set_ch_enabled(&dev, 6, true);
  dac81408_set_ch_enabled(&dev, 7, true);

  printf("Ch[%d]: %d/n", 0, dac81408_get_ch_enabled(&dev, 0));
  printf("Ch[%d]: %d/n", 1, dac81408_get_ch_enabled(&dev, 1));
  printf("Ch[%d]: %d/n", 2, dac81408_get_ch_enabled(&dev, 2));
  printf("Ch[%d]: %d/n", 3, dac81408_get_ch_enabled(&dev, 3));
  printf("Ch[%d]: %d/n", 4, dac81408_get_ch_enabled(&dev, 4));
  printf("Ch[%d]: %d/n", 5, dac81408_get_ch_enabled(&dev, 5));
  printf("Ch[%d]: %d/n", 6, dac81408_get_ch_enabled(&dev, 6));
  printf("Ch[%d]: %d/n", 7, dac81408_get_ch_enabled(&dev, 7));

  dac81408_set_range(&dev, 0, DAC81408_RANGE_0_5V);
  dac81408_set_range(&dev, 1, DAC81408_RANGE_0_5V);
  dac81408_set_range(&dev, 2, DAC81408_RANGE_0_5V);
  dac81408_set_range(&dev, 3, DAC81408_RANGE_0_5V);
  dac81408_set_range(&dev, 4, DAC81408_RANGE_0_5V);
  dac81408_set_range(&dev, 5, DAC81408_RANGE_0_5V);
  dac81408_set_range(&dev, 6, DAC81408_RANGE_0_5V);
  dac81408_set_range(&dev, 7, DAC81408_RANGE_0_5V);

  /* dac81408_set_out(&dev, 7, 39321); */
  /* dac81408_set_out(&dev, 3, 65000); */
  /* dac81408_set_out(&dev, 5, 10000); */

  dac81408_set_out(&dev, 0, 39321);
  dac81408_set_out(&dev, 1, 39321);
  dac81408_set_out(&dev, 2, 39321);
  dac81408_set_out(&dev, 3, 39321);
  dac81408_set_out(&dev, 4, 39321);
  dac81408_set_out(&dev, 5, 39321);
  dac81408_set_out(&dev, 6, 39321);
  dac81408_set_out(&dev, 7, 39321);

  /* for(int i = 0; i < 16; i++) { */
  /*     dac81408_set_out(&dev, 0, 4369*i); */
  /*     usleep(100000); */
  /* } */

  return 0;
}
