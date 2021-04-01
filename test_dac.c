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
void generate_sin_buffer(char ** buff, int cycles, int resolution);

int main() 
{
	char * buff = NULL;
	generate_sin_buffer(&buff, 2, RESOLUTION);
	printf("%d", buff[0]);

	int fd = open("/dev/dac", O_WRONLY);

	if(fd){
		ioctl(fd, DAC_EN);
		ioctl(fd, DAC_SD, 100);
		printf("%d", write(fd, buff, 2*RESOLUTION));
		ioctl(fd, DAC_DE);
		close(fd);
	}	


	return 0;
}

void generate_sin_buffer(char ** buff, int cycles, int resolution)
{
	free(*buff);
	int buff_size = cycles * resolution;
	
	*buff = malloc(buff_size * sizeof(char));
	if(*buff== NULL){
		printf("Could not allocate :(\n");
		return;
	}


	for(int i = 0; i < buff_size; i++){
		*(*buff + i) = sin(i * (M_PI/180.0))*255;
	}

}
