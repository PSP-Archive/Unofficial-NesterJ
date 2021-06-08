#ifndef _SCREEN_MANAGER
#define _SCREEN_MANAGER

#include "nes/types.h"
#include "nes/pixmap.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// 下記enumとchar*arrayは同じ順序で並んでいる必要があります。
enum {
	SCREEN_NORMAL = 0,
	SCREEN_FULL,
	SCREEN_X15,
	SCREEN_X15_CROPPED,
    SCREEN_GPU_NORMAL,
    SCREEN_GPU_X15,
    SCREEN_GPU_FULL,
	SCREEN_COUNT	// Last
};

static const char *aszScreenName[] =
{
	"NORMAL",
	"FULL SCREEN",
	"x1.5 [4:2.8333]",
	"x1.5 [4:2.9166](cropped)",
    "GPU NORMAL",
    "GPU 4:3",
    "GPU FULL SCREEN",
	NULL
};

// 画面にメッセージ表示用
void Scr_SetMessage(const char *pszMsg);

// パレット通知 pal[NES_MAX_COLORS][3]
void Scr_AssertPalette(uint8 pal[][3]);

// 画面モード PAL or NTSC
void Scr_SetScreenMode(const unsigned char ScreenMode);

// 指定した色で塗りつぶし
void Scr_ClearScreen(const uint8 PalNum);

// 画面へ転送
void Scr_BltScreen(void);

// 描画ロック
boolean Scr_Lock(pixmap *p);

// 描画アンロック
void Scr_Unlock(void);


// create thumbnail from current emu
boolean CreateThumnailFile(char *szThumbnailPath);
// load thumbnail
boolean LoadThumnailFile(char *szThumbnailPath, uint16 *pBuf);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
