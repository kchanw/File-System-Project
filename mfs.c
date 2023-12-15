/**************************************************************
* Class:  CSC-415-03 Fall 2022
* Names: Cristian Garcia, Kurtis Chan, Tsewang Sonam, and Francis Quang
* Student IDs: 920317889, 918139175, 921911364, 912679019
* GitHub Name: VtecFrancis
* Group Name: Clouds
* Project: Basic File System
*
* File: mfs.c
*
* Description: Functions implemented to have the file fsshell integrated into the file system
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>

#include "fsLow.h"
#include "mfs.h"
#include "fsInit.h"

#define SIZE_OF_VOLUME 10000000

#define st_atime st_atim.tv_sec      
#define st_mtime st_mtim.tv_sec
 #define st_ctime st_ctim.tv_sec

 struct timespec st_atim;        /* Time of last access.  */
struct timespec st_mtim;        /* Time of last modification.  */
struct timespec st_ctim;        /* Time of last status change.  */


char *directoryPath = NULL; //used to get current working directory

//ParsePathFunction

PathReturn parsePath(char *path)
{
    char *delim = "/";
    int blockSpot = rootStart; //which block we need to go to
    int atDir = 0;
    int dirCount = 0;
    PathReturn retval; //what we return
    char *pathBuff = malloc(sizeof(path));
    pathBuff = strcpy(pathBuff, path); //mutable path
    char *token;                       //when using strtok

    DirectoryEntry *readBuffer = malloc(BLOCK_SIZE);

    LBAread(readBuffer, 1, blockSpot);

    if (pathBuff == NULL || path == NULL)
    { //when a null pointer is passed through
        retval.dirPtr = -1;
        retval.index = -1;
        return retval;
    }

    token = strtok(pathBuff, delim);
    dirCount++;

    while (token != NULL)
    {
        token = strtok(NULL, delim);
        dirCount++;
    }

    if (path[0] == '/')
    { //absolute
        token = strtok(pathBuff + 1, delim);

        while (token != NULL)
        {
            for (int i = 0; i < 51; i++)
            { //find the directory entry in the directory
                if (strcmp(readBuffer[i].name, token) == 0)
                { //when you find the entry
                    blockSpot = readBuffer[i].location;

                    if ((dirCount - 1) == atDir)
                    { //when you get to the last directory
                        retval.dirPtr = blockSpot;
                        retval.index = i;
                        return retval;
                    }
                }
            }
            atDir++;
        }
        token = strtok(NULL, delim);
    }
    else
    { //relative path

        pathBuff = fs_getcwd(path, dirCount); //get the absolute path for the cwd
        token = strtok(pathBuff + 1, delim);
        while (token != NULL)
        {
            for (int i = 0; i < 51; i++)
            { //find the directory entry in the directory
                if (strcmp(readBuffer[i].name, token) == 0)
                { //when you find the entry
                    blockSpot = readBuffer[i].location;

                    if ((dirCount - 1) == atDir)
                    { //when you get to the last directory
                        retval.dirPtr = blockSpot;
                        retval.index = i;
                        return retval;
                    }
                }
            }
            atDir++;
        }
        token = strtok(NULL, delim);
    }

    free(pathBuff);
    pathBuff = NULL;

    return retval;
}

int fs_setcwd(char *pathname)
{

    PathReturn isValidDir = parsePath(pathname);

    //parsePath will return -1 if the directory doesn't exist
    if (isValidDir.dirPtr == -1)
    {
        //will not cd into the directory if it doesn't exist
        printf("Directory does not exist");
        return -1;
    }
    else
    {
        //will cd into the directory if it's existing
        return 0;
    }
}

//gets working directory
char *fs_getcwd(char *pathname, size_t size)
{
    // This function returns the current working directory.

    if (directoryPath == NULL)
    {
        directoryPath = malloc(sizeof(char) + 1);
        if (directoryPath == NULL)
        {
            printf("Malloc error in getCWD\n");
            return -1;
        }
        strcpy(directoryPath, "/");
        strcpy(pathname, directoryPath);
        return (pathname);
    }
    if (size == 0)
    {
        printf("Size is 0 in getCwd \n");
        return -1;
    }
    if (size < strlen(directoryPath))
    {
        printf("Increase size of buffer in getCwd \n");
        return -1;
    }
    strncpy(pathname, directoryPath, size);

    //returns directory
    return pathname;
}

//fs_isFile
//returns 1 if path is a file
int fs_isFile(char *filename)
{
    //checks fs_isDir function to see if it's a directory
    if (fs_isDir(filename) == 1)
    {
        return 0;
    }
    //if it's not a directory then it returns a 1
    else
    {
        return 1;
    }
}

int fs_isDir(char * path){
    PathReturn currentDir = parsePath(path);

    if(currentDir.index == -1){
        return -1;
    }
    DirectoryEntry* buffer = malloc(BLOCK_SIZE);

    LBAread(buffer, 1, currentDir.dirPtr);
    

    if(buffer[currentDir.index].isDir == 1){
        return 1;//is a directory
    }
    return 0;//not a directory -> means it is a file
}

int fs_mkdir(const char *pathname, mode_t mode)
{

    char cwd[256];
    char *currentPath = fs_getcwd(cwd, sizeof(cwd));

    if (strcmp(pathname,NULL) ==0 ||strcmp(pathname,"root")==0||strcmp(pathname,".")==0||strcmp(pathname,"..")==0 )
    {
        printf("Error \n");
        return 0;
    }

    fdDir* dir;
    int size= sizeof(*dir);
    dir = malloc(size);
    char* buffer = malloc(BLOCK_SIZE);
    PathReturn currentDirLBA = parsePath(currentPath);

    if (currentDirLBA.index != -1)
    {
       perror("Directory already exists"); // error : directory exist
    }
    else
    {
        int makeBlockStart = initRoot();
        return makeBlockStart;
    }
}

fs_delete(char *filename)
{ //deleting a file
    PathReturn isValid = parsePath(filename);
    DirectoryEntry *fileToDelete = malloc(BLOCK_SIZE);
    if (isValid.index != -1 && isValid.dirPtr != -1)
    {
        perror("INVALID INDEX/BLOCK SPOT");
        return -1;
    }

    LBAread(fileToDelete, 1, isValid.dirPtr);
    fileToDelete[isValid.index].free = 1;
    LBAwrite(fileToDelete, 1, isValid.dirPtr);

    free(fileToDelete);
    fileToDelete = NULL;
    return 0;
}

int fs_stat(const char *path, struct fs_stat *buf)
{
    if (strcmp(path, "/") != 0)
    {
        return ENOENT;
    }   else{
    buf -> st_size = SIZE_OF_VOLUME / BLOCK_SIZE;           /* total size, in bytes */
    buf -> st_blksize = 512;            // blocksize for file system I/O 
    buf -> st_blocks =  VCB_BLOCKSIZE;        // number of 512B blocks allocated /
    buf -> st_accesstime = st_atime;        // time of last access /
    buf -> st_modtime = st_mtime;        // time of last modification /
    buf -> st_createtime = st_ctime;        // time of last status change */
} 
}

int fs_rmdir(const char *pathname){ //deleting a directory
     PathReturn isValid =  parsePath(pathname);
  DirectoryEntry *fileToDelete = malloc(BLOCK_SIZE);
  if(isValid.index != -1 && isValid.dirPtr != -1){
    perror("INVALID INDEX/BLOCK SPOT"); 
    return -1;
  }
  LBAread(fileToDelete,1,isValid.dirPtr);

    for(int i = 0; i < 51; i++){
        if(fileToDelete[i].free != 1){
            perror("File not empty");
            return -1;
        }
    }


  LBAwrite(fileToDelete,1,isValid.dirPtr);

  free(fileToDelete);
  fileToDelete = NULL;

  return 0;
}
