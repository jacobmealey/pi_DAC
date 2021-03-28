#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int main() {
	char * test = "HELLO WORLD";
	int fd = open("/dev/dac", O_WRONLY);

	ioctl(fd, 1);

	ioctl(fd, 3, 100);

	if(fd){
		printf("%d", write(fd, test, 11));
		close(fd);
	}	

	ioctl(fd, 2);

	return 0;
}
