/**************************************************************
* Class:  CSC-415-03 Fall 2022
* Names: Cristian Garcia, Kurtis Chan, Tsewang Sonam, and Francis Quang
* Student IDs: 920317889, 918139175, 921911364, 912679019
* GitHub Name: VtecFrancis
* Group Name: Clouds
* Project: Basic File System
*
* File: fsInit.h
*
* Description: Header file for the main driver for file system assignment.
* 
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

#define BLOCK_ZERO 0 //block 0
#define VCB_BLOCKSIZE 1 // size of the VCB in blocks
#define VCB_MAGIC_NUMBER 0x1B4EB4DA // used for checking if the VCB is already initialized
uint64_t rootStart;

typedef struct VCB {

	int NUM_BLOCKS; // Number of blocks needed in our block address
    int NUM_FREE_BLOCKS; // Number of free blocks
    int FREE_FCB_BLOCK_CNT; // Size of free file control blocks
    int FCB_PTR; // Number of free file controls block pointers
	uint64_t signature; // magic number used to tell if the volume is initialized
    uint64_t block_size; //max bytes of data in each block
    uint64_t free_space_start; // block where free space starts

} VCB;

//VCB will be globally accessible
extern VCB *vcb;

#define NUM_ENTRIES 50 //number of directory entries

typedef struct DirectoryEntry {

    #define BLOCK_SIZE 512

    int identifier;//id for file system to use

    char name[50]; //character name can't exceed 50 characters

    bool protected; //determine whether it is a protected file or not. 
    //protected files don't let people without permissions write, but they can read and execute
    //0 not protected, 1 protected

    bool free; //0 means occupied, 1 means free

    bool isDir;//0 means it is a file, 1 means it is a directory

    int size; //size of directory

    int location; //location of directory

    time_t lastModified;//time last modified

    char lastUser[20];//user that modified last


}DirectoryEntry;
