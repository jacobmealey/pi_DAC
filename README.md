# Raspberry Pi DAC 

This driver is build and tested on a raspberry pi 3 using version 5.10.14-v7+ of the linux kernel. there is one helper script that simply recompiles the module then reloads it. to run it run `./reload_mod`

to compile the helper script run `gcc test_dac.c -lm -o test_dac`, this will create an executable called `test_dac` this program writes to sin waves to the dac. 

## Writing to the DAC
Though the DAC can be opened like a normal file you must do a few IOCTL operations before you can properly write to it.

- First you must enable the dac, this is done with: `ioctl(fd, DAC_IOEN)`
- Then you must set a frequency (the time between writes to the dac): `ioctl(fd, DAC_IOSF, 1000)` to dwrite to the dac every millisecond, note there is some variance as it takes to to actually write each byte.
- Finally, you must disable the dac when done: `ioctl(fd, DAC_IODE)`.

to get a better context, look in dac_test.c

