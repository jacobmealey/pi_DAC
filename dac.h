/* A. Sheaff 3/5/2020
 * DAC converter using GPIO pins.  8 bits
 */
#ifndef DAC_H
#define DAC_H

#include <linux/types.h>
#include <asm/ioctl.h>

// Magic number *cough cough*
#define MAGIC_NUM 0xC00FC00F

// IOCTL commands for Enable, Disable, and setting the sample frequency.
#define DAC_IOEN _IO(MAGIC_NUM, 1) // Dac Enable - for turning on the DAC
#define DAC_IODE _IO(MAGIC_NUM, 2) // Dac Disable - for turning off the DAC
#define DAC_IOSF _IOW(MAGIC_NUM, 3, unsigned long) // Dac Set Delay - for setting the delay betwee

#define MAX_FREQ 1000000 // MAX delay is one second.

#endif // DAC_H
