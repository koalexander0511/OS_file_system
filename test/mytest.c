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
	fs_ls();
	printf("\n");

	int fd = fs_open("test-file-2");
	const int NUMBYTES = 16;
	char buf[NUMBYTES];

	while(fs_read(fd, buf, NUMBYTES) != 0)
	{
		//printf("buffer value = <%.*s>\n", NUMBYTES, buf);
	}

	printf("Closing....\n");
	fs_close(fd);
	fs_umount();

	return 0;
}
