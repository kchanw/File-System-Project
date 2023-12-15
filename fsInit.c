/**************************************************************
* Class:  CSC-415-03 Fall 2022
* Names: Cristian Garcia, Kurtis Chan, Tsewang Sonam, and Francis Quang
* Student IDs: 920317889, 918139175, 921911364, 912679019
* GitHub Name: VtecFrancis
* Group Name: Clouds
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "fsLow.h"
#include "mfs.h"
#include "fsInit.h"

#define BLOCK_ZERO 0 //block 0
#define VCB_BLOCKSIZE 1 // size of the VCB in blocks
#define VCB_MAGIC_NUMBER 0x1B4EB4DA // used for checking if the VCB is already initialized
//VCB will be globally accessible
extern VCB *vcb;
#define SIZE_OF_VOLUME 10000000
int* theFreeSpaceMap;

VCB *vcbPtr = NULL;

	int freeSpace (){

    int total_blocks = SIZE_OF_VOLUME / BLOCK_SIZE;             //  bits needed for vcb
    int num_bytes = total_blocks / 8;                         // bytes needed for vcb
    int num_blocks = num_bytes/ BLOCK_SIZE;                         //  blocks needed for vcb

    int mod_block = SIZE_OF_VOLUME % BLOCK_SIZE; 
    int mod_byte = total_blocks % 8;
    int mod_bit = num_bytes % BLOCK_SIZE;

    if(mod_block >0){ total_blocks++; }
    if(mod_byte >0){ num_bytes++; }
    if(mod_bit >0){  num_blocks ++; }

    theFreeSpaceMap = malloc( num_blocks * BLOCK_SIZE);

// in the VCB block 0,  allocate first 6 bits as 0 (occupied )  and remaining as 1 (free)
for(int i=0; i< num_bytes ; i++){
    if ( i < num_blocks) {
            theFreeSpaceMap[i] = 0; } 
else{
        theFreeSpaceMap[i] = 1; 
    }
        }

int startBlock = theFreeSpaceMap[num_blocks];

// write 5 blocks starting from block 1. (VCB is block 0 ) 
LBAwrite(theFreeSpaceMap, num_blocks, 1);

    free(theFreeSpaceMap);
	theFreeSpaceMap = NULL;
    return startBlock;
} 

int initRoot(){

    int numEntries = 51; //50 entries per directory
    int dirSize = numEntries + 1 * sizeof(DirectoryEntry);//size of a directory in bytes + root dir
    int numBlocks = dirSize/BLOCK_SIZE; //how many blocks we needed
    int needAnother = dirSize%BLOCK_SIZE;//check if another block is needed to fit data

    if(needAnother > 0){
       numBlocks++;
    }

    DirectoryEntry *firstDir = malloc(numBlocks*BLOCK_SIZE);

    for(int i = 0; i < numBlocks; i++)
    {
        firstDir[i].free = 1;
    }

    time_t updatedTime = time(NULL);

    int startBlock = freeSpace(numBlocks);

    firstDir[0].name == ".";
    firstDir[0].identifier  = startBlock;
    firstDir[0].lastModified = updatedTime;
    firstDir[0].isDir = true;
    firstDir[0].size = dirSize;

    firstDir[1].name == "..";
    firstDir[1].identifier = startBlock;
    firstDir[1].lastModified = updatedTime;
    firstDir[1].isDir = true;
    firstDir[1].size = dirSize;

    LBAwrite(firstDir, numBlocks, startBlock);
    free(firstDir);
    firstDir = NULL;
    return startBlock;

}

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	//vcb pointer
	vcbPtr = malloc(blockSize);

	//LBAreads block 0
	LBAread(vcbPtr, VCB_BLOCKSIZE, BLOCK_ZERO); 

	// if signature matches, then the volume has already been initialized
	if (vcbPtr->signature == VCB_MAGIC_NUMBER) {
		//Volume is initialized!
		return 0;
	} else { // initialize the volume

		// Makes sure that the volume can hold the VCB
		if (VCB_BLOCKSIZE > numberOfBlocks) {
			return -1;
		}

		// initialize values in volume control block
		vcbPtr->NUM_BLOCKS = numberOfBlocks;
		vcbPtr->NUM_FREE_BLOCKS = numberOfBlocks;
		vcbPtr->block_size = blockSize;
		vcbPtr->free_space_start = 0;
		vcbPtr->signature = VCB_MAGIC_NUMBER;

		//initBimap
		freeSpace();
		//initrootdirectory
		initRoot();

		//Write VCB to block 0 /disc
		LBAwrite(vcbPtr,VCB_BLOCKSIZE,BLOCK_ZERO);

	}
		return 0;
	}
	

void exitFileSystem ()
	{
	printf ("System exiting\n");
	}
