#ifndef FILER_H
#define FILER_H

extern char FilerMsg[];

// 拡張子管理用
typedef struct {
	char *szExt;
	int nExtId;
} EXTENTIONS, *LPEXTENTIONS;

int getExtId(const char *szFilePath, LPEXTENTIONS pExt);

int getFilePath(char *out, char *pszStartPath, LPEXTENTIONS pExt, int nOldSel);

// 有効な拡張子
enum {
	EXT_NES,
	EXT_ZIP,
	EXT_ALL,
	EXT_UNKNOWN
};



#endif
