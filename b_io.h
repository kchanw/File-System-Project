/**************************************************************
* Class:  CSC-415-03 Fall 2022
* Names: Cristian Garcia, Kurtis Chan, Tsewang Sonam, and Francis Quang
* Student IDs: 920317889, 918139175, 921911364, 912679019
* GitHub Name: VtecFrancis
* Group Name: Clouds
* Project: Basic File System
*
* File: b_io.h
*
* Description: Interface of basic I/O functions
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>

typedef int b_io_fd;

b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);

#endif

