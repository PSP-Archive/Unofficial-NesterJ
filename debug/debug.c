#include "debug.h"
#include "../nes/fileio.h"
#include "../nes/types.h"
#include "../syscall.h"

// ‚¢‚Âƒnƒ“ƒO‚·‚é‚©‚í‚©‚ç‚È‚¢‚Ì‚Å–ˆ‰ño—Í
void OutputDebugString(void *buf, int nLen)
{
	int hFile = -1;
	char szPath[MAXPATH];

	GetModulePath(szPath, sizeof(szPath));
	_strcat(szPath, "Debug.log");
	hFile = sceIoOpen(szPath, O_CREAT|O_APPEND|O_RDWR, 0777);
	if (hFile >= 0) {
		if (nLen > 512) {
			((char *)buf)[nLen] = '\0';
			mh_print(0,0,buf, 0xFFFF);
			pgScreenFlipV();
		}else {
			sceIoWrite(hFile, buf, nLen);
			sceIoClose(hFile);
		}
	}
}
