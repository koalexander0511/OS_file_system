# Project 4 Report
---
---

## Introduction
The implemented file system is based on FAT where the layout
on a disk consists of four consecutive logical parts: the Superblock,
the File Allocation Table, the Root directory, and the remaining
Data Blocks.


## Data structure
The Superblock data structure has 7 items: signature, total block counts,
root directory block index, data block start index, amount of data blocks,
amount of FAT blocks, and the unused/padding elements. 

```c
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
```

The Root Directory is represented by a data structure with 4 elements:
the filename, filesize, index of first data block, and unused elements.
 ```c
struct __attribute__ ((__packed__)) DirectoryEntry
{
	unsigned char filename[16];
	uint32_t filesize;
	uint16_t dblk_i;
	uint8_t unused[10];
};
```

The FileDescriptor data structure represent each file and has two elements: 
the file represented by a DirectoryEntry struct and an integer
representing the offset.
```c
struct FileDescriptor
{
	struct DirectoryEntry * file;
	int offset;
};
```

## Mounting virtual disk
Mounting is done by the function `fs_mount`, which involves allocating 
memory for the file descriptor table, FAT, and the Root Directory with
`malloc`. All data in the file descriptor table is initialized to 0. 
In addition, several error checking conditions are checked using
functions like `block_read` from the given Disk API. A helper function
`errchk_spblk` was also used to check that all Superblock elements were 
correct.

## Unmounting virtual disk
Unmounting first involves checking that there exist virtual disk file opened. This is done by checking `block_disk_close` from the given Disk API as well as the global variable `num_open` which track the number of opened virtual disk. With all conditions satisfied, the memory allocated by `malloc` for the file descriptor table, FAT, and the Root Director are freed.

## File creation
Creating a new file is done with `fs_create` and begins with checking
the relevant conditions e.g. file exist and is not null). Afterwards,
a new Root Directory is made with its filename matching the input of
parameter of `fs_create`, with the Directory Entry's filesize set to 
0, index of first data block set to FAT_EOC. `block_write` from the Disk
API is used to write the content into the virtual disk's block.

## File deletion
File deletion is done with `fs_delete` and begins with checking relevant
conditions e.g. the existance of the targeted file. Afterwards, the file
entry is emptied and all data blocks which has the file's content is 
freed in the FAT. This is facilitated with the helper function
`delete_blocks` which zeroes out the entry.


## File opening
File opening is done with `fs_open` which returns a file descriptor
for further operations on the file. The global File Descriptor struct
has its file element pointing to the opened file, and File Descriptor
offset is set to 0. In addition, `num_open` is incremented to show the
new count of opened file.

## File closing
File closing is done with `fs_close` and has the global File Descriptor
struct's file element set to 0 (no longer pointing to the previously 
opened file), and its offset is once again set to 0. `num_open` is 
decremented to show the new count.

## File reading
The function `fs_read` is used to read from a file referenced by the
file descriptor parameter. A helper function `get_dblk_i` 
is used to return the index of the data block corresponding to 
the file's offset.

## File writing
The function `fs_write` is used to write into the file referenced by
the file descriptor parameter. A helper function `extend_file` is used
to allocate a new data block and link it to the end of the file's
data block chain in a first-fit strategy. 

