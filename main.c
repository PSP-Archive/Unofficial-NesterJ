#include "main.h"
#include "nes/nes.h"
#include "nes/nes_config.h"
#include "nes/fileio.h"
#include "menu_submenu.h"

int bSleep=0;

char RomPath[MAX_PATH];
char szLastGeniePath[MAX_PATH];
SETTING setting;

const EXTENTIONS stExtRom[] = {
 "nes",EXT_NES,
 "fds",EXT_NES,
 "fam",EXT_NES,
 "unf",EXT_NES,
 "nsf",EXT_NES,
 "zip",EXT_ZIP,
 NULL, EXT_UNKNOWN
};

extern 	unsigned char g_PadBit[6];	// pad state for NES

// ----------------------------------------------------------------------------------------
int exit_callback(void)
{
	char *p;

	bSleep=1;

// Cleanup the games resources etc (if required)
	save_config();

	PSPEMU_SaveRAM();
	// 一応クロック戻す
	scePowerSetClockFrequency(222,222,111);
// Exit game
	sceKernelExitGame();

	return 0;
}

#define POWER_CB_POWER		0x80000000
#define POWER_CB_HOLDON		0x40000000
#define POWER_CB_STANDBY	0x00080000
#define POWER_CB_RESCOMP	0x00040000
#define POWER_CB_RESUME		0x00020000
#define POWER_CB_SUSPEND	0x00010000
#define POWER_CB_EXT		0x00001000
#define POWER_CB_BATLOW		0x00000100
#define POWER_CB_BATTERY	0x00000080
#define POWER_CB_BATTPOWER	0x0000007F
void power_callback(int unknown, int pwrflags)
{
	int cbid;
	char *p;

	// Combine pwrflags and the above defined masks
	if(pwrflags & POWER_CB_POWER){
		bSleep=1;
		save_config();
		PSPEMU_SaveRAM();
	}

	// コールバック関数の再登録
	cbid = sceKernelCreateCallback("Power Callback", power_callback);
	scePowerRegisterCallback(0, cbid);
}

// Thread to create the callbacks and then begin polling
int CallbackThread(int args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback);
	SetExitCallback(cbid);
	cbid = sceKernelCreateCallback("Power Callback", power_callback);
	scePowerRegisterCallback(0, cbid);

	// ポーリング
	KernelPollCallbacks();
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}

// ----------------------------------------------------------------------------------------

int xmain(int argc, char *argv)
{
	int tid;
	char dir[MAX_PATH];
	_memset(RomPath, 0x00, sizeof(RomPath));
	_memset(szLastGeniePath, 0x00, sizeof(szLastGeniePath));

	pgMain(argc, argv);

	tid = SetupCallbacks();
	pgInit();
	pgScreenFrame(2,0);
	sceCtrlInit(0);
	wavoutInit();

	PSPEMU_NES_Init();
	NES_Config_SetDefaults_All();

	load_config();
	load_menu_bg();
	bgbright_change();
	wavout_enable=1;

	// mkdir save and state
	GetModulePath(dir, sizeof(dir));
	_strcat(dir,"SAVE");
	sceIoMkdir(dir,0777);
	GetModulePath(dir, sizeof(dir));
	_strcat(dir,"STATE");
	sceIoMkdir(dir,0777);

	if (_strlen(setting.szLastPath) < 5) {
		GetModulePath(setting.szLastPath, sizeof(setting.szLastPath));
	}
	if (_strlen(setting.szLastGeniePath) < 5) {
		GetModulePath(setting.szLastGeniePath, sizeof(setting.szLastGeniePath));
	}
	FilerMsg[0]=0;

	_memset(g_PadBit, 0x00, sizeof(g_PadBit));
	// パッド設定
	NES_set_pad(g_PadBit);

	// CPU Frequency変更
	if (setting.cpufrequency < 222 || setting.cpufrequency > 333) setting.cpufrequency = 222;
	scePowerSetClockFrequency(setting.cpufrequency,setting.cpufrequency,setting.cpufrequency/2);

	PSPEMU_Freeze();
	for(;;){
		extern int nSelRomFiler; // menu.c
		while((nSelRomFiler = getFilePath(RomPath, setting.szLastPath, (LPEXTENTIONS)&stExtRom, nSelRomFiler)) < 0)
			;
		// init and rom load
		if (NES_Init(RomPath)) {
			break;
		}
	}

	pgFillvram(0);
	pgScreenFlipV();
	pgFillvram(0);
	pgScreenFlip();
	PSPEMU_Thaw();

	for(;;) {
		PSPEMU_DoFrame();

		if(setting.key_config[6] && (paddata.buttons&setting.key_config[6])==setting.key_config[6]){
			PSPEMU_Freeze();
			rin_menu();
			if(g_NESConfig.sound.enabled) wavout_enable=1;
			PSPEMU_Thaw();
		}

		if(bSleep){
			PSPEMU_Freeze();
			pgWaitVn(180);
			bSleep=0;
			PSPEMU_Thaw();
		}
	}

	return 0;
}
