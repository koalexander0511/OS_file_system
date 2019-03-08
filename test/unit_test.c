#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fs.h>
#define DATA_SIZE 12288
#define BUF_SIZE 4096

int main(int argc, char **argv)
{

	char filename[] = "test.fs";
	char buf[BUF_SIZE];
	char data[DATA_SIZE];


	assert(fs_mount("nonexistant.fs") == -1);
	assert(fs_mount(filename) == 0);


	assert(fs_create("test1") == 0);
	assert(fs_create("test1") == -1); // file already exist

	int fd = fs_open("test1");
	assert(fs_open("nonexistant") == -1); // file does not exist 

	for(int i=0;i<2;i++)
	{
		memset(data, '0'+i%10, DATA_SIZE);
		printf("Writing: '%.*s...'\n",10,data);
		fs_write(fd, data, DATA_SIZE);
	}

	assert(fs_lseek(fd,0) == 0);

	while(fs_read(fd, buf, BUF_SIZE))
	{
		printf("Read: '%.*s...'\n",10,buf);
	}

	assert(fs_info() == 0);
	assert(fs_ls() == 0);

	assert(fs_close(fd) == 0);
	assert(fs_close(fd) == -1); // file already closed

	assert(fs_delete("test1") == 0);
	assert(fs_delete("test1") == -1); // file already deleted


	assert(fs_umount() == 0);
	assert(fs_umount() == -1); // no underlying virtual disk was opened

	assert(fs_info() == -1); // no underlying virtual disk was opened
	assert(fs_ls() == -1); // no underlying virtual disk was opened
	
	return 0;
}