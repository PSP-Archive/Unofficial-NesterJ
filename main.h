#ifndef MAIN_H
#define MAIN_H

#include "syscall.h"
#include "nes/types.h"
#include "filer.h"
#include "menu.h"
#include "pg.h"
#include "sound.h"
#include "string.h"

// 注意：下記構造体を変更したら NESTERJ_CFG_FORMATVERSION の値を変更して下さい。
// 変更しないとそれ以前のNESTERJ.CFGの入っているPSPでハングアップする恐れあり。

typedef struct
{
	char vercnf[16];
	int version;		// CFG file format version
	int vsync;			// wait vbrank start
	int screenmode;		// screen mode
	int showfps;		// show fps
	int key_config[20]; //余分に確保
	unsigned long color[4];
	int bgbright;
	int analog2dpad;
	int bToggle;		// toggle flag(speed mode1 speed mode2)
	int speedmode1;		// speed mode 1
	int speedmode2;		// speed mode 2
	int savethumbnail;	// save thumbnail
	int samplingrate;	// sampling rate
	int rapidmode;		// rapid mode
	int cpufrequency;   // CPU frequency(MHz)
	char szLastPath[MAX_PATH];
	char szLastGeniePath[MAX_PATH];
} SETTING;

// NESTERJ.CFGのフォーマット識別用(最大15文字まで)
// 毎回リリースごとに変更する必要なし。
#define NESTERJ_CFG_TAG "NesterJCFG"
#define NESTERJ_CFG_FORMATVERSION 102

enum{
	DEF_COLOR0=0x9063,
	DEF_COLOR1=RGB(85,85,95),
	DEF_COLOR2=RGB(105,105,115),
	DEF_COLOR3=0xffff,
};

#define BGBRIGHT_DEFAULT 100

#define SPEEDMODE_MIN 20
#define SPEEDMODE_MAX 300
#define SPEEDMODE1_DEFAULT 40
#define SPEEDMODE2_DEFAULT 80

extern char RomPath[];
extern char szLastGeniePath[];
extern SETTING setting;
extern const EXTENTIONS stExtRom[];

#endif
