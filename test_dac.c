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
void write_to_dac(char * buff, int size, int DEL);

int main() 
{
	if(fork() == 0){
		char * buff = NULL;
		generate_sin_buffer(&buff, 2, 200);
		write_to_dac(buff, 2*200, 1);	
		printf("child\n");
	}else{
		char * buff = NULL;
		generate_sin_buffer(&buff, 5, 256);
		write_to_dac(buff, 5*255, 10);
	}

	return 0;
}

// An abstracting function for wrint to the DAC,
void write_to_dac(char * buff, int size, int DEL){
	printf("%d", buff[0]);

	int fd = open("/dev/dac", O_WRONLY);

	if(fd){
		ioctl(fd, DAC_EN);
		ioctl(fd, DAC_SD, DEL);
		printf("%d", write(fd, buff, size));
		ioctl(fd, DAC_DE);
		close(fd);
	}
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
