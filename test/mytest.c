#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fs.h>
#define DATA_SIZE 2048
#define BUF_SIZE 4096

int main(int argc, char **argv)
{

	char filename[] = "test.fs";
	
	char buf[BUF_SIZE];
	char data[DATA_SIZE];

	if(fs_mount(filename) == -1)
	{
		printf("virtual disk <%s> not found. Exiting...\n",filename);
		return -1;
	}
	fs_create("test1");
	int fd = fs_open("test1");

	for(int i=0;i<16;i++)
	{
		memset(data, '0'+i%10, DATA_SIZE);
		printf("Writing: '%.*s...(%d bytes)'\n",10,data,fs_write(fd, data, DATA_SIZE));
	}

	fs_lseek(fd,0);

	while(fs_read(fd, buf, BUF_SIZE))
	{
		printf("Read: '%.*s...(%d bytes)'\n",10,buf,BUF_SIZE);
	}

	fs_info();
	fs_ls();

	fs_close(fd);
	fs_delete("test1");
	fs_umount();
	

	return 0;
}
