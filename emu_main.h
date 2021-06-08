#ifndef EMU_MAIN_H
#define EMU_MAIN_H
#include "nes/types.h"

enum {
	EMU_SPEED_NORMAL,
	EMU_SPEED_MODE1,
	EMU_SPEED_MODE2
};

// NES init
void PSPEMU_NES_Init(void);

// メニュー入るとき呼び出す
void PSPEMU_Freeze(void);

// Freeseして処理終えたら呼び出す
void PSPEMU_Thaw(void);

// get disk side number
uint8 PSPEMU_GetDiskSideNum(void);

// set disk side
void PSPEMU_SetDiskSide(uint8 side);

// get disk side
uint8 PSPEMU_GetDiskSide(void);

// apply game genie code
int PSPEMU_ApplyGameGenie(const char *pszFile);

// Apply graphics config
void PSPEMU_ApplyGraphicsConfig(void);

// Apply sound config
void PSPEMU_ApplySoundConfig(void);

// Save ram
void PSPEMU_SaveRAM(void);

// 現在の設定でのフレームレート設定を行う(internal)
void PSPEMU_SetFrameSpeed(void);

// エミュメイン
void PSPEMU_DoFrame(void);

#endif
