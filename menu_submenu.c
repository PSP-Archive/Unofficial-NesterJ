/*
    sub menu
*/
#include "main.h"
#include "menu_submenu.h"
#include "screenmanager.h"
#include "inputmanager.h"
#include "nes/nes.h"
#include "nes/nes_config.h"
#include "nes/nes_crc32.h"

extern u32 new_pad; // pg.c

extern struct dirent files[MAX_ENTRY]; // filter.c

// get timestamp string form state file
bool GetStateTime(const char *pszFile, char *pszOut)
{
	char path[MAXPATH], tmp[8];
	int nfiles = 0, fd, j;

	GetStatePath(path, sizeof(path));
	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) break;
		nfiles++;
	}
	sceIoDclose(fd);

	for(j=0; j<nfiles; j++){
		if(!_stricmp(pszFile,files[j].name)){
			_itoa(files[j].mtime.year,tmp);
			_strcpy(pszOut,tmp);
			_strcat(pszOut,"/");

			if(files[j].mtime.mon < 10) _strcat(pszOut,"0");
			_itoa(files[j].mtime.mon,tmp);
			_strcat(pszOut,tmp);
			_strcat(pszOut,"/");

			if(files[j].mtime.mday < 10) _strcat(pszOut,"0");
			_itoa(files[j].mtime.mday,tmp);
			_strcat(pszOut,tmp);
			_strcat(pszOut," ");

			if(files[j].mtime.hour < 10) _strcat(pszOut,"0");
			_itoa(files[j].mtime.hour,tmp);
			_strcat(pszOut,tmp);
			_strcat(pszOut,":");

			if(files[j].mtime.min < 10) _strcat(pszOut,"0");
			_itoa(files[j].mtime.min,tmp);
			_strcat(pszOut,tmp);
			_strcat(pszOut,":");

			if(files[j].mtime.sec < 10) _strcat(pszOut,"0");
			_itoa(files[j].mtime.sec,tmp);
			_strcat(pszOut,tmp);
			return TRUE;
		}
	}
	if(j>=nfiles){
		_strcpy(pszOut,"None");
	}
	return FALSE;
}


int submenu_stateslot(statemethod method)
{
	char msg[STATE_SLOT_MAX+1][32], path[MAXPATH], name[MAXNAME], tmp[8];
	char thumbnailPath[MAXPATH];
	uint16 thumbnail[112*128];
	boolean bThumbnail = FALSE;
	int selOld = -1;
	int x,y,i,j,fd,sel=0;

	GetStatePath(path, sizeof(path));
	_strcpy(name, NES_ROM_GetRomName());
	_strcpy(thumbnailPath, path);
	_strcat(thumbnailPath, name);
	_strcat(thumbnailPath, ".tn0");
	_strcat(name,".ss0");

	int nfiles = 0;
	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) break;
		nfiles++;
	}
	sceIoDclose(fd);

	for(i=0; i<STATE_SLOT_MAX; i++){
		_strcpy(msg[i],"0 - ");
		msg[i][0] = name[_strlen(name)-1] = i + '0';
		for(j=0; j<nfiles; j++){
			if(!_stricmp(name,files[j].name)){
				_itoa(files[j].mtime.year,tmp);
				_strcat(msg[i],tmp);
				_strcat(msg[i],"/");

				if(files[j].mtime.mon < 10) _strcat(msg[i],"0");
				_itoa(files[j].mtime.mon,tmp);
				_strcat(msg[i],tmp);
				_strcat(msg[i],"/");

				if(files[j].mtime.mday < 10) _strcat(msg[i],"0");
				_itoa(files[j].mtime.mday,tmp);
				_strcat(msg[i],tmp);
				_strcat(msg[i]," ");

				if(files[j].mtime.hour < 10) _strcat(msg[i],"0");
				_itoa(files[j].mtime.hour,tmp);
				_strcat(msg[i],tmp);
				_strcat(msg[i],":");

				if(files[j].mtime.min < 10) _strcat(msg[i],"0");
				_itoa(files[j].mtime.min,tmp);
				_strcat(msg[i],tmp);
				_strcat(msg[i],":");

				if(files[j].mtime.sec < 10) _strcat(msg[i],"0");
				_itoa(files[j].mtime.sec,tmp);
				_strcat(msg[i],tmp);

				break;
			}
		}
		if(j>=nfiles){
			_strcat(msg[i],"None");
		}
	}

	for(;;){
		readpad();
		if(new_pad & CTRL_CIRCLE)
			return sel;
		else if(new_pad & CTRL_CROSS)
			return -1;
		else if(new_pad & CTRL_DOWN){
			sel++;
			if(sel>=STATE_SLOT_MAX) sel=0;
		}else if(new_pad & CTRL_UP){
			sel--;
			if(sel<0) sel=STATE_SLOT_MAX-1;
		}else if(new_pad & CTRL_RIGHT){
			sel+=STATE_SLOT_MAX/2;
			if(sel>=STATE_SLOT_MAX) sel=STATE_SLOT_MAX-1;
		}else if(new_pad & CTRL_LEFT){
			sel-=STATE_SLOT_MAX/2;
			if(sel<0) sel=0;
		}

		if (selOld != sel) {
			thumbnailPath[_strlen(thumbnailPath)-1] = sel + '0';
			bThumbnail = LoadThumnailFile(thumbnailPath, (uint16*)thumbnail);
			selOld = sel;
		}

		if(method == STATE_SAVE)
			menu_frame("Select State Save Slot", "○：OK　×：CANCEL");
		else
			menu_frame("Select State Load Slot", "○：OK　×：CANCEL");

		x=4, y=5;
		if(method == STATE_SAVE)
			pgPrint(x++,y++,setting.color[3],"STATE SAVE:");
		else
			pgPrint(x++,y++,setting.color[3],"STATE LOAD:");
		for(i=0; i<STATE_SLOT_MAX; i++){
			if(i==sel)
				pgPrint(x,y++,setting.color[2],msg[i]);
			else
				pgPrint(x,y++,setting.color[3],msg[i]);
		}

		if (bThumbnail) {
			pgDrawFrame(300-1,50-1,300+128,50+112,setting.color[1]);
			pgDrawFrame(300-2,50-2,300+128+1,50+112+1,setting.color[1]);
			pgBitBlt(300,50,128,112,1,(uint16*)thumbnail);
		}

		pgScreenFlipV();
	}
}

void submenu_diskchange(void)
{
	enum
	{
		// 順番変更すると異常動作するので注意
		CONFIG_EJECT = 0,
		CONFIG_1STDISKSIDEA,
		CONFIG_1STDISKSIDEB,
		CONFIG_2NDDISKSIDEA,
		CONFIG_2NDDISKSIDEB,
	};
	char msg[256], szTemp[256];
	int sel, x, y, i, bPad;
	int nCursor = 0;
	uint8 disksidenum = PSPEMU_GetDiskSideNum();
	sel = PSPEMU_GetDiskSide();

	pgWaitVn(15);

	for(;;){
		readpad();
		if(new_pad==CTRL_UP){
			if(sel!=0)	sel--;
			else		sel=CONFIG_2NDDISKSIDEB;
		}else if(new_pad==CTRL_DOWN){
			if(sel!=CONFIG_2NDDISKSIDEB)sel++;
			else				sel=0;
		}else if(new_pad & CTRL_CROSS){
			break;
		}else if(new_pad & CTRL_CIRCLE){
			if(sel==CONFIG_EJECT) {
				PSPEMU_SetDiskSide(0x00);
				break;
			} else if(sel==CONFIG_1STDISKSIDEA && disksidenum >= 1) {
				PSPEMU_SetDiskSide(0x01);
				break;
			} else if(sel==CONFIG_1STDISKSIDEB && disksidenum >= 2) {
				PSPEMU_SetDiskSide(0x02);
				break;
			} else if(sel==CONFIG_2NDDISKSIDEA && disksidenum >= 3) {
				PSPEMU_SetDiskSide(0x03);
				break;
			} else if(sel==CONFIG_2NDDISKSIDEB && disksidenum >= 4) {
				PSPEMU_SetDiskSide(0x04);
				break;
			}
		}else if(new_pad & CTRL_LEFT){
			if(sel<=CONFIG_EJECT) sel=CONFIG_1STDISKSIDEA;
			else if(sel<=CONFIG_2NDDISKSIDEB) sel=CONFIG_EJECT;
		}else if(new_pad & CTRL_RIGHT){
			if(sel<=CONFIG_EJECT) sel=CONFIG_1STDISKSIDEA;
			else if(sel<=CONFIG_1STDISKSIDEA) sel=CONFIG_EJECT;
		}

		_strcpy(msg,"○：OK  ×：Return to Main Menu");

		menu_frame(0, msg);

		x=2; y=5;
		pgPrint(x,y++,setting.color[3],"  DISK EJECT");
		y++;
		pgPrint(x,y++,(disksidenum >= 1) ? setting.color[3]: setting.color[2],"  CHANGE 1ST DISK SIDE A");
		pgPrint(x,y++,(disksidenum >= 2) ? setting.color[3]: setting.color[2],"  CHANGE 1ST DISK SIDE B");
		pgPrint(x,y++,(disksidenum >= 3) ? setting.color[3]: setting.color[2],"  CHANGE 2ND DISK SIDE A");
		pgPrint(x,y++,(disksidenum >= 4) ? setting.color[3]: setting.color[2],"  CHANGE 2ND DISK SIDE B");

		x = 2;
		y = sel + 5;
		if(sel >= CONFIG_1STDISKSIDEA)       y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
}

uint8 g_BiosCheckBuf[0x2000];

void DiskSystemBiosCheck(void)
{
	extern char pg_workdir[];
	uint32 ulReadSize;
	int hFile;
	uint32 ulAccBytes;

	char szBiosPath[MAX_PATH];
	_strcpy(szBiosPath, pg_workdir);
	_strcat(szBiosPath, "DISKSYS.ROM");

	if((hFile = sceIoOpen(szBiosPath, O_RDONLY, 0777)) >= 0) {
		uint8 head[3];
		ulReadSize = sceIoRead(hFile, head, sizeof(head));
		if(head[0] == 'N' && head[1] == 'E' && head[2] == 'S') {
			sceIoLseek(hFile, 0x6010, 0);
		}
		else {
			sceIoLseek(hFile, 0, 0);
		}
		_memset(g_BiosCheckBuf, 0x00, sizeof(g_BiosCheckBuf));
		ulReadSize = sceIoRead(hFile, g_BiosCheckBuf, sizeof(g_BiosCheckBuf));
		sceIoClose(hFile);
		ulAccBytes = CrcCalc(g_BiosCheckBuf, sizeof(g_BiosCheckBuf));
		if (ulAccBytes == 0x5e607dcf) {
			MessageBox("FDS BIOS is correct", MB_OK);
		}
		else {
			MessageBox("FDS BIOS is Incorrect", MB_OK);
		}
	}
	else
	{
		MessageBox("FDS BIOS FILE NOT FOUND!", MB_OK);
		return;
	}
}

void submenu_preferencesconfig(void)
{
	enum
	{
		CONFIG_USEROMDATABASE = 0,
		CONFIG_SKIPDISKACCESS,
		CONFIG_SAVESTATETHUMBNAIL,
		CONFIG_TVMODE,
		CONFIG_RAPIDMODE,
		CONFIG_SPEEDMODETOGGLE,
		CONFIG_SPEEDMODE1,
		CONFIG_SPEEDMODE2,
		CONFIG_CPUFREQUENCY,
		CONFIG_SHOWFPS,
		CONFIG_DISKSYSROMCHECK,
		CONFIG_END = CONFIG_DISKSYSROMCHECK
	};
	char msg[256], szTemp[256];
	int sel=0, x, y, i, bPad;
	int nCursor = 0;
	int nPushCounter = 0;
	int nDelta;

	pgWaitVn(15);

	for(;;){
		readpad();
		if (!now_pad) nPushCounter = 0;
		if(new_pad==CTRL_LEFT) {
			nPushCounter++;
			nDelta = (nPushCounter >=20) ? 5:1;
			if (sel == CONFIG_SPEEDMODE1) {
				setting.speedmode1-=nDelta;
				if (setting.speedmode1 < SPEEDMODE_MIN) setting.speedmode1 = SPEEDMODE_MIN;
			}
			if (sel == CONFIG_SPEEDMODE2) {
				setting.speedmode2-=nDelta;
				if (setting.speedmode2 < SPEEDMODE_MIN) setting.speedmode2 = SPEEDMODE_MIN;
			}
			if (sel == CONFIG_CPUFREQUENCY) {
				setting.cpufrequency-=nDelta;
				if (setting.cpufrequency < 222) setting.cpufrequency = 222;
			}
		}
		else if (new_pad==CTRL_RIGHT){
			nPushCounter++;
			nDelta = (nPushCounter >=20) ? 5:1;
			if (sel == CONFIG_SPEEDMODE1) {
				setting.speedmode1+=nDelta;
				if (setting.speedmode1 > SPEEDMODE_MAX) setting.speedmode1 = SPEEDMODE_MAX;
			}
			if (sel == CONFIG_SPEEDMODE2) {
				setting.speedmode2+=nDelta;
				if (setting.speedmode2 > SPEEDMODE_MAX) setting.speedmode2 = SPEEDMODE_MAX;
			}
			if (sel == CONFIG_CPUFREQUENCY) {
				setting.cpufrequency+=nDelta;
				if (setting.cpufrequency > 333) setting.cpufrequency = 333;
			}
		}else if(new_pad==CTRL_UP){
			if(sel!=0)	sel--;
			else		sel=CONFIG_END;
		}else if(new_pad==CTRL_DOWN){
			if(sel!=CONFIG_END)sel++;
			else				sel=0;
		}else if(new_pad & CTRL_CROSS){
			break;
		}else if(new_pad & CTRL_CIRCLE){
			if(sel==CONFIG_USEROMDATABASE)
				g_NESConfig.preferences.UseRomDataBase = g_NESConfig.preferences.UseRomDataBase ? FALSE: TRUE;
			else if(sel==CONFIG_SKIPDISKACCESS)
				g_NESConfig.preferences.UseFDSDiskASkip = g_NESConfig.preferences.UseFDSDiskASkip ? FALSE: TRUE;
			else if(sel==CONFIG_SAVESTATETHUMBNAIL)
				setting.savethumbnail = setting.savethumbnail ? FALSE: TRUE;
			else if(sel==CONFIG_TVMODE)
				g_NESConfig.preferences.TV_Mode = (g_NESConfig.preferences.TV_Mode + 1) % 3;
			else if(sel==CONFIG_RAPIDMODE)
				setting.rapidmode = (setting.rapidmode + 1) % RAPID_COUNT;
			else if(sel==CONFIG_SPEEDMODETOGGLE)
				setting.bToggle = setting.bToggle ? FALSE: TRUE;
			else if(sel==CONFIG_DISKSYSROMCHECK)
				DiskSystemBiosCheck();
			else if(sel==CONFIG_SHOWFPS)
				setting.showfps = setting.showfps ? FALSE: TRUE;
		}

		_strcpy(msg,"○：OK  ×：Return to Main Menu");

		menu_frame(0, msg);

		x=2; y=5;
		_strcpy(msg, "  EXTERNAL ROM DATABASE : ");
		_strcat(msg, g_NESConfig.preferences.UseRomDataBase ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  SKIP DISK ACCESS      : ");
		_strcat(msg, g_NESConfig.preferences.UseFDSDiskASkip ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  SAVE STATE THUMBNAIL  : ");
		_strcat(msg, setting.savethumbnail ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  TV MODE               : ");
		_strcat(msg, g_NESConfig.preferences.TV_Mode ?
			(g_NESConfig.preferences.TV_Mode == 2)? "PAL":"NTSC":"AUTO");
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  RAPID MODE            : ");
		_strcat(msg, aszRapidModeName[setting.rapidmode]);
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  SPEED MODE TOGGLE     : ");
		_strcat(msg, setting.bToggle ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  SPEED MODE 1          : ");
		_itoa(setting.speedmode1, szTemp);
		_strcat(msg, szTemp);
		_strcat(msg, " FPS");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  SPEED MODE 2          : ");
		_itoa(setting.speedmode2, szTemp);
		_strcat(msg, szTemp);
		_strcat(msg, " FPS");
		pgPrint(x,y++,setting.color[3],msg);
		y+=2;
		_strcpy(msg, "  CPU FREQUENCY         : ");
		_itoa(setting.cpufrequency, szTemp);
		_strcat(msg, szTemp);
		_strcat(msg, " MHz");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  SHOW FPS              : ");
		_strcat(msg, setting.showfps ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		pgPrint(x,y++,setting.color[3],"  CHECK FDS BIOS");

		x = 2;
		y = sel + 5;
		if(sel >= CONFIG_RAPIDMODE) y++;
		if(sel >= CONFIG_SPEEDMODETOGGLE) y++;
		if(sel >= CONFIG_CPUFREQUENCY) y+=2;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
	scePowerSetClockFrequency(setting.cpufrequency,setting.cpufrequency,setting.cpufrequency/2);
}

int MessageBox(unsigned char*pszMsg, uint32 ulTypes)
{
	char msg[256];
	int sel=0, x, y, i, bPad;
	int nRet = IDOK;

	pgWaitVn(15);

	for(;;){
		readpad();
		if(new_pad){
			break;
		}

		_strcpy(msg,"○：OK");

		menu_frame(0, msg);

		_memset(msg, 0x00, sizeof(msg));
		_strncpy(msg, pszMsg, 60);
		x = 30 - (_strlen(pszMsg) / 2);
		y=15;
		pgPrint(x,y,setting.color[3],msg);

		pgScreenFlipV();
	}
}


void submenu_graphicsconfig(void)
{
	enum
	{
		CONFIG_EXTENDSPRITES = 0,
		CONFIG_BLACKANDWHITE,
		CONFIG_SCREENMODE,
		CONFIG_WAITVBLANKSTART,
	};
	char msg[256], szTemp[256];
	int sel=0, x, y, i, bPad;
	int nCursor = 0;

	pgWaitVn(15);

	for(;;){
		readpad();
		if(new_pad==CTRL_UP){
			if(sel!=0)	sel--;
			else		sel=CONFIG_WAITVBLANKSTART;
		}else if(new_pad==CTRL_DOWN){
			if(sel!=CONFIG_WAITVBLANKSTART)sel++;
			else				sel=0;
		}else if(new_pad & CTRL_CROSS){
			break;
		}else if(new_pad & CTRL_CIRCLE){
			if(sel==CONFIG_EXTENDSPRITES)
				g_NESConfig.graphics.show_more_than_8_sprites = g_NESConfig.graphics.show_more_than_8_sprites ? FALSE: TRUE;
			else if(sel==CONFIG_BLACKANDWHITE)
				g_NESConfig.graphics.black_and_white = g_NESConfig.graphics.black_and_white ? FALSE: TRUE;
			else if(sel==CONFIG_SCREENMODE)
				setting.screenmode = (setting.screenmode + 1) % SCREEN_COUNT;
			else if(sel==CONFIG_WAITVBLANKSTART)
				setting.vsync = setting.vsync ? FALSE: TRUE;
		}else if(new_pad & CTRL_LEFT){
			if(sel<=CONFIG_BLACKANDWHITE) sel=CONFIG_WAITVBLANKSTART;
			else if(sel<=CONFIG_SCREENMODE) sel=CONFIG_EXTENDSPRITES;
			else if(sel<=CONFIG_WAITVBLANKSTART) sel=CONFIG_SCREENMODE;
		}else if(new_pad & CTRL_RIGHT){
			if(sel<=CONFIG_BLACKANDWHITE) sel=CONFIG_SCREENMODE;
			else if(sel<=CONFIG_SCREENMODE) sel=CONFIG_WAITVBLANKSTART;
			else if(sel<=CONFIG_WAITVBLANKSTART) sel=CONFIG_EXTENDSPRITES;
		}

		_strcpy(msg,"○：OK  ×：Return to Main Menu");

		menu_frame(0, msg);

		x=2; y=5;
		_strcpy(msg, "  EXTEND SPRITES MODE  : ");
		_strcat(msg, g_NESConfig.graphics.show_more_than_8_sprites ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		_strcpy(msg, "  BLACK AND WHITE MODE : ");
		_strcat(msg, g_NESConfig.graphics.black_and_white ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  SCREEN SIZE          : ");
		_strcat(msg, aszScreenName[setting.screenmode]);
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  WAIT VBLANK(VSYNC)   : ");
		_strcat(msg, setting.vsync ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);

		x = 2;
		y = sel + 5;
		if(sel >= CONFIG_SCREENMODE)            y++;
		if(sel >= CONFIG_WAITVBLANKSTART)       y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
	PSPEMU_ApplyGraphicsConfig();
}

void submenu_soundconfig(void)
{
	enum
	{
		CONFIG_ENABLESOUND = 0,
		CONFIG_ENABLERECTANGLE1,
		CONFIG_ENABLERECTANGLE2,
		CONFIG_ENABLETRIANGLE,
		CONFIG_ENABLENOISE,
		CONFIG_ENABLEDPCM,
		CONFIG_ENABLEEXTRACHANNEL,
		CONFIG_SAMPLINGRATE,
		CONFIG_LAST = CONFIG_SAMPLINGRATE
	};
	char msg[256], szTemp[256];
	int sel=0, x, y, i, bPad;
	int nCursor = 0;

	pgWaitVn(15);

	for(;;){
		readpad();
		if(new_pad==CTRL_UP){
			if(sel!=0)	sel--;
			else		sel=CONFIG_LAST;
		}else if(new_pad==CTRL_DOWN){
			if(sel!=CONFIG_LAST)sel++;
			else				sel=0;
		}else if(new_pad & CTRL_CROSS){
			break;
		}else if(new_pad & CTRL_CIRCLE){
			if(sel==CONFIG_ENABLESOUND)
				g_NESConfig.sound.enabled = g_NESConfig.sound.enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLERECTANGLE1)
				g_NESConfig.sound.rectangle1_enabled = g_NESConfig.sound.rectangle1_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLERECTANGLE2)
				g_NESConfig.sound.rectangle2_enabled = g_NESConfig.sound.rectangle2_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLETRIANGLE)
				g_NESConfig.sound.triangle_enabled = g_NESConfig.sound.triangle_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLENOISE)
				g_NESConfig.sound.noise_enabled = g_NESConfig.sound.noise_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLEDPCM)
				g_NESConfig.sound.dpcm_enabled = g_NESConfig.sound.dpcm_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_ENABLEEXTRACHANNEL)
				g_NESConfig.sound.ext_enabled = g_NESConfig.sound.ext_enabled ? FALSE: TRUE;
			else if(sel==CONFIG_SAMPLINGRATE)
				setting.samplingrate = (setting.samplingrate + 1) % SAMPLINGRATE_COUNT;
		}else if(new_pad & CTRL_LEFT){
			if(sel<=CONFIG_ENABLESOUND) sel=CONFIG_SAMPLINGRATE;
			else if(sel<=CONFIG_ENABLEEXTRACHANNEL) sel=CONFIG_ENABLESOUND;
			else if(sel<=CONFIG_SAMPLINGRATE) sel=CONFIG_ENABLERECTANGLE1;
		}else if(new_pad & CTRL_RIGHT){
			if(sel<=CONFIG_ENABLESOUND) sel=CONFIG_ENABLERECTANGLE1;
			else if(sel<=CONFIG_ENABLEEXTRACHANNEL) sel=CONFIG_SAMPLINGRATE;
			else if(sel<=CONFIG_SAMPLINGRATE) sel=CONFIG_ENABLESOUND;
		}

		_strcpy(msg,"○：OK  ×：Return to Main Menu");

		menu_frame(0, msg);

		x=2; y=5;
		_strcpy(msg, "  SOUND  : ");
		_strcat(msg, g_NESConfig.sound.enabled ? "ENABLE":"DISABLE");
		pgPrint(x,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  RECTANGLE 1   : ");
		_strcat(msg, g_NESConfig.sound.rectangle1_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "  RECTANGLE 2   : ");
		_strcat(msg, g_NESConfig.sound.rectangle2_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "  TRIANGLE      : ");
		_strcat(msg, g_NESConfig.sound.triangle_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "  NOISE         : ");
		_strcat(msg, g_NESConfig.sound.noise_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "  DPCM          : ");
		_strcat(msg, g_NESConfig.sound.dpcm_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		_strcpy(msg, "  EXTRA CHANNEL : ");
		_strcat(msg, g_NESConfig.sound.ext_enabled ? "ENABLE":"DISABLE");
		pgPrint(x+2,y++,setting.color[3],msg);
		y++;
		_strcpy(msg, "  SAMPLING RATE   : ");
		_strcat(msg, aszSamplingRateName[setting.samplingrate]);
		pgPrint(x,y++,setting.color[3],msg);

		x = 2;
		y = sel + 5;
		if(sel >= CONFIG_ENABLERECTANGLE1)       y++;
		if (sel >= CONFIG_ENABLERECTANGLE1 && sel <= CONFIG_ENABLEEXTRACHANNEL) x+=2;
		if(sel >= CONFIG_SAMPLINGRATE)       y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
	PSPEMU_ApplySoundConfig();
}






void submenu_menucolorconfig(void)
{
	enum
	{
		COLOR0_R=0,
		COLOR0_G,
		COLOR0_B,
		COLOR1_R,
		COLOR1_G,
		COLOR1_B,
		COLOR2_R,
		COLOR2_G,
		COLOR2_B,
		COLOR3_R,
		COLOR3_G,
		COLOR3_B,
		BG_BRIGHT,
		EXIT,
		INIT,
	};
	char tmp[4], msg[256];
	int color[4][3];
	int sel=0, x, y, i;
	int nCursor = 0;

	_memset(color, 0, sizeof(int)*4*3);
	for(i=0; i<4; i++){
		color[i][2] = setting.color[i]>>10 & 0x1F;
		color[i][1] = setting.color[i]>>5 & 0x1F;
		color[i][0] = setting.color[i] & 0x1F;
	}

	for(;;){
		readpad();
		if(new_pad & CTRL_CIRCLE){
			if(sel==EXIT){
				break;
			}else if(sel==INIT){
				color[0][2] = DEF_COLOR0>>10 & 0x1F;
				color[0][1] = DEF_COLOR0>>5 & 0x1F;
				color[0][0] = DEF_COLOR0 & 0x1F;
				color[1][2] = DEF_COLOR1>>10 & 0x1F;
				color[1][1] = DEF_COLOR1>>5 & 0x1F;
				color[1][0] = DEF_COLOR1 & 0x1F;
				color[2][2] = DEF_COLOR2>>10 & 0x1F;
				color[2][1] = DEF_COLOR2>>5 & 0x1F;
				color[2][0] = DEF_COLOR2 & 0x1F;
				color[3][2] = DEF_COLOR3>>10 & 0x1F;
				color[3][1] = DEF_COLOR3>>5 & 0x1F;
				color[3][0] = DEF_COLOR3 & 0x1F;
				setting.bgbright = 100;
				if(g_bEnableBitmap){
					load_menu_bg();
					bgbright_change();
				}
			}else if(sel == BG_BRIGHT) {
				//輝度変更
				setting.bgbright += 10;
				if(setting.bgbright > 100) setting.bgbright=0;
				if(g_bEnableBitmap){
					load_menu_bg();
					bgbright_change();
				}
			}else{
				if(color[sel/3][sel%3]<31)
					color[sel/3][sel%3]++;
			}
		}else if(new_pad & CTRL_CROSS){
			if(sel == BG_BRIGHT) {
				//輝度変更
				setting.bgbright -= 10;
				if(setting.bgbright < 0) setting.bgbright=100;
				if(g_bEnableBitmap){
					load_menu_bg();
					bgbright_change();
				}
			}else if(sel>=COLOR0_R && sel<=COLOR3_B){
				if(color[sel/3][sel%3]>0)
					color[sel/3][sel%3]--;
			}
		}else if(new_pad & CTRL_UP){
			if(sel!=0)	sel--;
			else		sel=INIT;
		}else if(new_pad & CTRL_DOWN){
			if(sel!=INIT)	sel++;
			else			sel=0;
		}else if(new_pad & CTRL_RIGHT){
			if(sel<COLOR1_R) 		sel=COLOR1_R;
			else if(sel<COLOR2_R)	sel=COLOR2_R;
			else if(sel<COLOR3_R)	sel=COLOR3_R;
			else if(sel<BG_BRIGHT)	sel=BG_BRIGHT;
			else if(sel<EXIT)		sel=EXIT;
		}else if(new_pad & CTRL_LEFT){
			if(sel>BG_BRIGHT)		sel=BG_BRIGHT;
			else if(sel>COLOR3_B)	sel=COLOR3_R;
			else if(sel>COLOR2_B)	sel=COLOR2_R;
			else if(sel>COLOR1_B)	sel=COLOR1_R;
			else					sel=COLOR0_R;
		}

		for(i=0; i<4; i++)
			setting.color[i]=color[i][2]<<10|color[i][1]<<5|color[i][0]|0x8000;

		x = 2;
		y = 5;

		if(sel>=COLOR0_R && sel<=BG_BRIGHT)
			_strcpy(msg, "○：Add  ×：Sub");
		else
			_strcpy(msg, "○：OK");

		menu_frame(0, msg);

		pgPrint(x,y++,setting.color[3],"  COLOR0 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR0 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR0 B:");
		y++;
		pgPrint(x,y++,setting.color[3],"  COLOR1 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR1 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR1 B:");
		y++;
		pgPrint(x,y++,setting.color[3],"  COLOR2 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR2 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR2 B:");
		y++;
		pgPrint(x,y++,setting.color[3],"  COLOR3 R:");
		pgPrint(x,y++,setting.color[3],"  COLOR3 G:");
		pgPrint(x,y++,setting.color[3],"  COLOR3 B:");
		y++;
		if(setting.bgbright / 100 == 1)
			pgPrint(x,y++,setting.color[3],"  BG BRIGHT:100%");
		else
			pgPrint(x,y++,setting.color[3],"  BG BRIGHT:  0%");
		if(setting.bgbright % 100 != 0)			// 10%～90%
			pgPutChar((x+13)*8,(y-1)*8,setting.color[3],0,'0'+setting.bgbright/10,1,0,1);
		y++;
		pgPrint(x,y++,setting.color[3],"  Return to Main Menu");
		pgPrint(x,y++,setting.color[3],"  Initialize");

		x=14; y=5;
		for(i=0; i<12; i++){
			if(i!=0 && i%3==0) y++;
			_itoa(color[i/3][i%3], tmp);
			pgPrint(x,y++,setting.color[3],tmp);
		}

		x = 2;
		y = sel + 5;
		if(sel>=COLOR1_R) y++;
		if(sel>=COLOR2_R) y++;
		if(sel>=COLOR3_R) y++;
		if(sel>=BG_BRIGHT) y++;
		if(sel>=EXIT) y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}
}

// by kwn
void submenu_keyconfig(void)
{
	enum
	{
		CONFIG_A = 0,
		CONFIG_B,
		CONFIG_RAPIDA,
		CONFIG_RAPIDB,
		CONFIG_SELECT,
		CONFIG_START,
		CONFIG_MENU,
		CONFIG_PLAYER2MIC,
		CONFIG_SPEEDMODE1,
		CONFIG_SPEEDMODE2,
		CONFIG_SOUND,
		CONFIG_SCREENSIZE,
		CONFIG_QUICKSAVE,
		CONFIG_QUICKLOAD,
		CONFIG_STATE_SLOT,
		CONFIG_BATTERY_METER,
		CONFIG_ANALOG2DPAD,
		CONFIG_EXIT,
		CONFIG_INIT,
	};
	char msg[256];
	int sel=0, x, y, i, bPad;
	int nCursor = 0;

	pgWaitVn(15);

	for(;;){
		readpad();
		if(now_pad==CTRL_LEFT || now_pad==CTRL_RIGHT){
			if(sel!=CONFIG_EXIT && sel!=CONFIG_MENU && sel!=CONFIG_INIT && sel!=CONFIG_ANALOG2DPAD)
				setting.key_config[sel] = 0;
		}else if(now_pad==CTRL_UP){
			if(bPad==0){
				if(sel!=0)	sel--;
				else		sel=CONFIG_INIT;
				bPad++;
			}else if(bPad >= 25){
				if(sel!=0)	sel--;
				else		sel=CONFIG_INIT;
				bPad=20;
			}else
				bPad++;
		}else if(now_pad==CTRL_DOWN){
			if(bPad==0){
				if(sel!=CONFIG_INIT)sel++;
				else				sel=0;
				bPad++;
			}else if(bPad >= 25){
				if(sel!=CONFIG_INIT)sel++;
				else				sel=0;
				bPad=20;
			}else
				bPad++;
		}else if(new_pad != 0){
			if(sel==CONFIG_INIT && new_pad&CTRL_CIRCLE){
				setkeydefault();
			}
			else if(sel==CONFIG_EXIT && new_pad&CTRL_CIRCLE)
				break;
			else if(sel==CONFIG_ANALOG2DPAD && new_pad&CTRL_CIRCLE)
				setting.analog2dpad = !setting.analog2dpad;
			else
				setting.key_config[sel] = now_pad;
		}else{
			bPad=0;
		}

		if(sel>=CONFIG_ANALOG2DPAD)
			_strcpy(msg,"○：OK");
		else
			_strcpy(msg,"");

		menu_frame(0, msg);

		x=2; y=5;
		pgPrint(x,y++,setting.color[3],"  A BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  B BUTTON       :");
		pgPrint(x,y++,setting.color[3],"  A BUTTON(RAPID):");
		pgPrint(x,y++,setting.color[3],"  B BUTTON(RAPID):");
		pgPrint(x,y++,setting.color[3],"  SELECT BUTTON  :");
		pgPrint(x,y++,setting.color[3],"  START BUTTON   :");
		pgPrint(x,y++,setting.color[3],"  MENU BUTTON    :");
		pgPrint(x,y++,setting.color[3],"  PLAYER2 MIC    :");
		pgPrint(x,y++,setting.color[3],"  SPEED MODE 1   :");
		pgPrint(x,y++,setting.color[3],"  SPEED MODE 2   :");
		pgPrint(x,y++,setting.color[3],"  SOUND ON/OFF   :");
		pgPrint(x,y++,setting.color[3],"  SCREEN SIZE    :");
		pgPrint(x,y++,setting.color[3],"  QUICK SAVE     :");
		pgPrint(x,y++,setting.color[3],"  QUICK LOAD     :");
		pgPrint(x,y++,setting.color[3],"  STATE SLOT     :");
		pgPrint(x,y++,setting.color[3],"  BATTERY LIFE   :");
		y++;
		if(setting.analog2dpad)
			pgPrint(x,y++,setting.color[3],"  AnalogPad to D-Pad: ON");
		else
			pgPrint(x,y++,setting.color[3],"  AnalogPad to D-Pad: OFF");
		y++;
		pgPrint(x,y++,setting.color[3],"  Return to Main Menu");
		pgPrint(x,y++,setting.color[3],"  Initialize");

		for (i=0; i<CONFIG_ANALOG2DPAD; i++){
			y = i + 5;
			int j = 0;
			msg[0]=0;
			if(setting.key_config[i] == 0){
				_strcpy(msg,"UNDEFINED");
			}else{
				if (setting.key_config[i] & CTRL_LTRIGGER){
					msg[j++]='L'; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_RTRIGGER){
					msg[j++]='R'; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_CIRCLE){
					msg[j++]=1; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_CROSS){
					msg[j++]=2; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_SQUARE){
					msg[j++]=3; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_TRIANGLE){
					msg[j++]=4; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_START){
					_strcat(msg,"START+"); j+=6;
				}
				if (setting.key_config[i] & CTRL_SELECT){
					_strcat(msg,"SELECT+"); j+=7;
				}
				if (setting.key_config[i] & CTRL_UP){
					msg[j++]=5; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_RIGHT){
					msg[j++]=6; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_DOWN){
					msg[j++]=7; msg[j++]='+'; msg[j]=0;
				}
				if (setting.key_config[i] & CTRL_LEFT){
					msg[j++]=8; msg[j++]='+'; msg[j]=0;
				}
				msg[_strlen(msg)-1]=0;
			}
			pgPrint(21,y,setting.color[3],msg);
		}

		x = 2;
		y = sel + 5;
		if(sel >= CONFIG_ANALOG2DPAD) y++;
		if(sel >= CONFIG_EXIT)        y++;
		if (nCursor/5) pgPutChar((x+1)*8,y*8,setting.color[3],0,127,1,0,1);
		nCursor = (nCursor + 1 ) %10;

		pgScreenFlipV();
	}

}
