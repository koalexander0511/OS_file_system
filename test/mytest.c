#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fs.h>

int main(int argc, char **argv)
{
	char filename[] = "test.fs";
	fs_mount(filename);
	fs_info();

	fs_create("testfile1");
	fs_create("testfile2");
	fs_create("testfile3");
	fs_create("testfile4");
	fs_create("testfile5");
	fs_create("testfile6");
	fs_create("testfile7");
	fs_create("testfile8");

	int fd1a = fs_open("testfile1");
	int fd1b = fs_open("testfile1");
	int fd2 = fs_open("testfile2");
	int fd3 = fs_open("testfile3");
	fs_close(fd1b);
	int fd4 = fs_open("testfile4");

	assert(fs_stat(fd1a) == 0);
	assert(fs_stat(fd1b) == 0);
	assert(fs_stat(fd2) == 0);
	assert(fs_stat(fd3) == 0);
	assert(fs_stat(fd4) == 0);


	fs_ls();
	fs_info();
	return 0;
}
