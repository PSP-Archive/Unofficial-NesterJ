/*
	load rom image by ruka
*/
#include "nes/types.h"
#include "filer.h"
#include "menu.h"
#include "main.h"
#include "syscall.h"
#include "pg.h"
#include "unzip/zlibInterface.h"

#define SHOW_ERRMSG(bgcolor,msg,color,vcnt) \
	{int nCount=vcnt; \
	while (nCount--) { \
		pgFillvram(bgcolor); \
		mh_print(0,0,msg,color); \
		pgScreenFlipV(); \
	} \
}




// コールバック受け渡し用
typedef struct {
	byte *pLoadBuffer;			// pointer to rom image
	uint32 ulLoadBufferSize;	// rom image size
	uint32 ulLoadedRomSize;		// loaded rom size
	boolean bIsBufferError;		// buffer error flag
	boolean bIsLoadDone;		// is load done?
}ROM_INFO, *LPROM_INFO;


// Unzip コールバック
int funcUnzipCallback(int nCallbackId, unsigned long ulExtractSize, unsigned long ulCurrentPosition,
                      const void *pData, unsigned long ulDataSize, unsigned long ulUserData)
{
    const char *pszFileName;
    int nExtId;
    const unsigned char *pbData;
    LPROM_INFO pRomInfo = (LPROM_INFO)ulUserData;

    switch(nCallbackId) {
    case UZCB_FIND_FILE:
		pszFileName = (const char *)pData;
		// 拡張子がファイルがNESなら展開
		nExtId = getExtId(pszFileName, (LPEXTENTIONS)&stExtRom);
		if (nExtId == EXT_NES) {
			return UZCBR_OK;
		}
        break;
    case UZCB_EXTRACT_PROGRESS:
		pbData = (const unsigned char *)pData;
		if ((ulCurrentPosition + ulDataSize) > pRomInfo->ulLoadBufferSize) {
			pRomInfo->bIsBufferError = TRUE;
			return UZEXR_CANCEL;
		}
		// 展開されたデータを格納しよう
		_memcpy(pRomInfo->pLoadBuffer + ulCurrentPosition, pbData, ulDataSize);
		draw_load_rom_progress(ulCurrentPosition + ulDataSize, ulExtractSize);
		// 展開終わっていたら抜ける
		if (ulCurrentPosition + ulDataSize == ulExtractSize) {
			pRomInfo->ulLoadedRomSize = ulExtractSize;
			pRomInfo->bIsLoadDone = TRUE;
			return UZEXR_CANCEL;
		}
		return UZCBR_OK;
        break;
    default: // unknown...
        break;
    }
    return UZCBR_PASS;
}


// load rom image by ruka
uint32 load_rom_image(uint8 *buf, uint32 bufLen, const char *szRomPath)
{
	int fd;
	long lReadSize=0;
	ROM_INFO stRomInfo;
	int nRet;
	int nExtId;
	char szTemp[10];

	stRomInfo.pLoadBuffer = buf;
	stRomInfo.ulLoadBufferSize = bufLen;
	stRomInfo.ulLoadedRomSize = 0;
	stRomInfo.bIsBufferError = FALSE;
	stRomInfo.bIsLoadDone = FALSE;

	switch(getExtId(szRomPath, (LPEXTENTIONS)&stExtRom)) {
	case EXT_NES:	// "nes" or "fds" or "fam" or "unf" or "nsf"
		fd=sceIoOpen(szRomPath, O_RDONLY, 0777);
		if (fd >= 0) {
			stRomInfo.ulLoadedRomSize = sceIoRead(fd, stRomInfo.pLoadBuffer, stRomInfo.ulLoadBufferSize);
			sceIoClose(fd);
			break;
		}
		return 0;
		break;
	case EXT_ZIP:	// "zip"
		// Unzipコールバックセット
		Unzip_setCallback(funcUnzipCallback);
		// Unzip展開する
	    nRet = Unzip_execExtract(szRomPath, (unsigned long)&stRomInfo);
		if (stRomInfo.bIsLoadDone) {
			// 展開成功
			break;
		}
		if (nRet == UZEXR_CANCEL && stRomInfo.bIsBufferError) {
			// out of buffer.
			SHOW_ERRMSG(RGB(255,0,0), "Unzip out of buffer.", 0xFFFF, 60);
			return 0;
		}
		// fatal error.
		SHOW_ERRMSG(RGB(255,0,0), "Unzip fatal error.", 0xFFFF, 60);
		return 0;
		break;
	default:
		return 0;
		break;
	}
	return stRomInfo.ulLoadedRomSize;
}
