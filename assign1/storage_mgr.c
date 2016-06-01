#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "storage_mgr.h"
#include "dberror.h"

char *NULL0;

/* Use global FILE variable? */
extern void initStorageManager(void)    {
    int i;
    NULL0 = (char *) malloc(PAGE_SIZE);
    for(i=0; i<PAGE_SIZE; i++)  {
        NULL0[i] = '\0';
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
    uint16_t pos = 0;
//    char null0;

    if(access(fileName, F_OK) != -1)    {
        if(remove(fileName) != 0)   return RC_FILE_REMOVE_FAILED;
    }

    fil = fopen(fileName, "w+");

    if(fil == NULL) return RC_FILE_INIT_FAILED;

    int bytes_wrote = 0;
    if(bytes_wrote = fwrite(NULL0, 1, PAGE_SIZE, fil) < PAGE_SIZE)  {
        return RC_WRITE_FAILED;
    }

//    char null0 = '\0';
//    while(pos < PAGE_SIZE)  {
//        fwrite(&null0, sizeof(char), 1, fil);
//        pos++;
//    }
    if(fflush(fil) != 0)    return RC_WRITE_FAILED;
    if(fclose(fil) != 0)    return RC_FILE_CLOSE_ERROR;

    return RC_OK;
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
    FILE *fil;
    if(access(fileName, F_OK) != 0)    {
        return RC_FILE_NOT_FOUND;
    }

    if(fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;

    fil = fopen(fileName, "rw");
    if(fil == NULL) return RC_FILE_NOT_FOUND;

    // Create a SM_FileHandle in the heap
    // NO NOT DO THIS!
//    fHandle =   (struct SM_FileHandle *)
//                malloc(sizeof(struct SM_FileHandle));

//    if(fHandle == NULL) return RC_MALLOC_FAILED;

    // Copy the fileName into the heap
    // to preserve it independent of the scope
    // of the fileName parameter
    fHandle->fileName = (char *) malloc(sizeof(char)*strlen(fileName) + 1);
    if(fHandle->fileName == NULL)   return RC_MALLOC_FAILED;
    strcpy(fHandle->fileName, fileName);

    if(fseek(fil, SEEK_SET, 0) != 0)    return RC_READ_ERROR;
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
//    free(fHandle);

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

/* 
 * Most of the checking should be in readBlock
 * to avoid redundancy.
 */

extern RC checkHandle(SM_FileHandle *fHandle)   {
    if(fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;
    FILE *fil = (FILE *) fHandle->mgmtInfo;
    if(fil == NULL) return RC_FILE_NOT_FOUND;

    return RC_OK;
}

extern RC readBlock (int pageNum,
                    SM_FileHandle *fHandle,
                    SM_PageHandle memPage)    {

    RC check_RC = RC_OK;
    if(check_RC = checkHandle(fHandle) != RC_OK)   return check_RC;

    FILE *fil = (FILE *) fHandle->mgmtInfo;
    if(fseek(fil, pageNum*PAGE_SIZE, SEEK_SET) != 0) return RC_READ_ERROR;

    /* update error message to specify how many blocks
     * were read before a read error occured */
    int bytes_read = 0;
    if(bytes_read = fread(memPage, 1, PAGE_SIZE, fil) < PAGE_SIZE)  {
        return RC_READ_ERROR;
    }

    return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle) {
//    FILE *fil = (FILE *) fHandle->mgmtInfo;
//    if(fHandle == NULL) return RC_FILE_HANDLE_NOT_INIT;

    RC check_RC = RC_OK;
    /* if fHandle is undefined, there is no curPagePos
     * returning an RC type would clash with
     * expected, correct return types */
    if(check_RC = checkHandle(fHandle) != RC_OK)   return -1;
   
    return fHandle->curPagePos;
}

extern RC readFirstBlock(   SM_FileHandle *fHandle,
                            SM_PageHandle memPage)    {
    RC check_RC = RC_OK;
    if(check_RC = checkHandle(fHandle) != RC_OK)    return check_RC;

    /* Check that page 0 exists */
    if(fHandle->totalNumPages < 1)  return RC_READ_NON_EXISTING_PAGE;

    return readBlock(0, fHandle, memPage);
}

extern RC readPreviousBlock(SM_FileHandle *fHandle,
                            SM_PageHandle memPage) {
    RC check_RC = RC_OK;
    if(check_RC = checkHandle(fHandle) != RC_OK)    return check_RC;

    /* Check that curPagePos isn't the first block
     * and is otherwise valid.
     * Though technically the previous block can be read
     * if curPagePos == totalNumPages, an error is
     * returned instead because this should not be the case. */
    if( fHandle->curPagePos < 1
        ||
        fHandle->curPagePos >=
            fHandle->totalNumPages) return RC_READ_NON_EXISTING_PAGE;

    fHandle->curPagePos -= 1;
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

extern RC readCurrentBlock( SM_FileHandle *fHandle,
                            SM_PageHandle memPage)  {
    RC check_RC = RC_OK;
    if(check_RC = checkHandle(fHandle) != RC_OK)    return check_RC;

    /* Check that the curPagePos is valid */
    if( fHandle->curPagePos < 0
        ||
        fHandle->curPagePos >=
           fHandle->totalNumPages) return RC_READ_NON_EXISTING_PAGE;

    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

extern RC readNextBlock(SM_FileHandle *fHandle,
                        SM_PageHandle memPage) {
    RC check_RC = RC_OK;
    if(check_RC = checkHandle(fHandle) != RC_OK)    return check_RC;

    /* Checks if next page will fall off end of file.
     * Also makes sure curPagePos is at least 0,
     * even though technically the next block if curPagePos == -1
     * is 0, curPagePos should not be -1. */
    if( fHandle->curPagePos < 0
        ||
        fHandle->curPagePos + 1 >=
            fHandle->totalNumPages) return RC_READ_NON_EXISTING_PAGE;

    fHandle->curPagePos += 1;

    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

extern RC readLastBlock(SM_FileHandle *fHandle,
                        SM_PageHandle memPage) {
    RC check_RC = RC_OK;
    if(check_RC = checkHandle(fHandle) != RC_OK)    return check_RC;

    if(fHandle->totalNumPages < 0)  return RC_READ_NON_EXISTING_PAGE;

    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

/* writing blocks to a page file */

extern RC writeBlock(   int pageNum,
                        SM_FileHandle *fHandle,
                        SM_PageHandle memPage)  {
    RC check_RC = RC_OK;
    if(check_RC = checkHandle(fHandle) != RC_OK)    return check_RC;

    if( pageNum < 0
        ||
        pageNum >=
            fHandle->totalNumPages)  return RC_WRITE_OUT_OF_BOUND_INDEX;

//    FILE *fil = (FILE *) fHandle->mgmtInfo;
    FILE *fil = fopen(fHandle->fileName, "w");
    if(fil == NULL) return RC_FILE_NOT_FOUND;
    if(fseek(fil, pageNum*PAGE_SIZE, SEEK_SET) != 0) return RC_READ_ERROR;

    int bytes_wrote = 0;
    if(bytes_wrote = fwrite(memPage, 1, PAGE_SIZE, fil) != PAGE_SIZE)    {
        return RC_WRITE_FAILED;
    }

    if(fclose(fil) != 0)    return RC_FILE_CLOSE_ERROR;
    return RC_OK;
}

extern RC writeCurrentBlock(SM_FileHandle *fHandle,
                            SM_PageHandle memPage)  {
    RC check_RC = RC_OK;
    if(check_RC = checkHandle(fHandle) != RC_OK)    return check_RC;

    if( fHandle->curPagePos < 0
        ||
        fHandle->curPagePos >=
            fHandle->totalNumPages) return RC_WRITE_OUT_OF_BOUND_INDEX;

    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
    RC check_RC = RC_OK;
    if(check_RC = checkHandle(fHandle) != RC_OK)    return check_RC;

    /* Checks that there aren't negative page numbers
     * otherwise page numbers will not be accurate */
    if(fHandle->totalNumPages < 0)  return RC_WRITE_OUT_OF_BOUND_INDEX;

    /* Confirm the write was successful before
     * incrementing totalNumPages */
    RC rc_write = RC_OK;
    if(rc_write = writeBlock(fHandle->totalNumPages + 1,
                            fHandle,
                            NULL0) != RC_OK) return rc_write;
    else fHandle->totalNumPages++;

    return RC_OK;
}

extern RC ensureCapacity(   int numberOfPages,
                            SM_FileHandle *fHandle)    {
    RC check_RC = RC_OK;
    if(check_RC = checkHandle(fHandle) != RC_OK)    return check_RC;

    while(fHandle->totalNumPages < numberOfPages)   {
        appendEmptyBlock(fHandle);
    }
}
