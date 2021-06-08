#include "fileio.h"
#include "nes_string.h"
#include "../syscall.h"
#include "../main.h"
#include "../debug/debug.h"

// fgets専用
uint8 g_DBBuf[1024*1024];	// NES_ROMのfgets専用
uint32 g_DBbufPtr, g_DBbufLen = 0;	// g_DBbufの位置


HANDLE NES_fopen(const char *pPath, int mode)
{
	int hFile = -1;
	g_DBbufLen = g_DBbufPtr = 0;
	DEBUG(pPath);
	switch (mode) {
	case FILE_MODE_READ:
		DEBUG("NES_fopen(READ)");
		hFile = sceIoOpen(pPath, O_RDONLY, 0777);
		break;
	case FILE_MODE_WRITE:
		DEBUG("NES_fopen(WRITE)");
		hFile = sceIoOpen(pPath, O_CREAT|O_WRONLY|O_TRUNC, 0777);
		break;
	case FILE_MODE_APPEND:
		DEBUG("NES_fopen(APPEND)");
		hFile = sceIoOpen(pPath, O_APPEND|O_RDWR, 0777);
		break;
	}
	if (hFile < 0) {
		DEBUG("NES_fopen failed");
		DEBUG(pPath);
		return hFile;
	}
	DEBUG("NES_fopen ");
	DEBUG(pPath);
	return (HANDLE)hFile;
}

int NES_fgetc(HANDLE fh)
{
	unsigned char ch = 0;
	DEBUG(" NES_fgetc");
	if (sceIoRead((int)fh, &ch, 1)) {
		return ch;
	}
	else {
		return -1;
	}
}

int NES_fputc(int chVal, HANDLE fh)
{
	char ch = chVal;
	DEBUG(" NES_fputc");
	return sceIoWrite((int)fh, &ch, 1);
}



// 改行のみの行はすっ飛ばします
int NES_fgets(char *Buf, size_t nLen, HANDLE fh)
{
/*	下記コードは死ぬほど遅い
	int ch, cbI = 0;

	nLen--;// '\0'
	while (1) {
		ch = NES_fgetc(fh);
		if (ch == -1) return 0;
		if (ch != '\r' && ch !='\n') break;
	}
	do {
		Buf[cbI] = ch;
		cbI++;
		if (cbI > nLen) break;
		ch = NES_fgetc(fh);
	} while (ch != -1 && ch != '\r' && ch !='\n');

	Buf[cbI] = '\0';
	return 1;
*/
	int ch, cbI = 0;

	nLen--;// '\0'
	if (!g_DBbufPtr) {
		// 一気読み
		g_DBbufLen = NES_fread(g_DBBuf, 1, sizeof(g_DBBuf), fh);
	}
	while (1) {
		if (g_DBbufPtr > g_DBbufLen) return 0;
		if (g_DBBuf[g_DBbufPtr] != '\r' && g_DBBuf[g_DBbufPtr] !='\n') break;
		g_DBbufPtr++;
	}
	do {
		Buf[cbI] = g_DBBuf[g_DBbufPtr];
		cbI++;
		g_DBbufPtr++;
		if (cbI > nLen || g_DBbufPtr > g_DBbufLen) break;
	} while (g_DBBuf[g_DBbufPtr] != '\r' && g_DBBuf[g_DBbufPtr] !='\n');

	Buf[cbI] = '\0';
	return 1;
}

size_t NES_fread(void *buf, size_t size, size_t count, HANDLE fh)
{
	DEBUG(" NES_fread");
	return sceIoRead(fh, buf, size*count);
}

size_t NES_fwrite(void *buf, size_t size, size_t count, HANDLE fh)
{
	DEBUGVALUE(" NES_fwrite hFile", fh);
	return sceIoWrite(fh, buf, size*count);
}

int NES_fseek(HANDLE fh, long offset, int origin)
{
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2
	DEBUG("NES_fseek");
	switch (origin) {
	case FILE_SEEK_CUR:
		return sceIoLseek((int)fh, (long long)offset, SEEK_CUR);
	case FILE_SEEK_END:
		return sceIoLseek((int)fh, (long long)offset, SEEK_END);
	case FILE_SEEK_SET:
		return sceIoLseek((int)fh, (long long)offset, SEEK_SET);
	}
	return -1;
}

int NES_fclose(HANDLE fh)
{
	DEBUG("NES_fclose hFile");
	sceIoClose((int)fh);
	return TRUE;
}

void NES_DeleteFile(const char *pFile)
{
	sceIoRemove(pFile);
}

extern char pg_workdir[MAX_PATH];// pg.c

void GetModulePath(char *fn, int nSize)
{
	_strncpy(fn, pg_workdir, nSize);
}

void GetStatePath(char *fn, int nSize)
{
	GetModulePath(fn, nSize);
	_strcat(fn, "STATE/");
}

void GetSavePath(char *fn, int nSize)
{
	GetModulePath(fn, nSize);
	_strcat(fn, "SAVE/");
}

char *PathFindFileName(const char *fn)
{
	char *pszSlash = NULL;
	int cbI = 0;

	while(fn[cbI]) {
		if ((((unsigned char)fn[cbI]>=0x80) && ((unsigned char)fn[cbI]<0xa0)) || ((unsigned char)fn[cbI]>=0xe0)) {
			if (fn[cbI+1]) {
				cbI+=2;
				continue;
			}
			break;
		}
		if (fn[cbI] == '/') {
			pszSlash = (char*)fn+cbI;
		}
		cbI++;
	}
	if (!pszSlash) {
		// 応急処置
		pszSlash = _strrchr(fn, '/');
	}
	if (pszSlash) pszSlash++;
	return pszSlash;
}


void NES_PathAddBackslash(char *fn )
{
	int len = _strlen(fn);
	if (len) {
		if (fn[len-1] != '\\') {
			fn[len] = '\\';
			fn[len+1] = '\0';
		}
	}

}

void NES_PathRemoveFileSpec(char *fn)
{
	int pt = _strlen(fn);
	while(fn[pt] != '\\') pt--;
	fn[pt+1] = '\0';
}

void NES_PathRemoveExtension(char *fn)
{
	int pt = _strlen(fn);
	while(fn[pt] != '.') {
		pt--;
		if (pt < 0) return;
	}
	fn[pt] = '\0';
}


