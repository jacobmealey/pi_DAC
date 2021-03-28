// A. Sheaff 3/5/2020
// DAC converter using GPIO pins.  8 bits
#ifndef DAC_H
#define DAC_H

#include <linux/types.h>
#include <asm/ioctl.h>

// Magic number

// IOCTL commands for Enable, Disable, and setting the sample frequency.
#define DAC_EN 1 
#define DAC_DE 2
#define DAC_SF 3

#endif	// DAC_H
