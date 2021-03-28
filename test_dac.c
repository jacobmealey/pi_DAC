#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
	char * test = "HELLO WORLD";
	int fd = open("/dev/dac", O_APPEND);
	if(fd){
		write(fd, test, sizeof(test));
		close(fd);
	}	

	return 0;
}
