#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define WR_VALUE _IOW('a','a',int32_t*)
#define WR_VALUE_2 _IOW('a','c',int32_t*)
#define BUFFER_LENGTH 256

static char buffer[BUFFER_LENGTH];

int main(int argc, char** argv){
	int res, fd, fd1, input_encoding = 0, output_encoding = 0;

	char koi8r [] = "KOI8-R";
	char cp1251 [] = "cp1251";
	char cp866 [] = "cp866";

	if(strcmp(cp866,argv[1]) == 0)
		input_encoding = 1;
	if(strcmp(koi8r,argv[1]) == 0)
		input_encoding = 2;
	if(strcmp(cp1251,argv[1]) == 0)
		input_encoding = 3;
	if(strcmp(cp866,argv[2]) == 0)
		output_encoding = 1;
	if(strcmp(koi8r,argv[2]) == 0)
		output_encoding = 2;
	if(strcmp(cp1251,argv[2]) == 0)
		output_encoding = 3;

	fd = open("/dev/conv_d", O_RDWR);
	//fd1 = open(argv[3], O_WRONLY);
	if(fd < 0){return errno;}
	//if(fd1 < 0){return errno;}

	ioctl(fd, WR_VALUE, (int32_t*) &input_encoding);
	ioctl(fd, WR_VALUE_2, (int32_t*) &output_encoding);

	res = read(fd, buffer, BUFFER_LENGTH);
	if(res < 0){printf("Failed to read from cdev\n");}
	//res = write(fd1, buffer, strlen(buffer));
	//if(res < 0){printf("Failed to write to file");}

	printf("%s", buffer);

	close(fd);
	close(fd1);
	return 0;
}
