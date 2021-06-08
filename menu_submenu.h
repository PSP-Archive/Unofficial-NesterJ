#ifndef MENU_SUBMENU_H
#define MENU_SUBMENU_H

extern int StateSlot;

void submenu_diskchange(void);

void submenu_preferencesconfig(void);
void submenu_graphicsconfig(void);
void submenu_soundconfig(void);
void submenu_colorconfig(void);
void submenu_keyconfig(void);

// Msgbox types
#define MB_OK		0x00000000L

// Msgbox ret val(åªç›IDOKÇµÇ©ãAÇ¡ÇƒÇ±Ç»Ç¢)
#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
int MessageBox(unsigned char*pszMsg, uint32 ulTypes);

typedef enum {
	STATE_LOAD,	// load
	STATE_SAVE	// save
}statemethod;

#define STATE_SLOT_MAX 10	// 0-9

// get timestamp string form state file
bool GetStateTime(const char *pszFile, char *pszOut);

int submenu_stateslot(statemethod method);

#endif
