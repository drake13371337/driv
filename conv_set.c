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
#define RD_VALUE _IOR('a','b',int32_t*)
#define WR_VALUE_2 _IOW('a','c',int32_t*)
#define RD_VALUE_2 _IOR('a','d',int32_t*)
#define BUFFER_LENGTH 256

static char message[BUFFER_LENGTH];

int main(int argc, char** argv){
	int ret, fd, value, input_encoding = 0, output_encoding = 0;
	char koi8r [] = "KOI8-R";
	char cp1251 [] = "cp1251";
	char cp866 [] = "cp866";
	char stringToSend[BUFFER_LENGTH];

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

	fd = open("/dev/conv_c", O_RDWR);
	if(fd < 0){return errno;}

	ioctl(fd, WR_VALUE, (int32_t*) &input_encoding);
	ioctl(fd, WR_VALUE_2, (int32_t*) &output_encoding);

	ioctl(fd, RD_VALUE, (int32_t*) &value);
	ioctl(fd, RD_VALUE_2, (int32_t*) &value);

	ret = read(fd, message, BUFFER_LENGTH);
	if (ret < 0){return errno;}
	printf("%s", message);

	close (fd);
	return 0;
}
