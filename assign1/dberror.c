#include <stdio.h>

#include "dberror.h"

/* Done improperly, do again with char *RC_message */
void printError(RC error)   {
    switch(error)   {
        case RC_OK:
            printf("%d: No error.\n", error);
        case RC_FILE_HANDLE_NOT_INIT:
            printf("%d: File Handle not initiatied.\n", error);
        case RC_FILE_NOT_FOUND:
            printf("%d: File not found.\n", error);
        case RC_FILE_INIT_FAILED:
            printf("%d: File creation failed.\n", error);
        case RC_FILE_READ_ERROR:
            printf("%d: File could not be read.\n", error);
        case RC_READ_NON_EXISTING_PAGE:
            printf("%d: Attempted to read page beyond file bounds.\n", error);
        case RC_WRITE_FAILED:
            printf("%d: File could not be written to.\n", error);
        case RC_WRITE_OUT_OF_BOUNDS_INDEX:
            printf("%d: Attempted to write page beyond file bounds.\n", error);
        case RC_FILE_CLOSE_ERROR:
            printf("%d: File did not close properly\n.", error);
        case RC_MALLOC_FAILED:
            printf("%d: malloc command failed.\n", error);
        case RC_FILE_REMOVE_FAILED:
            printf("%d: Could not remove file.\n", error);
    }
}
