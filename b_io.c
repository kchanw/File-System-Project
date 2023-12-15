/**************************************************************
* Class:  CSC-415-03 Fall 2022
* Names: Cristian Garcia, Kurtis Chan, Tsewang Sonam, and Francis Quang
* Student IDs: 920317889, 918139175, 921911364, 912679019
* GitHub Name: VtecFrancis
* Group Name: Clouds
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "mfs.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
{
	/** TODO add al the information you need in the file control block **/
	char *buf;	  //holds the open file buffer
	int index;	  //holds the current position in the buffer
	int buflen;	  //holds how many valid bytes are in the buffer
	int fileSize; //Holds the size of the file
	int cBlock;	  //Holds current Block address	
	int nBlock;   //Holds number of blocks file occupies
} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0; //Indicates that this has not been initialized

//Method to initialize our file system
void b_init()
{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
	}

	startup = 1;
}

//Method to get a free FCB element
b_io_fd b_getFCB()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].buf == NULL)
		{
			return i; //Not thread safe (But do not worry about it for this assignment)
		}
	}
	return (-1); //all in use
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open(char *filename, int flags)
{
	b_io_fd returnFd;
	char * buf;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//

	if (startup == 0)
		b_init(); //Initialize our system
	PathReturn isValid = parsePath(filename);
	if (isValid.index != -1 && isValid.dirPtr != -1)
	{
		return -1;
	}
	buf = malloc(B_CHUNK_SIZE);
	if (buf == NULL)
	{
		return (-1);
	}

	returnFd = b_getFCB(); // getting the file descriptor

	fcbArray[returnFd].buf = buf;
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].buflen = 0;
	fcbArray[returnFd].cBlock = 0;
	fcbArray[returnFd].nBlock = (fcbArray[returnFd].fileSize + (B_CHUNK_SIZE - 1)) / B_CHUNK_SIZE;

	return (returnFd); 
}

// Interface to seek function
int b_seek(b_io_fd fd, off_t offset, int whence)
{
	if (startup == 0)
		b_init(); //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); //invalid file descriptor
	}

	if (whence == 1)
	{
		//offset is set to be the current index plus the offset itself
		fcbArray[fd].index += offset;
	}
	else
	{
		if (whence == 2)
		{
			//current index is set to be the offset
			fcbArray[fd].index = offset;
		}
		else
		{
			if (whence == 3)
			{
				//index is set to be the file size plus the offset
				fcbArray[fd].index = fcbArray[fd].fileSize + offset;
			}
		}
	}
	return (fcbArray[fd].index);
}

// Interface to write function
int b_write(b_io_fd fd, char *buffer, int count)
{
	if (startup == 0)
		b_init(); //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); //invalid file descriptor
	}

	return (0); //Change this
}

// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read(b_io_fd fd, char *buffer, int count)
{
	int blocksRead;
	int bytesReturned;
	int p1, p2, p3;
	int copyBlocks;
	int bytesRemaining;

	if (startup == 0)
		b_init(); //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); //invalid file descriptor
	}
	//num bytes remaining in the buffer
	bytesRemaining = fcbArray[fd].buflen - fcbArray[fd].index;

	//limiting the number of bytes to read to length of file.
	int bytesDelivered = (fcbArray[fd].cBlock * BLOCK_SIZE);
	if((count + bytesDelivered) > fcbArray[fd].fileSize){
		count = fcbArray[fd].fileSize - bytesDelivered;

		if(count < 0){
			printf("ERROR: delivered more than fileSize");
		}
	}
	//from professor's example of assignment 5 separating into 3 parts
	//one for initial, one to handle multiple, and one to handle remaining
	
	if(count <= bytesRemaining){//enough space in buffer
		p1 = count;
		p2 = 0;
		p3 = 0;
	}
	else{
		p1 = bytesRemaining;//fill up buffer with remaining space

		p3 = count - bytesRemaining;

		copyBlocks = p3 / BLOCK_SIZE;//getting how many blocks needed to copy
		p2 = copyBlocks * BLOCK_SIZE;// #bytes to copy 

		p3 = p3 - p2 //whatever is left over
	}

	//fill existing buffer
	if(p1 > 0){
		memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, p1);
		fcbArray[fd].index += p1;
	}

	//fill multiple blocks
	if(p2 > 0){
		int temp = 0;

		for(int i = 0; i < copyBlocks; i++){
			blocksRead = LBAread(fcbArray[fd].buf , 1 , fcbArray[fd].cBlock + i);
			temp += blocksRead*BLOCK_SIZE;	

		}
		fcbArray[fd].cBlock += p2;
		p2 = temp;
	}

	//fill a new block
	if(p3 > 0){
		LBAread(fcbArray[fd].buf, 1, fcb[fd].cBlock);
		fcb[fd].index = 0;

		if(p3 > 0){
			memcpy(buffer + p1 + p2, fcbArray[fd].buf + fcbArray[fd].index, p3);
			fcbArray[fd].index += p3;
		}
	}

	bytesReturned = p1+p2+p3;
	return bytesReturned;	//Change this
}

// Interface to Close the file
int b_close(b_io_fd fd)
{
	free(fcbArray[fd].buf); 
	fcbArray[fd].buf = NULL; 

}
