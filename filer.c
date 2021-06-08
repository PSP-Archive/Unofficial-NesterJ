#include "main.h"

extern u32 new_pad;

struct dirent files[MAX_ENTRY];
struct dirent *sortfiles[MAX_ENTRY];
int nfiles;

////////////////////////////////////////////////////////////////////////
// クイックソート

void SJISCopy(struct dirent *a, unsigned char *file)
{
	unsigned char ca;
	int i;
	int len = _strlen(a->name);

	for(i=0;i<=len;i++){
		ca = a->name[i];
		if (((0x81 <= ca)&&(ca <= 0x9f))
		|| ((0xe0 <= ca)&&(ca <= 0xef))){
			file[i++] = ca;
			file[i] = a->name[i];
		}
		else{
			if(ca>='a' && ca<='z') ca-=0x20;
			file[i] = ca;
		}
	}

}
int cmpFile(struct dirent *a, struct dirent *b)
{
    unsigned char file1[0x108];
    unsigned char file2[0x108];
	unsigned char ca, cb;
	int i, n, ret;

	if(a->type==b->type){
		SJISCopy(a, file1);
		SJISCopy(b, file2);
		n=_strlen(file1);
		for(i=0; i<=n; i++){
			ca=file1[i]; cb=file2[i];
			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}

	if(a->type & TYPE_DIR)	return -1;
	else					return 1;
}

void sort(struct dirent **a, int left, int right) {
	struct dirent *tmp, *pivot;
	int i, p;

	if (left < right) {
		pivot = a[left];
		p = left;
		for (i=left+1; i<=right; i++) {
			if (cmpFile(a[i],pivot)<0){
				p=p+1;
				tmp=a[p];
				a[p]=a[i];
				a[i]=tmp;
			}
		}
		a[left] = a[p];
		a[p] = pivot;
		sort(a, left, p-1);
		sort(a, p+1, right);
	}
}

int getExtId(const char *szFilePath, LPEXTENTIONS pExt) {
	char *pszExt;
	int i;
	if((pszExt = _strrchr(szFilePath, '.'))) {
		pszExt++;
		for (i = 0; pExt[i].szExt; i++) {
			if (!_stricmp(pExt[i].szExt,pszExt)) {
				break;
			}
		}
		return pExt[i].nExtId;
	}
	return EXT_UNKNOWN;
}



void getDir(const char *path, LPEXTENTIONS pExt) {
	int fd, b=0;
	char *p;

	nfiles = 0;

	if(_strcmp(path,"ms0:/")){
		_strcpy(files[nfiles].name,"..");
		files[nfiles].type = TYPE_DIR;
		sortfiles[nfiles] = files + nfiles;
		nfiles++;
		b=1;
	}

	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		_memset(&files[nfiles], 0x00, sizeof(struct dirent));
		if(sceIoDread(fd, &files[nfiles])<=0) break;
		if(files[nfiles].name[0] == '.') continue;
		if(files[nfiles].type == TYPE_DIR){
			_strcat(files[nfiles].name, "/");
			sortfiles[nfiles] = files + nfiles;
			nfiles++;
			continue;
		}
		sortfiles[nfiles] = files + nfiles;
		if (pExt) {
			if(getExtId(files[nfiles].name, pExt) != EXT_UNKNOWN) nfiles++;
		}
		else {
			nfiles++;
		}
	}
	sceIoDclose(fd);
	if(b)
		sort(sortfiles+1, 0, nfiles-2);
	else
		sort(sortfiles, 0, nfiles-1);
}

char FilerMsg[256];
int getFilePath(char *out, char *pszStartPath, LPEXTENTIONS pExt, int nOldSel)
{
	unsigned long color;
	int top, rows=21, x, y, h, i, len, bMsg=0, up=0;
	char path[MAXPATH], oldDir[MAXNAME], *p;
	int sel = nOldSel;

	top = sel-3;

	_strcpy(path, pszStartPath);
	if(FilerMsg[0])
		bMsg=1;

	getDir(path, pExt);
	for(;;){
		readpad();
		if(new_pad)
			bMsg=0;
		if(new_pad & CTRL_CIRCLE){
			if(sortfiles[sel]->type == TYPE_DIR){
				if(!_strcmp(sortfiles[sel]->name,"..")){
					up=1;
				}else{
					_strcat(path,sortfiles[sel]->name);
					getDir(path, pExt);
					sel=0;
				}
			}else{
				_strcpy(out, path);
				_strcat(out, sortfiles[sel]->name);
				_strcpy(pszStartPath,path);
				return sel;
			}
		}else if(new_pad & CTRL_CROSS){
			return -1;
		}else if(new_pad & CTRL_TRIANGLE){
			up=1;
		}else if(new_pad & CTRL_UP){
			sel--;
		}else if(new_pad & CTRL_DOWN){
			sel++;
		}else if(new_pad & CTRL_LEFT){
			sel-=10;
		}else if(new_pad & CTRL_RIGHT){
			sel+=10;
		}

		if(up){
			if(_strcmp(path,"ms0:/")){
				p=_strrchr(path,'/');
				*p=0;
				p=_strrchr(path,'/');
				p++;
				_strcpy(oldDir,p);
				_strcat(oldDir,"/");
				*p=0;
				getDir(path, pExt);
				sel=0;
				for(i=0; i<nfiles; i++) {
					if(!_strcmp(oldDir, sortfiles[i]->name)) {
						sel=i;
						top=sel-3;
						break;
					}
				}
			}
			up=0;
		}

		if(top > nfiles-rows)	top=nfiles-rows;
		if(top < 0)				top=0;
		if(sel >= nfiles)		sel=nfiles-1;
		if(sel < 0)				sel=0;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;

		if(bMsg)
			menu_frame(FilerMsg,"○：OK　×：CANCEL　△：UP");
		else
			menu_frame(path,"○：OK　×：CANCEL　△：UP");

		// スクロールバー
		if(nfiles > rows){
			h = 219;
			pgDrawFrame(445,25,446,248,setting.color[1]);
			pgFillBox(448, h*top/nfiles + 27,
				460, h*(top+rows)/nfiles + 27,setting.color[1]);
		}

		x=28; y=32;
		for(i=0; i<rows; i++){
			if(top+i >= nfiles) break;
			if(top+i == sel) color = setting.color[2];
			else			 color = setting.color[3];
			mh_print(x, y, sortfiles[top+i]->name, color);
			y+=10;
		}

		pgScreenFlipV();
	}
}
