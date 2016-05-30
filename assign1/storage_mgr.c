#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include "storage_mgr.h"
#include "dberror.h"


/* Use global FILE variable? */
extern void initStorageManager(void)    {
    //TODO
}

/* creates a Page File in the heap
 * initializes it to a single page filled with '\0'
 * overwrites a file if it is already present
 */
extern RC createPageFile(char *fileName)   {
    FILE *fil;
    uint16_t pos = 0;
    char null0;

    fil = fopen(fileName, "w");

    null0 = '\0';
    while(pos < PAGE_SIZE)  {
        fwrite(&null0, sizeof(char), 1, fil);
        pos++;
    }
    fclose(fil);

    return RC_OK;
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
    FILE *fil;
    fil = fopen(fileName, "r");
    if(fil == NULL) return RC_FILE_NOT_FOUND;

    // Create a SM_FileHandle in the heap
    fHandle =   (struct SM_FileHandle *)
                malloc(sizeof(struct SM_FileHandle));

    // Copy the fileName into the heap
    // to preserve it independent of the scope
    // of the fileName parameter
    fHandle->fileName = (char *) malloc(sizeof(char)*strlen(fileName) + 1);
    strcpy(fHandle->fileName, fileName);

    fseek(fil, 0, SEEK_END);

    fHandle->totalNumPages = (int) (ftell(fil) / PAGE_SIZE);
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = fil;

    return RC_OK;
}

/* frees the memory allocated by openPageFile
 * not safe, can break if fHandle is not
 * what it should be
 */
extern RC closePageFile (SM_FileHandle *fHandle)    {
    if(fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;

    FILE *fil = (FILE *) fHandle->mgmtInfo;
    fclose(fil);

    free(fHandle->fileName);
    free(fHandle);

    return RC_OK;
}

extern RC destroyPageFile (char *fileName)  {
    remove(fileName);
    return RC_OK;
}

/* reading blocks from disc */
/* ALL OF THESE METHODS NEED ERROR CHECKING!
 * Most of the checking should be in readBlock
 * to avoid redundancy.
 */

extern RC readBlock (int pageNum,
                    SM_FileHandle *fHandle, SM_PageHandle memPage)    {
    FILE *fil = (FILE *) fHandle->mgmtInfo;
    fseek(fil,  pageNum*PAGE_SIZE, SEEK_SET);

    fread(memPage, 1, PAGE_SIZE, fil);

    return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle) {
//    FILE *fil = (FILE *) fHandle->mgmtInfo;
    return fHandle->curPagePos;
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)    {
    return readBlock(0, fHandle, memPage);
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    fHandle->curPagePos -= 1;
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage)  {
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    fHandle->curPagePos += 1;
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

/* writing blocks to a page file */
/*
extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC appendEmptyBlock (SM_FileHandle *fHandle);
extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle);
*/
