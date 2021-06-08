#ifndef NES_FILEIO_H_
#define NES_FILEIO_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	FILE_SEEK_CUR,
	FILE_SEEK_END,
	FILE_SEEK_SET,
};

enum {
 FILE_MODE_READ,
 FILE_MODE_WRITE,
 FILE_MODE_APPEND,
};

typedef int HANDLE;
//typedef unsigned int size_t;

HANDLE NES_fopen(const char *pPath, int mode);

int NES_fgetc(HANDLE fh);
int NES_fputc(int ch, HANDLE fh);

int NES_fgets(char *Buf, size_t nLen, HANDLE fh);

size_t NES_fread(void *, size_t size, size_t count, HANDLE fh);
size_t NES_fwrite(void *, size_t size, size_t count, HANDLE fh);

int NES_fseek(HANDLE fh, long offset, int origin);

int NES_fclose(HANDLE fh);


void GetModulePath(char *fn, int nSize);
void GetStatePath(char *fn, int nSize);
void GetSavePath(char *fn, int nSize);
char *PathFindFileName(const char *fn);
void NES_PathAddBackslash(char *fn);
void NES_PathRemoveFileSpec(char *fn);
void NES_PathRemoveExtension(char *fn);




#ifdef __cplusplus
}
#endif

#endif
