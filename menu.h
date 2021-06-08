#ifndef MENU_H
#define MENU_H

extern int StateSlot;
extern boolean g_bEnableBitmap;
extern unsigned short g_bgBitmap[480*272];

void setkeydefault(void);
void draw_load_rom_progress(unsigned long ulTotal, unsigned long ulCurrent);

int save_state(void);
int load_state(void);
long load_rom(const char *pszFile);
void menu_frame(const char *msg0, const char *msg1);
void rin_menu(void);

#endif
