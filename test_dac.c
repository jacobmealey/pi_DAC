#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include "dac.h"

#define RESOLUTION 100

/*
 * buff is the buffer to write to
 * cycles is the amount of complete sin wave periods to generate
 * and resolution is the amount of resolution is the amount 
 * a cycle is divided into.
 */
void generate_sin_buffer(char **buff, int cycles, int resolution);
int write_to_dac(char *buff, int size, int DEL);

int main(){
	if (fork() == 0) {
		char *buff = NULL;
		generate_sin_buffer(&buff, 2, 200);
		if(!write_to_dac(buff, 2 * 200, 1000))
			printf("Test 1 passed correctly\n");
	} else {
		char *buff = NULL;
		generate_sin_buffer(&buff, 5, 256);
		if(!write_to_dac(buff, 5 * 255, 500000))
			printf("Test 2 passed correctly\n");
	}
	return 0;
}

// An abstracting function for wrint to the DAC,
int write_to_dac(char *buff, int size, int DEL)
{
	int ret = 0;

	printf("%d", buff[0]);

	int fd = open("/dev/dac", O_WRONLY);
	if(fd < 0){
		ret = fd;
		goto end_block;
	}

	// check if its  opened 
	if((ret = ioctl(fd, DAC_IOEN)) < 0) goto end_block;
	if((ret = ioctl(fd, DAC_IOSF, DEL)) < 0) goto end_block;
	if((ret = ioctl(fd, DAC_IODE)) < 0) goto end_block;
	close(fd);
	return ret;

end_block:
	close(fd);
	return ret;

}

void generate_sin_buffer(char **buff, int cycles, int resolution)
{
	free(*buff);
	int buff_size = cycles * resolution;

	*buff = malloc(buff_size * sizeof(char));
	if (*buff == NULL) {
		printf("Could not allocate :(\n");
		return;
	}

	for (int i = 0; i < buff_size; i++) {
		*(*buff + i) = sin(i * (M_PI / 180.0)) * 255;
	}
}
