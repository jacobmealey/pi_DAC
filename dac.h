/* A. Sheaff 3/5/2020
 * DAC converter using GPIO pins.  8 bits
 */
#ifndef DAC_H
#define DAC_H

#include <linux/types.h>
#include <asm/ioctl.h>

// Magic number

// IOCTL commands for Enable, Disable, and setting the sample frequency.
#define DAC_EN 1 // Dac Enable - for turning on the DAC
#define DAC_DE 2 // Dac Disable - for turning off the DAC
#define DAC_SD 3 // Dac Set Delay - for setting the delay between 
		 // writes in the buffer

#define MAX_DELAY 1000000 // MAX delay is one second.

#endif // DAC_H
