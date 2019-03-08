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
`malloc`. Since the data structure has a fixed size, we are able to
transfer the entire data structure by using block_read(). However, the FAT
might span several blocks, so a for loop and memory address offset was used
to complete the FAT.
All data in the file descriptor table is initialized to 0. 
Several error checking conditions are checked using
functions like `block_read` from the given Disk API. A helper function
`errchk_spblk` was also used to check that all Superblock elements were 
correct.

## Unmounting virtual disk
Unmounting involves checking that there exist virtual disk file opened.
This is done by checking `block_disk_close` from the given Disk API as well as
the global variable `num_open` which tracks the number of opened virtual disk.
With all conditions satisfied, the memory allocated by `malloc` for the file
descriptor table, FAT, and the Root Director are freed. The pointers to these
data structures are set to NULL.

## File creation
Creating a new file is done with `fs_create` and begins with checking
the relevant conditions (e.g. file exist and is not null). If all is well,
a helper function find_file() looks for an empty slot in the Root Directory
by finding a slot with the file name ""(=nothing). The pointer to that entry
is returned. The filesize is set to 0, the block index(dblk_i) is set to 
FAT_EOC. The changes are saved on disk by calling block_write().

## File deletion
File deletion is done with `fs_delete` and begins with checking relevant
conditions e.g. the existance of the targeted file. If the target filename
is found by find_file(), the target files entries are deleted. The filename
is set to "", filesize is set to 0, and the chaing of blocks referenced in the
FAT are all set to 0 by calling a helper function delete_blocks(). This function
traverses the chain of blocks for the specified file, and deletes them. The 
changes are saved on disk by calling block_write().

## File opening
File opening is done with `fs_open` which first finds the DirectoryEntry for
the filename specified by calling find_file(). Once the file is found, the
function traverses through the file descriptor table to find an empty slot.
One an empty slot is found, the index of the file descriptor table entry
is returned.

## File closing
File closing is done with `fs_close` and has the global File Descriptor
struct's file element set to 0 (no longer pointing to the previously 
opened file), and its offset is once again set to 0. `num_open` is 
decremented to show the new count.

## File reading
The function `fs_read` is used to read from a file referenced by the
file descriptor parameter. The main part of this function is the while loop,
which keeps going until the number of bytes read equals the number of bytes
requested by `count`. Note that `count` is adjusted beforehand to make sure
that the read operation does not go past the end of the file.
At the beginning of the function, a helper function get_dblk_i() is called.
This uses the file's location and offset to find which block the offset is
pointed to, and returns the data block index needed. block_read() is called
to get the specified block into a bounce buffer.
The first part of the while loop calculates the amount of data that needs to
be read from this block. If the bytes we need is greater than what can be read
from the offset to the end of the block, it read the entire block from the 
offset. If the bytes needed are all within the block, the number to read to set
to that value. The variable `need_from_curr_dblk` stores how much to read
from this block. After the reading operation is done, the `bytes_read` variable
is increamented, and the next block is read into the bounce_buffer. The loop
terminates when the number required by `count` is equal to the bytes read.
The offset is incremented by `bytes_read`, and `bytes_read` is reurned.

## File writing
The function `fs_write` is used to write into the file referenced by
the file descriptor parameter. A similar approach to `fs_read` is used here,
where there is a loop that tracks the number of bytes written, and the loop
continues until that value is equal to `count`. A bounce buffer is used here
as well, but in a different way. First, we get the block that we want to write
to by calling block_read() and storing it into the bounce buffer. Then, we 
determine which parts we want to modify in the block, and change the values. 
Once we are done with the block, the entire block is written back into where
we read it from. A variable `bytes_written` tracks the numbero of bytes written
into the file.
When we want to write past the end of the file, another block must be allocated
to the end of the file. This is done by the function extend_file(), which 
(1)finds the block pointed to FAT_EOC, (2)find a data block that is unused by
traversing through the FAT, and getting the first entry with value 0, and (3)
concatenating the new block at the end of the block chain. The last block is
returned.

##Testing
In addition to the provided testers, two tester functions were used.
The first tester, `unit_test.c` goes through all of the behaviors specified
by the API `fs.h` ("return -1 if file does not exist" etc...). All of the
invalid cases are tested here, as well as basic file manipulation.
The second tester, `mytest.c` performs more complex read/write operations.
It first continuously writes into a file with a for loop, and another loop
reads the same file to see if the data was written correctly. Disk and file
info are printed at the end.
The continuous write and read tests if inter-block operations are accurately
executed. Also, DATA_SIZE was changed to try to write more than the available
data space to see if the write operation stops when the available data space 
was depleted.

## Testing
In addition to the given `test_fs.c`, other test files like `mytest.c`and `unit_test.c`
were used to ensure that the FS API was correctly working, and that incorrect operations
would result in the correct return value.

