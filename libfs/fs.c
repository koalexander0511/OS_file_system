#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>

#include "disk.h"
#include "fs.h"

const uint16_t FAT_EOC = 0xFFFF;

struct __attribute__ ((__packed__)) Superblock
{
	unsigned char signature[8];
	uint16_t total_blk_count;
	uint16_t rdir_blk;
	uint16_t data_blk;
	uint16_t data_blk_count;
	uint8_t fat_blk_count;
	uint8_t unused[4079];
};

static int fat_free_count = -1;
static int rdir_free_count = -1;

struct __attribute__ ((__packed__)) DirectoryEntry
{
	unsigned char filename[16];
	uint32_t filesize;
	uint16_t dblk_i;
	uint8_t unused[10];
};

struct FileDescriptor
{
	struct DirectoryEntry * file;
	int offset;
};

struct Superblock * spblk = NULL;
uint16_t * FAT = NULL;
struct DirectoryEntry * rtdir = NULL;
struct FileDescriptor * fd_table = NULL;
int num_open = 0;

static int errchk_spblk() {

	// Invalid signature for superblock
	if (strncmp((char *)spblk->signature, "ECS150FS", 8) != 0) {
		return -1;
	}

	// No virtual disk file opened
	if (block_disk_count() == -1) {
		return -1;
	} 
	// Blocks open disk contains does not match with amount
	if (block_disk_count() != spblk->total_blk_count) {
		return -1;
	}

	return 0;
}


int fs_mount(const char *diskname)
{
	//create file descriptor table and initialize all data to 0
	fd_table = malloc(sizeof(struct FileDescriptor)*FS_OPEN_MAX_COUNT);
	memset(fd_table, 0, sizeof(struct FileDescriptor)*FS_OPEN_MAX_COUNT);

	if (block_disk_open(diskname) == -1)
		return -1;

	spblk = (struct Superblock *)malloc(BLOCK_SIZE); 

	if(block_read(0, (void *)spblk) == -1)
		return -1;

	if (errchk_spblk() == -1)
		return -1;

	// alocate memory for FAT
	FAT = malloc(spblk->data_blk_count * BLOCK_SIZE);

	for(int i=0; i<spblk->data_blk_count; i++)
		block_read( i+1, (void *)(FAT + (BLOCK_SIZE/sizeof(FAT))*i) );

	// alocate memory for Root Directory
	rtdir = malloc(BLOCK_SIZE);

	if(block_read(spblk->rdir_blk, (void *)rtdir) == -1)
		return -1;
	
	return 0;
}

int fs_umount(void)
{
	if(num_open != 0)
		return -1;

	if(block_disk_close())
		return -1;
	
	free(spblk);
	free(FAT);
	free(rtdir);

	spblk = NULL;
	FAT = NULL;
	rtdir = NULL;

	return 0;
}

int fs_info(void)
{
	if(spblk == NULL)
		return -1;

	fat_free_count = 0;
	for(int i=0; i<spblk->data_blk_count;i++)
		if(FAT[i] == 0)
			fat_free_count++;

	rdir_free_count = 0;
	for(int i=0; i<FS_FILE_MAX_COUNT; i++)
		if(strcmp((char *)rtdir[i].filename,"") == 0)
			rdir_free_count++;

	printf("FS Info:\n");
	printf("total_blk_count=%d\n", spblk->total_blk_count);
	printf("fat_blk_count=%d\n", spblk->fat_blk_count);
	printf("rdir_blk=%d\n", spblk->rdir_blk);
	printf("data_blk=%d\n", spblk->data_blk);
	printf("data_blk_count=%d\n", spblk->data_blk_count);
	printf("fat_free_ratio=%d/%d\n", fat_free_count, spblk->data_blk_count);
	printf("rdir_free_ratio=%d/%d\n", rdir_free_count, FS_FILE_MAX_COUNT);
	return 0;
}

struct DirectoryEntry * find_file(const char *target_fname){

	for(int i=0; i<FS_FILE_MAX_COUNT; i++)
		if(strcmp((char *)rtdir[i].filename,target_fname) == 0)
			return &rtdir[i];

	return NULL;
}

int fs_create(const char *filename)
{
	if(filename == NULL)
		return -1;

	if(find_file(filename) != NULL)
		return -1;

	if(strlen(filename) > FS_FILENAME_LEN)
		return -1;

	struct DirectoryEntry * new_entry = find_file("");

	if(new_entry == NULL)
		return -1;

	strcpy((char *)new_entry->filename, filename);

	new_entry->filesize = 0;

	new_entry->dblk_i = FAT_EOC;
	
	block_write(spblk->rdir_blk, rtdir);

	return 0;
}

void delete_blocks(struct DirectoryEntry * target)
{
	uint16_t curr = target->dblk_i;
	uint16_t prev;
	
	target->dblk_i = 0;

	if(curr == FAT_EOC)
		return;

	while(FAT[curr] != FAT_EOC)
	{
		prev = curr;
		curr = FAT[curr];
		FAT[prev] = 0;
	}
	FAT[curr] = 0;

}

int fs_delete(const char *filename)
{

	if(filename == NULL)
		return -1;

	struct DirectoryEntry * target_entry = find_file(filename);

	if(target_entry == NULL)
		return -1;

	for(int i=0; i<FS_OPEN_MAX_COUNT; i++)
		if(fd_table[i].file != 0)
			if(strcmp((char *)fd_table[i].file->filename, filename) == 0)
				return -1;
	
	strcpy((char *)target_entry->filename, "");
	target_entry->filesize = 0;
	delete_blocks(target_entry);

	for(int i=0; i<spblk->fat_blk_count; i++)
		block_write( i+1, (void *)(FAT + (BLOCK_SIZE/sizeof(FAT))*i) );
	block_write(spblk->rdir_blk, rtdir);
	return 0;
}

int fs_ls(void)
{
	if(spblk == NULL)
		return -1;

	for(int i=0; i<FS_FILE_MAX_COUNT; i++)
		if(strcmp((char *)rtdir[i].filename,"") != 0)
			printf("file: %s, size: %d, data_blk: %d\n", rtdir[i].filename, rtdir[i].filesize, rtdir[i].dblk_i);

	return 0;
}

int fs_open(const char *filename)
{
	if(filename == NULL)
		return -1;

	struct DirectoryEntry * target_entry = find_file(filename);

	if(target_entry == NULL)
		return -1;


	for(int i=0; i<FS_OPEN_MAX_COUNT; i++)
	{
		if(fd_table[i].file == 0)
		{
			fd_table[i].file = target_entry;
			fd_table[i].offset = 0;
			num_open++;
			return i;
		}

	}

	return -1;
}

int fs_close(int fd)
{
	if(fd < 0 || fd > FS_OPEN_MAX_COUNT)
		return -1;

	if(fd_table[fd].file == 0)
		return -1;

	fd_table[fd].file = 0;
	fd_table[fd].offset = 0;
	
	num_open--;
	return 0;
}

int fs_stat(int fd)
{
	if(fd < 0 || fd > FS_OPEN_MAX_COUNT)
		return -1;

	if(fd_table[fd].file == 0)
		return -1;

	return fd_table[fd].file->filesize;
}

int fs_lseek(int fd, size_t offset)
{
	if(fd < 0 || fd > FS_OPEN_MAX_COUNT)
		return -1;

	if(fd_table[fd].file == 0)
		return -1;
	
	if(fd_table[fd].file->filesize < offset || offset < 0)
		return -1;

	fd_table[fd].offset = offset;

	return 0;
}

int get_dblk_i(int fd){

	uint16_t curr = fd_table[fd].file->dblk_i;
	int num_hops = fd_table[fd].offset / BLOCK_SIZE;

	for(int i=0;i<num_hops;i++)
	{
		curr = FAT[curr]; 
	}
	
	return curr;
}

uint16_t extend_file(int fd)
{
	uint16_t curr = fd_table[fd].file->dblk_i;
	uint16_t new_block = 0;

	//find a new block
	for(int i=0;i<spblk->data_blk_count;i++)
	{
		if(FAT[i] == 0)
		{
			new_block = i;
			break;
		}
	}

	//no more space in disk
	if(new_block == 0)
		return 0;

	//go to the last block of this file
	if(curr != FAT_EOC)
		while(FAT[curr] != FAT_EOC)
			curr = FAT[curr];

	//chain the new block with the file
	if(curr == FAT_EOC)
		fd_table[fd].file->dblk_i = new_block;
	else
	{
		FAT[curr] = new_block;
	}
	FAT[new_block] = FAT_EOC;

	//return the index of the new block
	return new_block;
}

int fs_write(int fd, void *buf, size_t count)
{
	if(fd < 0 || fd > FS_OPEN_MAX_COUNT)
		return -1;

	if(fd_table[fd].file == 0)
		return -1;

	unsigned char * bounce_buf;
	bounce_buf = malloc(BLOCK_SIZE);
	int curr_dblk_offset;
	int curr_dblk_i;
	int bytes_written = 0;
	int write_to_curr_dblk;

	curr_dblk_i = get_dblk_i(fd);
	curr_dblk_offset = fd_table[fd].offset % BLOCK_SIZE;

	while(bytes_written != count)
	{
		if(curr_dblk_i == FAT_EOC)
		{
			curr_dblk_i = extend_file(fd);
			//disk full; return the amount written to disk so far
			if(!curr_dblk_i)
				break;
		}
		
		if(block_read(spblk->data_blk + curr_dblk_i, bounce_buf) == -1)
			return 0;

		//we need more blocks
		if(curr_dblk_offset+count-bytes_written > BLOCK_SIZE)
		{
			write_to_curr_dblk = BLOCK_SIZE - curr_dblk_offset;
		}
		else
		{
			write_to_curr_dblk = count - bytes_written;
		}

		memcpy((char *)&bounce_buf[curr_dblk_offset], (char *)(buf+bytes_written), write_to_curr_dblk);
		block_write(spblk->data_blk + curr_dblk_i, bounce_buf);

		bytes_written += write_to_curr_dblk;
		
		curr_dblk_i = FAT[curr_dblk_i];
		curr_dblk_offset = 0;

	}

	fd_table[fd].offset += bytes_written;
	if(fd_table[fd].offset > fd_table[fd].file->filesize)
		fd_table[fd].file->filesize += bytes_written;

	for(int i=0; i<spblk->fat_blk_count; i++)
		block_write( i+1, (void *)(FAT + (BLOCK_SIZE/sizeof(FAT))*i) );
	block_write(spblk->rdir_blk, rtdir);
	
	return bytes_written;
}

int fs_read(int fd, void *buf, size_t count)
{
	if(fd < 0 || fd > FS_OPEN_MAX_COUNT)
		return -1;

	if(fd_table[fd].file == 0)
		return -1;

	if(fd_table[fd].offset == fd_table[fd].file->filesize)
	{
		return 0;
	}

	unsigned char * bounce_buf;
	bounce_buf = malloc(BLOCK_SIZE);

	int curr_dblk_offset;
	uint16_t curr_dblk_i;
	int bytes_read = 0;
	int need_from_curr_dblk;


	if(count > fd_table[fd].file->filesize - fd_table[fd].offset)
	{
		count = fd_table[fd].file->filesize - fd_table[fd].offset;
	}

	curr_dblk_i = get_dblk_i(fd);

	if(block_read(spblk->data_blk + curr_dblk_i, bounce_buf) == -1)
		return 0;

	curr_dblk_offset = fd_table[fd].offset % BLOCK_SIZE;

	while(bytes_read != count)
	{
		
		if(curr_dblk_offset + count - bytes_read > BLOCK_SIZE)
		{
			need_from_curr_dblk = BLOCK_SIZE - curr_dblk_offset;
		}
		else
		{
			need_from_curr_dblk = count - bytes_read;
		}
		
		memcpy((char *)(buf+bytes_read), (char *)&bounce_buf[curr_dblk_offset], need_from_curr_dblk);

		bytes_read += need_from_curr_dblk;

		curr_dblk_i = FAT[curr_dblk_i];
		if(curr_dblk_i != FAT_EOC)
			if(block_read(spblk->data_blk + curr_dblk_i, bounce_buf) == -1)
				return 0;

		curr_dblk_offset = 0;
	}


	fd_table[fd].offset += bytes_read;

	return bytes_read;
}

