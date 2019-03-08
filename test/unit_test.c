#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fs.h>
#define DATA_SIZE 2048
#define BUF_SIZE 2048

int main(int argc, char **argv)
{

	printf("Initiating unit test...\n\n");

	char filename[] = "unit_test.fs";
	
	char buf[BUF_SIZE];
	char data[DATA_SIZE];

	memset(data, 'X', DATA_SIZE);

	if(fs_mount(filename) == -1)
	{
		printf("virtual disk <%s> not found. Please create an empty virtual disk with this name.\n",filename);

		return -1;
	}
	else
		printf("Mounted <%s>\n",filename);

	assert(fs_mount("aasdfasdf;laksjdfa") == -1); //trying to open invalid file
	assert(fs_umount() == 0); //correct: unmounted test.fs
	assert(fs_umount() == -1); //error: nothing's mounted
	assert(fs_info() == -1); // error: nothing's mounted
	assert(fs_ls() == -1); //error: nothing's mounted

	fs_mount(filename);
	assert(fs_create("test1") == 0); //correct: file created
	assert(fs_create("test1") == -1); //error: filename already exists
	assert(fs_create(NULL) == -1); //error: invalid filename
	assert(fs_create("test12345678901234567890") == -1); //error: filename too long

	int fd;
	assert(fs_open(NULL) == -1); //error: invalid filename
	assert(fs_open("foo") == -1); //error: filename does not exist

	for(int i=0; i<FS_OPEN_MAX_COUNT;i++)
		fd = fs_open("test1");
	
	assert(fs_open("test1") == -1); //error: cannot open any more files

	for(int i=0; i<FS_OPEN_MAX_COUNT;i++)
		assert(fs_close(i) == 0);

	assert(fs_close(fd) == -1);//error: file already closed

	fd = fs_open("test1");
	assert(fd == 0);
	assert(fs_stat(-1) == -1);
	assert(fs_stat(99999) == -1);
	assert(fs_stat(2) == -1);
	assert(fs_stat(fd) == 0);

	assert(fs_write(-1, data, DATA_SIZE) == -1);
	assert(fs_write(99999, data, DATA_SIZE) == -1);
	assert(fs_write(fd, data, DATA_SIZE) == DATA_SIZE);

	assert(fs_lseek(-1, 0) == -1);
	assert(fs_lseek(99999, 0) == -1);
	assert(fs_lseek(fd, 99999) == -1);
	assert(fs_lseek(fd, -10) == -1);
	assert(fs_lseek(fd, 0) == 0);


	assert(fs_read(-1, buf, BUF_SIZE) == -1);
	assert(fs_read(99999, buf, BUF_SIZE) == -1);
	assert(fs_read(fd, buf, BUF_SIZE) == BUF_SIZE);

	printf("Writing: '%.*s...(%d bytes)'\n",10,data,DATA_SIZE);
	printf("Read: '%.*s...(%d bytes)'\n",10,buf,BUF_SIZE);

	assert(fs_delete("test1") == -1);


	assert(fs_close(-1) == -1);
	assert(fs_close(99999) == -1);
	assert(fs_close(fd) == 0);
	assert(fs_close(fd) == -1);

	assert(fs_delete(NULL) == -1);
	assert(fs_delete("foo") == -1);
	assert(fs_delete("test1") == 0);
	
	assert(fs_umount() == 0);
	assert(fs_umount() == -1);

	printf("\nUnit test success!\n");


	return 0;
}
