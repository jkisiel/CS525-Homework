#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include "storage_mgr.h"
#include "dberror.h"

char *NULL0;

/* Use global FILE variable? */
extern void initStorageManager(void)    {
    int i;
    NULL0 = (char *) malloc(PAGE_SIZE);
    for(i=0; i<PAGE_SIZE; i++)  {
        NULL0[i] = '\0'
    }
}

/* makes sure all memory is freed
 * call after all file manipulation is done. */
extern void closeStorageManager(void)   {
    if(NULL0 != NULL)   free(NULL0);
}

/* creates a Page File
 * initializes it to a single page filled with '\0'
 * overwrites a file if it is already present
 */
extern RC createPageFile(char *fileName)   {
    FILE *fil;
//    uint16_t pos = 0;
//    char null0;

    fil = fopen(fileName, "w+");

    if(fil == NULL) return RC_FILE_INIT_FAILED;

    bytes_wrote = 0;
    if(bytes_wrote = fwrite(NULL0, 1, PAGE_SIZE, fil) < PAGE_SIZE)  {
        return RC_WRITE_FAILED;
    }

//    null0 = '\0';
//    while(pos < PAGE_SIZE)  {
//        fwrite(null0, sizeof(char), 1, fil);
//        pos++;
//    }
    if(fclose(fil) != 0)    return RC_FILE_CLOSE_ERROR;

    return RC_OK;
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
    FILE *fil;
    fil = fopen(fileName, "w");
    if(fil == NULL) return RC_FILE_NOT_FOUND;

    // Create a SM_FileHandle in the heap
    fHandle =   (struct SM_FileHandle *)
                malloc(sizeof(struct SM_FileHandle));

    if(fHandle == NULL) return RC_MALLOC_FAILED;

    // Copy the fileName into the heap
    // to preserve it independent of the scope
    // of the fileName parameter
    fHandle->fileName = (char *) malloc(sizeof(char)*strlen(fileName) + 1);
    if(fHandle->fileName == NULL)   return RC_MALLOC_FAILED;
    strcpy(fHandle->fileName, fileName);

    if(fseek(fil, 0, SEEK_END) != 0)    return RC_READ_ERROR;

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
    if(fil == NULL) return RC_FILE_NOT_FOUND;
    if(fclose(fil) != 0)    return RC_FILE_CLOSE_ERROR;

    free(fHandle->fileName);
    free(fHandle);

    return RC_OK;
}

extern RC destroyPageFile (char *fileName)  {
    FILE *fil = fopen(fileName, "r");
    if(fil == NULL) return RC_FILE_NOT_FOUND;
    if(fclose(fil) != 0)    return RC_FILE_CLOSE_ERROR;

    if(remove(fileName) != 0)   return RC_FILE_REMOVE_FAILED;

    return RC_OK;
}

/* reading blocks from disc */
/* ALL OF THESE METHODS NEED ERROR CHECKING!
 * Most of the checking should be in readBlock
 * to avoid redundancy.
 */

extern RC readBlock (int pageNum,
                    SM_FileHandle *fHandle,
                    SM_PageHandle memPage)    {

    if(fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;
    FILE *fil = (FILE *) fHandle->mgmtInfo;
    if(fil == NULL) return RC_FILE_NOT_FOUND;

    fseek(fil, pageNum*PAGE_SIZE, SEEK_SET);

    /* update error message to specify how many blocks
     * were read before a read error occured */
    bytes_read = 0;
    if(bytes_read = fread(memPage, 1, PAGE_SIZE, fil) < PAGE_SIZE)  {
        return RC_READ_ERROR;
    }

    return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle) {
//    FILE *fil = (FILE *) fHandle->mgmtInfo;
//    if(fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;
    return fHandle->curPagePos;
}

extern RC readFirstBlock(   SM_FileHandle *fHandle,
                            SM_PageHandle memPage)    {
    return readBlock(0, fHandle, memPage);
}

extern RC readPreviousBlock(SM_FileHandle *fHandle,
                            SM_PageHandle memPage) {
    fHandle->curPagePos -= 1;
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

extern RC readCurrentBlock( SM_FileHandle *fHandle,
                            SM_PageHandle memPage)  {
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

extern RC readNextBlock(SM_FileHandle *fHandle,
                        SM_PageHandle memPage) {
    fHandle->curPagePos += 1;
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

extern RC readLastBlock(SM_FileHandle *fHandle,
                        SM_PageHandle memPage) {
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

/* writing blocks to a page file */

extern RC writeBlock(   int pageNum,
                        SM_FileHandle *fHandle,
                        SM_PageHandle memPage)  {
    FILE *fil = (FILE *) fHandle->mgmtInfo;
    fseek(fil, pageNum*PAGE_SIZE, SEEK_SET);

    bytes_wrote = 0;
    if(bytes_wrote = fwrite(memPage, PAGE_SIZE, 1, fil) < PAGE_SIZE)    {
        return RC_WRITE_FAILED;
    }

    return RC_OK;
}

extern RC writeCurrentBlock(SM_FileHandle *fHandle,
                            SM_PageHandle memPage)  {
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
    return writeBlock(fHandle->totalNumPages, fHandle, NULL0);
}

extern RC ensureCapacity(   int numberOfPages,
                            SM_FileHandle *fHandle)    {

    if(fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;
    while(fHandle->totalNumPages < numberOfPages)   {
        appendEmptyBlock(fHandle);
        fHandle->totalNumPages++;
    }
}
