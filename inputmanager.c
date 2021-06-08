/*
	Input manager by ruka
*/
#include "main.h"
#include "nes/nes_pad.h"
#include "emu_main.h"
#include "nes/nes_config.h"
#include "inputmanager.h"
#include "screenmanager.h"
#include "menu_submenu.h"


int now_sensor_x,now_sensor_y;

unsigned char g_PadBit[6];


#define UPPER_THRESHOLD  0xcf
#define LOWER_THRESHOLD  0x2f

KeyState stKeyState = {FALSE, FALSE, EMU_SPEED_NORMAL};

void InputUpdateRapid(void)
{
	static uint32 ulCounter = 0;
	boolean bPush = FALSE;
	ulCounter++;
	switch(setting.rapidmode) {
	case RAPID_30COUNT:  // 30count/sec
		if (ulCounter%2) bPush = TRUE;
	break;
	case RAPID_20COUNT:  // 20count/sec
		if (ulCounter%3>=1) bPush = TRUE;
	break;
	case RAPID_15COUNT:  // 15count/sec
		if (ulCounter%4>=2) bPush = TRUE;
	break;
	case RAPID_10COUNT:  // 10count/sec
		if (ulCounter%6>=3) bPush = TRUE;
	break;
	}
	if (bPush) {
		if (stKeyState.bRapidA) g_PadBit[0] |= NES_A;
		if (stKeyState.bRapidB) g_PadBit[0] |= NES_B;
	}
	else {
		if (stKeyState.bRapidA) g_PadBit[0] &= ~NES_A;
		if (stKeyState.bRapidB) g_PadBit[0] &= ~NES_B;
	}
}


void InputUpdatePad(void)
{
	static boolean bPushFlag = FALSE;
	static boolean bTogglePushFlag = FALSE;
	char msg[256], tmp[256];
	static int ret;


	// by kwn
	g_PadBit[0]=0;
	stKeyState.bRapidA = FALSE;
	stKeyState.bRapidB = FALSE;
	sceCtrlRead(&paddata, 1);
	// A、B AB連射
	if((paddata.buttons&setting.key_config[2])==setting.key_config[2] && setting.key_config[2])
		stKeyState.bRapidA = TRUE;
	else if((paddata.buttons&setting.key_config[0])==setting.key_config[0] && setting.key_config[0])
		g_PadBit[0] |= NES_A;
	if((paddata.buttons&setting.key_config[3])==setting.key_config[3] && setting.key_config[3])
		stKeyState.bRapidB = TRUE;
	else if((paddata.buttons&setting.key_config[1])==setting.key_config[1] && setting.key_config[1])
		g_PadBit[0] |= NES_B;
	if((paddata.buttons&setting.key_config[4])==setting.key_config[4] && setting.key_config[4])
		g_PadBit[0] |= NES_SELECT;
	if((paddata.buttons&setting.key_config[5])==setting.key_config[5] && setting.key_config[5])
		g_PadBit[0] |= NES_START;

	g_PadBit[4]=0;
	// PLAYER2 MIC
	if((paddata.buttons&setting.key_config[7])==setting.key_config[7] && setting.key_config[7])
		g_PadBit[4] |= 0x04;

	// cross
	if(paddata.buttons & CTRL_DOWN)  g_PadBit[0] |= NES_DOWN;
	if(paddata.buttons & CTRL_UP)    g_PadBit[0] |= NES_UP;
	if(paddata.buttons & CTRL_LEFT)  g_PadBit[0] |= NES_LEFT;
	if(paddata.buttons & CTRL_RIGHT) g_PadBit[0] |= NES_RIGHT;

	// Analog pad state
	if(setting.analog2dpad){
		if (paddata.analog[CTRL_ANALOG_Y] > UPPER_THRESHOLD) g_PadBit[0] |= NES_DOWN; // DOWN
		if (paddata.analog[CTRL_ANALOG_Y] < LOWER_THRESHOLD) g_PadBit[0] |= NES_UP; // UP
		if (paddata.analog[CTRL_ANALOG_X] < LOWER_THRESHOLD) g_PadBit[0] |= NES_LEFT; // LEFT
		if (paddata.analog[CTRL_ANALOG_X] > UPPER_THRESHOLD) g_PadBit[0] |= NES_RIGHT; // RIGHT
	}

	// Toggle モードじゃないときの処理
	if(!setting.bToggle) {
		// Speed mode
		if(setting.key_config[8] && (paddata.buttons&setting.key_config[8])==setting.key_config[8]){
			stKeyState.nSpeedMode = EMU_SPEED_MODE1;
		}else if(setting.key_config[9] && (paddata.buttons&setting.key_config[9])==setting.key_config[9]){
			stKeyState.nSpeedMode = EMU_SPEED_MODE2;
		}
		else {
			stKeyState.nSpeedMode = EMU_SPEED_NORMAL;
		}
	}
	else {

		// Speed mode 1(Toggle)
		if(setting.bToggle && setting.key_config[8] && (paddata.buttons&setting.key_config[8])==setting.key_config[8]){
			if (!bTogglePushFlag) {
				if( stKeyState.nSpeedMode == EMU_SPEED_MODE1 ){
					char szTemp[128];
					stKeyState.nSpeedMode = EMU_SPEED_NORMAL;
					_strcpy(szTemp, "SPEED MODE: NORMAL");
					Scr_SetMessage(szTemp);
				}else{
					char msg[256], szTemp[128];
					stKeyState.nSpeedMode = EMU_SPEED_MODE1;
					_strcpy(szTemp, "SPEED MODE: MODE 1 (");
					_itoa(setting.speedmode1, msg);
					_strcat(szTemp, msg);
					_strcat(szTemp, "fps)");
					Scr_SetMessage(szTemp);
				}
				bTogglePushFlag = TRUE;
			}
			// キーをクリアすると押したとき動きが止まるのでクリアしない
			//g_PadBit[0]=0;
		}
		// Speed mode 2(Toggle)
		else if(setting.bToggle && setting.key_config[9] && (paddata.buttons&setting.key_config[9])==setting.key_config[9]){
			if (!bTogglePushFlag) {
				if( stKeyState.nSpeedMode == EMU_SPEED_MODE2 ){
					char szTemp[128];
					stKeyState.nSpeedMode = EMU_SPEED_NORMAL;
					_strcpy(szTemp, "SPEED MODE: NORMAL");
					Scr_SetMessage(szTemp);
				}else{
					char msg[256], szTemp[128];
					stKeyState.nSpeedMode = EMU_SPEED_MODE2;
					_strcpy(szTemp, "SPEED MODE: MODE 2 (");
					_itoa(setting.speedmode2, msg);
					_strcat(szTemp, msg);
					_strcat(szTemp, "fps)");
					Scr_SetMessage(szTemp);
				}
				bTogglePushFlag = TRUE;
			}
			// キーをクリアすると押したとき動きが止まるのでクリアしない
			//g_PadBit[0]=0;
		}
		else {
			bTogglePushFlag = FALSE;
		}
	}

	if (1) {
		// Sound on/off
		if(setting.key_config[10] && (paddata.buttons&setting.key_config[10])==setting.key_config[10]){
			if (!bPushFlag) {
				g_NESConfig.sound.enabled = g_NESConfig.sound.enabled ? 0: 1;
				Scr_SetMessage(g_NESConfig.sound.enabled ? "SOUND ENABLE": "SOUND DISABLE");
				PSPEMU_ApplySoundConfig();
				wavout_enable=g_NESConfig.sound.enabled;
				bPushFlag = TRUE;
			}
			g_PadBit[0]=0;
		}
		// Screen size change
		else if(setting.key_config[11] && (paddata.buttons&setting.key_config[11])==setting.key_config[11]){
			if (!bPushFlag) {
				setting.screenmode = (setting.screenmode + 1) % SCREEN_COUNT;
				char szTemp[128];
				_strcpy(szTemp, "SCREEN SIZE:");
				_strcat(szTemp, aszScreenName[setting.screenmode]);
				// 画面の掃除
				pgFillvram(0); pgScreenFlip();
				pgFillvram(0); pgScreenFlip();
				Scr_SetMessage(szTemp);
				bPushFlag = TRUE;
			}
			g_PadBit[0]=0;
		}
		// Quick save
		else if(setting.key_config[12] && (paddata.buttons&setting.key_config[12])==setting.key_config[12]){
			if (!bPushFlag) {
				PSPEMU_Freeze();
				if(PSPEMU_SaveState(StateSlot)) {
					char szTemp[128];
					_strcpy(szTemp, "State save at x");
					szTemp[sizeof("State save at x") - 2] = StateSlot+'0';
					Scr_SetMessage(szTemp);
				}
				else
					Scr_SetMessage("State save Failed");
				bPushFlag = TRUE;
				PSPEMU_Thaw();
			}
			g_PadBit[0]=0;
		}
		// Quick load
		else if(setting.key_config[13] && (paddata.buttons&setting.key_config[13])==setting.key_config[13]){
			if (!bPushFlag) {
				if(PSPEMU_LoadState(StateSlot)) {
					char szTemp[128];
					_strcpy(szTemp, "State load at x");
					szTemp[sizeof("State load at x") - 2] = StateSlot+'0';
					Scr_SetMessage(szTemp);
				}
				else
					Scr_SetMessage("State load Failed");
				bPushFlag = TRUE;
			}
			g_PadBit[0]=0;
		}
		// Quick Slot change
		else if(setting.key_config[14] && (paddata.buttons&setting.key_config[14])==setting.key_config[14]){
			if (!bPushFlag) {
				char szTemp[128];
				StateSlot = (StateSlot + 1) % STATE_SLOT_MAX;
				_strcpy(szTemp,"QUICK SLOT : x");
				szTemp[sizeof("QUICK SLOT : x") -2] = StateSlot+'0';
				Scr_SetMessage(szTemp);
				bPushFlag = TRUE;
			}
			g_PadBit[0]=0;
		}
		//Battery Display by Smiths
		else if(setting.key_config[15] && (paddata.buttons&setting.key_config[15])==setting.key_config[15]){
			if (!bPushFlag) {
				char szTemp[128];
				if(scePowerIsBatteryExist()){
					_strcpy(szTemp,"BATTERY LIFE : ");
					ret = scePowerGetBatteryLifePercent();
					_itoa(ret,tmp);
					_strcat(szTemp,tmp);
					_strcat(szTemp,"%");

					if(!scePowerIsPowerOnline()){
						if((ret=scePowerGetBatteryLifeTime()) >= 0){
							_strcat(szTemp,"(");
							_itoa(ret/60,tmp);
							_strcat(szTemp,tmp);
							_strcat(szTemp,":");
							_itoa(ret%60+100,tmp);
							_strcat(szTemp,tmp+1);
							_strcat(szTemp,")");
						}
					}
				}
				Scr_SetMessage(szTemp);
				bPushFlag = TRUE;
			}
			// キーをクリアすると押したとき動きが止まるのでクリアしない
			// メッセージしか表示しないしねぇ
			// g_PadBit[0]=0;
		}
		else {
			bPushFlag = FALSE;
		}
	}

	now_sensor_x=2047-paddata.analog[CTRL_ANALOG_X]+127;
	now_sensor_y=2047-paddata.analog[CTRL_ANALOG_Y]+127;
}
