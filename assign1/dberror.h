#ifndef DBERROR_H
#define DBERROR_H

#define PAGE_SIZE 4096
typedef int RC;

RC RC_OK = 0;
RC RC_FILE_NOT_FOUND = 1;
RC RC_FILE_HANDLE_NOT_INIT = 2;
RC RC_WRITE_FAILED = 3;
RC RC_READ_NON_EXISTING_PAGE = 4;
RC RC_FILE_PRESENT = 5;
RC RC_FILE_READ_ERROR = 6;
RC RC_WRITE_OUT_OF_BOUND_INDEX = 7;

void printError(RC error);
void char *errorMessage(RC error);

#endif
