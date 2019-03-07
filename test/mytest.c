#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fs.h>
#define DATA_SIZE 1024
#define BUF_SIZE 128

int main(int argc, char **argv)
{

	char filename[] = "test.fs";
	
	char buf[BUF_SIZE];
	char data[DATA_SIZE];

	fs_mount(filename);
	fs_create("test1");
	int fd = fs_open("test1");

	for(int i=0;i<38;i++)
	{
		memset(data, 'A'+i%26, DATA_SIZE);
		printf("Writing %.*s\n",DATA_SIZE,data);
		fs_write(fd, data, DATA_SIZE);
	}

	fs_lseek(fd,0);

	while(fs_read(fd, buf, BUF_SIZE))
	{
		printf("%.*s\n",BUF_SIZE,buf);
	}

	fs_info();
	fs_ls();

	fs_close(fd);
	fs_delete("test1");
	fs_umount();
	

	return 0;
}
