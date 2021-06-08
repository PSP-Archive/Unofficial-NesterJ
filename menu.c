#include "main.h"
#include "nes/nes.h"
#include "menu_submenu.h"
#include "nes/nes_config.h"
#include "nes/fileio.h"
#include "inputmanager.h"
#include "screenmanager.h"
#include "colbl.c"

extern u32 new_pad;

int StateSlot=0;

// progress
void draw_load_rom_progress(unsigned long ulTotal, unsigned long ulCurrent)
{
	int nPer = 100 * ulTotal / ulCurrent;
	static int nOldPer = 0;
	if (nOldPer == nPer & 0xFFFFFFFE) {
		return ;
	}
	nOldPer = nPer;
	if(g_bEnableBitmap)
		pgBitBlt(0,0,480,272,1,g_bgBitmap);
	else
		pgFillvram(setting.color[0]);
	// プログレス
	pgDrawFrame(89,121,391,141,setting.color[1]);
	pgFillBox(90,123, 90+nPer*3, 139,setting.color[1]);
	// ％
	char szPer[16];
	_itoa(nPer, szPer);
	_strcat(szPer, "%");
	pgPrint(28,16,setting.color[3],szPer);
	// pgScreenFlipV()を使うとpgWaitVが呼ばれてしまうのでこちらで。
	// プログレスだからちらついても良いよね～
	pgScreenFlip();
}


// by kwn
void save_config(void)
{
	char CfgPath[MAX_PATH];
	char *p;

	GetModulePath(CfgPath, sizeof(CfgPath));
	_strcat(CfgPath, "NESTERJ.CFG");

	int fd;
	fd = sceIoOpen(CfgPath,O_CREAT|O_WRONLY|O_TRUNC, 0777);
	if(fd>=0){
		sceIoWrite(fd, &setting, sizeof(setting));
		sceIoWrite(fd, &g_NESConfig, sizeof(g_NESConfig));
		sceIoClose(fd);
	}
}

// by kwn
void load_config(void)
{
	char CfgPath[MAX_PATH];
	char *p;

	_memset(&setting, 0, sizeof(setting));

	GetModulePath(CfgPath, sizeof(CfgPath));
	_strcat(CfgPath, "NESTERJ.CFG");

	int fd;
	fd = sceIoOpen(CfgPath,O_RDONLY, 0777);
	if(fd>=0){
		sceIoRead(fd, &setting, sizeof(setting));
		sceIoRead(fd, &g_NESConfig, sizeof(g_NESConfig));
		sceIoClose(fd);
		// 簡易エラーチェック
		if(_strcmp(setting.vercnf, NESTERJ_CFG_TAG)) goto SetDefault;
		if(setting.version != NESTERJ_CFG_FORMATVERSION) goto SetDefault;
		if(setting.bgbright<0 || setting.bgbright>100) goto SetDefault;
		if(setting.speedmode1 < SPEEDMODE_MIN || setting.speedmode1 > SPEEDMODE_MAX) goto SetDefault;
		if(setting.speedmode2 < SPEEDMODE_MIN || setting.speedmode2 > SPEEDMODE_MAX) goto SetDefault;
		if(setting.screenmode >= SCREEN_COUNT) goto SetDefault;
		if(setting.samplingrate >= SAMPLINGRATE_COUNT) goto SetDefault;
		if(setting.rapidmode >= RAPID_COUNT) goto SetDefault;
		// 問題なし
		if(setting.key_config[6]==0) setting.key_config[6] = CTRL_TRIANGLE;
		return;
	}
SetDefault:
	_memset(&setting, 0, sizeof(setting));
	_strcpy(setting.vercnf, NESTERJ_CFG_TAG);
	setting.version = NESTERJ_CFG_FORMATVERSION;
	setting.screenmode = 0;
	setting.vsync = FALSE;
	setting.showfps = FALSE;
	setting.savethumbnail = TRUE;
	setkeydefault();
	setting.color[0] = DEF_COLOR0;
	setting.color[1] = DEF_COLOR1;
	setting.color[2] = DEF_COLOR2;
	setting.color[3] = DEF_COLOR3;
	setting.bgbright=BGBRIGHT_DEFAULT;
	setting.analog2dpad=TRUE;
	setting.bToggle    = FALSE;
	setting.speedmode1 = SPEEDMODE1_DEFAULT;
	setting.speedmode2 = SPEEDMODE2_DEFAULT;
	setting.rapidmode = RAPID_15COUNT;
	setting.samplingrate = SAMPLINGRATE_22050;
	NES_Config_SetDefaults_All();
}

void setkeydefault(void)
{
	int i;
	setting.key_config[0] = CTRL_CIRCLE;	// A
	setting.key_config[1] = CTRL_CROSS;		// B
	setting.key_config[2] = CTRL_TRIANGLE;	// A(RAPID)
	setting.key_config[3] = CTRL_SQUARE;	// B(RAPID)
	setting.key_config[4] = CTRL_SELECT;	// SELECT
	setting.key_config[5] = CTRL_START;		// START
	setting.key_config[6] = CTRL_LTRIGGER;	// MENU
	setting.key_config[7] = 0;				// PLAYER2MIC
	setting.key_config[8] = 0;				// SPEEDMODE1
	setting.key_config[9] = 0;				// SPEEDMODE2
	setting.key_config[10] = 0;				// SOUND
	setting.key_config[11] = 0;				// SCREENSIZE
	setting.key_config[12] = CTRL_RTRIGGER|CTRL_SELECT;	// QUICKSAVE
	setting.key_config[13] = CTRL_RTRIGGER|CTRL_START;	// QUICKLOAD
	setting.key_config[14] = 0;				// STATE_SLOT
	setting.key_config[15] = 0;				// BATTERY_METER
	for(i=16; i<20; i++)
		setting.key_config[i] = 0;
}

boolean g_bEnableBitmap;
unsigned short g_bgBitmap[480*272];

// by kwn
void load_menu_bg()
{
	byte *menu_bg;
	unsigned char *vptr;
	static byte menu_bg_buf[480*272*3+0x36];
	char BgPath[MAX_PATH];
	char *p;
 	unsigned short x,y,yy,r,g,b,data;

	GetModulePath(BgPath, sizeof(BgPath));
	_strcat(BgPath, "MENU.BMP");

	int fd;
	fd = sceIoOpen(BgPath,O_RDONLY,0777);
	if(fd>=0){
		sceIoRead(fd, menu_bg_buf, 480*272*3+0x36);
		sceIoClose(fd);

		menu_bg = menu_bg_buf + 0x36;
		vptr=(unsigned char*)g_bgBitmap;
		for(y=0; y<272; y++){
			for(x=0; x<480; x++){
				yy = 271 - y;
				r = *(menu_bg + (yy*480 + x)*3 + 2);
				g = *(menu_bg + (yy*480 + x)*3 + 1);
				b = *(menu_bg + (yy*480 + x)*3);
				data = (((b & 0xf8) << 7) | ((g & 0xf8) << 2) | (r >> 3));
				*(unsigned short *)vptr=data;
				vptr+=2;
			}
		}
		g_bEnableBitmap = TRUE;
	}else{
		g_bEnableBitmap = FALSE;
	}
}

// 半透明処理
unsigned short rgbTransp(unsigned short fgRGB, unsigned short bgRGB, int alpha) {

    unsigned short fgR, fgG, fgB;
    unsigned short bgR, bgG, bgB;
	unsigned short R, G, B;
 	unsigned short rgb;

    fgB = (fgRGB >> 10) & 0x1F;
    fgG = (fgRGB >> 5) & 0x1F;
    fgR = fgRGB & 0x1F;

    bgB = (bgRGB >> 10) & 0x1F;
    bgG = (bgRGB >> 5) & 0x1F;
    bgR = bgRGB & 0x1F;

	R = coltbl[fgR][bgR][alpha/10];
	G = coltbl[fgG][bgG][alpha/10];
	B = coltbl[fgB][bgB][alpha/10];

	rgb = (((B & 0x1F)<<10)+((G & 0x1F)<<5)+((R & 0x1F)<<0)+0x8000);
    return rgb;
}

void bgbright_change()
{
	unsigned short *vptr,rgb;
 	int i;

//	load_menu_bg();
	vptr=g_bgBitmap;
	for(i=0; i<272*480; i++){
			rgb = *vptr;
			*vptr = rgbTransp(rgb, 0x0000, setting.bgbright);
			vptr++;
	}
}

void menu_frame(const char *msg0, const char *msg1)
{
	if(g_bEnableBitmap)
		pgBitBlt(0,0,480,272,1,g_bgBitmap);
	else
		pgFillvram(setting.color[0]);
	mh_print(314, 0, " ■ NesterJ for PSP Ver1.07 ■", setting.color[1]);
	// メッセージなど
	if(msg0!=0) mh_print(17, 14, msg0, setting.color[2]);
	pgDrawFrame(17,25,463,248,setting.color[1]);
	pgDrawFrame(18,26,462,247,setting.color[1]);
	// 操作説明
	if(msg1!=0) mh_print(17, 252, msg1, setting.color[2]);
}

// 最後に選択したファイルのsel位置
int nSelRomFiler = 0, nSelGenieFiler = 0;

void rin_menu(void)
{
	enum
	{
		STATE_SLOT,
		STATE_SAVE,
		STATE_LOAD,
		DISK_CHANGE,
		PREFERENCES_CONFIG,
		GRAPHIC_CONFIG,
		SOUND_CONFIG,
		MENU_COLOR_CONFIG,
		KEY_CONFIG,
		CHEAT_GAME_GENIE,
		LOAD_ROM,
		RESET,
		CONTINUE,
	};
	char msg[256], statefile[MAX_PATH], szSlotNum[128], szTmp[32], statetime[128];
	uint16 thumbnail[112*128];
	boolean bAvailableThumbnail = FALSE;
	boolean bRefreshThumbnail = TRUE;
	int selOld = -1;
	static int sel=0;
	int x, y, ret;
	int bSave, fd, romsize, ramsize;
	int nCursor = 0;
	char *p;
	uint8 disksidenum = PSPEMU_GetDiskSideNum();

	old_pad = 0;
	readpad();
	old_pad = paddata.buttons;

	msg[0]='\0';
	statetime[0]='\0';

	for(;;){
		readpad();
		if(new_pad & CTRL_CIRCLE){
			if(sel == STATE_SLOT){
				StateSlot++;
				if(StateSlot >= STATE_SLOT_MAX) StateSlot=0;
				bRefreshThumbnail = TRUE; // refresh thumbnail
			}else if(sel == STATE_SAVE){
				ret = submenu_stateslot(STATE_SAVE);
				if(ret>=0){
					if(PSPEMU_SaveState(ret)) {
						_strcpy(msg, "State Saved Successfully");
						bRefreshThumbnail = TRUE;
					}
					else
						_strcpy(msg, "State Save Failed");
				}
			}else if(sel == STATE_LOAD){
				ret = submenu_stateslot(STATE_LOAD);
				if(ret>=0){
					if(PSPEMU_LoadState(ret))
						break;
					else
						_strcpy(msg, "State Load Failed");
				}
			}else if(sel == DISK_CHANGE && disksidenum){
				submenu_diskchange();
				msg[0]=0;
			}else if(sel == PREFERENCES_CONFIG){
				submenu_preferencesconfig();
				msg[0]=0;
			}else if(sel == GRAPHIC_CONFIG){
				submenu_graphicsconfig();
				msg[0]=0;
			}else if(sel == SOUND_CONFIG){
				submenu_soundconfig();
				msg[0]=0;
			}else if(sel == MENU_COLOR_CONFIG){
				submenu_menucolorconfig();
				msg[0]=0;
			}else if(sel == KEY_CONFIG){
				submenu_keyconfig();
				msg[0]=0;
			}else if(sel == CHEAT_GAME_GENIE){
				int nRet;
				_strcpy(FilerMsg, "Select Game Genie file");
				nRet = getFilePath(szLastGeniePath, setting.szLastGeniePath, NULL, nSelGenieFiler);
				if (nRet >= 0){
					nSelGenieFiler = nRet;
					// apply game genie code
					if (PSPEMU_ApplyGameGenie(szLastGeniePath)) {
						_strcpy(msg, "Apply Game Genie code(s) Successfully");
					}
					else {
						_strcpy(msg, "Apply Game Genie code(s) Failed");
					}
				}
			}else if(sel == LOAD_ROM){
				int nRet;
				FilerMsg[0]=0;
				nRet = getFilePath(RomPath, setting.szLastPath, (LPEXTENTIONS)&stExtRom, nSelRomFiler);
				if (nRet >= 0){
					nSelRomFiler = nRet;
					// ロム読み込み
					if (NES_loadROM(RomPath)) {
						break;
					}
				}
			}else if(sel == RESET){
				// リセット
				NES_reset(0);
				break;
			}else if(sel == CONTINUE){
				break;
			}
		}else if(new_pad & CTRL_CROSS){
			break;
		}else if(new_pad & CTRL_LEFT){
			if(sel<=STATE_LOAD) sel=LOAD_ROM;
			else if(sel<=DISK_CHANGE) sel=STATE_SLOT;
			else if(sel<=KEY_CONFIG) sel=DISK_CHANGE;
			else if(sel<=CHEAT_GAME_GENIE) sel=PREFERENCES_CONFIG;
			else if(sel<=CONTINUE) sel=CHEAT_GAME_GENIE;
		}else if(new_pad & CTRL_RIGHT){
			if(sel<=STATE_LOAD) sel=DISK_CHANGE;
			else if(sel<=DISK_CHANGE) sel=PREFERENCES_CONFIG;
			else if(sel<=KEY_CONFIG) sel=CHEAT_GAME_GENIE;
			else if(sel<=CHEAT_GAME_GENIE) sel=LOAD_ROM;
			else if(sel<=CONTINUE) sel=STATE_SLOT;
		}else if(setting.key_config[6] && (new_pad&setting.key_config[6])==setting.key_config[6]){
			break;
		}else if(new_pad & CTRL_UP){
			if(sel!=0)
				sel--;
			else
				sel=CONTINUE;
		}else if(new_pad & CTRL_DOWN){
			if(sel!=CONTINUE)	sel++;
			else				sel=0;
		}

		if (bRefreshThumbnail) {
			// thumbnail呼び出し
			char thumbnailPath[MAXPATH];
			GetStatePath(thumbnailPath, sizeof(thumbnailPath));
			_strcat(thumbnailPath, NES_ROM_GetRomName());
			_strcat(thumbnailPath, ".tn0");
			thumbnailPath[_strlen(thumbnailPath)-1] = StateSlot + '0';
			bAvailableThumbnail = LoadThumnailFile(thumbnailPath, (uint16*)thumbnail);
			// get timestamp string
			_strcpy(statefile, NES_ROM_GetRomName());
			_strcat(statefile, ".ss0");
			statefile[_strlen(statefile)-1] = StateSlot + '0';
			GetStateTime(statefile, statetime);
			bRefreshThumbnail = FALSE;
		}

		menu_frame(msg, "○：OK　×：CANCEL");

		x = 2;
		y = 5;

		_itoa(StateSlot, szTmp);
		_strcpy(szSlotNum, "  QUICK SLOT : ");
		_strcat(szSlotNum, szTmp);
		_strcat(szSlotNum, " - ");
		_strcat(szSlotNum, statetime);
		pgPrint(x,y++,setting.color[3], szSlotNum);
		pgPrint(x,y++,setting.color[3],"  STATE SAVE");
		pgPrint(x,y++,setting.color[3],"  STATE LOAD");
		y++;
		pgPrint(x,y++,disksidenum ? setting.color[3]: setting.color[2],"  DISK CHANGE");
		y++;
		pgPrint(x,y++,setting.color[3],"  PREFERENCES CONFIG");
		pgPrint(x,y++,setting.color[3],"  GRAPHICS CONFIG");
		pgPrint(x,y++,setting.color[3],"  SOUND CONFIG");
		pgPrint(x,y++,setting.color[3],"  MENU COLOR CONFIG");
		pgPrint(x,y++,setting.color[3],"  KEY CONFIG");
		y++;
		pgPrint(x,y++,setting.color[3],"  LOAD CHEAT FILE(GAME GENIE)");
		y++;
		pgPrint(x,y++,setting.color[3],"  Back to ROM list");
		pgPrint(x,y++,setting.color[3],"  Reset");
		pgPrint(x,y++,setting.color[3],"  Continue");

		y = sel + 5;

		if (sel >= DISK_CHANGE) y++;
		if (sel >= PREFERENCES_CONFIG) y++;
		if (sel >= CHEAT_GAME_GENIE) y++;
		if (sel >= LOAD_ROM) y++;

		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		if (bAvailableThumbnail && sel == STATE_SLOT) {
			pgDrawFrame(300-1,50-1,300+128,50+112,setting.color[1]);
			pgDrawFrame(300-2,50-2,300+128+1,50+112+1,setting.color[1]);
			pgBitBlt(300,50,128,112,1,(uint16*)thumbnail);
		}

		pgScreenFlipV();
	}

	pgFillvram(0);
	pgScreenFlipV();
	pgFillvram(0);
	pgScreenFlipV();
	pgWaitVn(10);
	_memset(&paddata, 0x00, sizeof(paddata));

	extern int bSleep;
	bSleep = 0;
}
