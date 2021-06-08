#include "syscall.h"

int _memcmp(const void *buf1, const void *buf2,int n)
{
	int ret;
	int i;
	
	for(i=0; i<n; i++){
		ret = ((unsigned char*)buf1)[i] - ((unsigned char*)buf2)[i];
		if(ret!=0)
			return ret;
	}
	return 0;
}

void* _memcpy(void *buf1, const void *buf2, int n)
{
	while(n-->0)
		((unsigned char*)buf1)[n] = ((unsigned char*)buf2)[n];
	return buf1;
}

void* _memset(void *buf, int ch, int n)
{
	unsigned char *p = buf;
	
	while(n>0)
		p[--n] = ch;
	
	return buf;
}

int _strlen(const char *s)
{
	int ret;
	
	for(ret=0; s[ret]; ret++)
		;
	
	return ret;
}

char* _strcpy(char *dest, const char *src)
{
	int i;
	
	for(i=0; src[i]; i++)
		dest[i] = src[i];
	dest[i] = 0;
	
	return dest;
}

char* _strncpy(char *dest, const char *src, int size)
{
	int i;

	for(i=0; src[i] && size; i++, size--)
		dest[i] = src[i];
	while (size--) {
		dest[i] = 0;
		i++;
	}

	return dest;
}

char* _strrchr(const char *src, int c)
{
	int len;
	
	len=_strlen(src);
	while(len>0){
		len--;
		if(*(src+len) == c)
			return (char*)(src+len);
	}
	
	return NULL;
}

char* _strcat(char *dest, const char *src)
{
	int i;
	int len;
	
	len=_strlen(dest);
	for(i=0; src[i]; i++)
		dest[len+i] = src[i];
	dest[len+i] = 0;
	
	return dest;
}

int _strcmp(const char *str1, const char *str2)
{
	char c1, c2;
	for(;;){
		c1 = *str1;
		c2 = *str2;
		
		if(c1!=c2)
			return 1;
		else if(c1==0)
			return 0;
		
		str1++; str2++;
	}
}

int _strncmp(const char *str1, const char *str2, int size)
{
	char c1, c2;
	while(size--){
		c1 = *str1;
		c2 = *str2;

		if(c1!=c2)
			return 1;
		else if(c1==0)
			return 0;

		str1++; str2++;
	}
	return 0;
}

int _stricmp(const char *str1, const char *str2)
{
	char c1, c2;
	for(;;){
		c1 = *str1;
		if(c1>=0x61 && c1<=0x7A) c1-=0x20;
		c2 = *str2;
		if(c2>=0x61 && c2<=0x7A) c2-=0x20;
		
		if(c1!=c2)
			return 1;
		else if(c1==0)
			return 0;
		
		str1++; str2++;
	}
}

void _strrev(char *s){
	char tmp;
	int i;
	int len = _strlen(s);
	
	for(i=0; i<len/2; i++){
		tmp = s[i];
		s[i] = s[len-1-i];
		s[len-1-i] = tmp;
	}
}

void _itoa(int val, char *s) {
	char *t;
	int mod;

	if(val < 0) {
		*s++ = '-';
		val = -val;
	}
	t = s;

	while(val) {
		mod = val % 10;
		*t++ = (char)mod + '0';
		val /= 10;
	}

	if(s == t)
		*t++ = '0';

	*t = '\0';

	_strrev(s);
}

void _itoa32(unsigned long val, char *s) {
	char *t;
	unsigned long mod;

	if(val < 0) {
		*s++ = '-';
		val = -val;
	}
	t = s;

	while(val) {
		mod = val % 10;
		*t++ = (char)mod + '0';
		val /= 10;
	}

	if(s == t)
		*t++ = '0';

	*t = '\0';

	_strrev(s);
}

int _atoi(const unsigned char *pszStr)
{
	unsigned long ulVal = 0;
	int cbI = 0;

	while (pszStr[cbI]) {
		if (pszStr[cbI] >= '0' && pszStr[cbI] <= '9') {
			ulVal *= 10;
			ulVal += pszStr[cbI] - '0';
		}
		else {
			break;
		}
		cbI++;
	}
	return ulVal;
}

unsigned long _atoh(const unsigned char *pszStr)
{
	unsigned long ulVal = 0;
	int cbI = 0;

	while (pszStr[cbI]) {
		if (pszStr[cbI] >= '0' && pszStr[cbI] <= '9') {
			ulVal <<= 4;
			ulVal += pszStr[cbI] - '0';
		}
		else if (pszStr[cbI] >= 'a' && pszStr[cbI] <= 'f') {
			ulVal <<= 4;
			ulVal += pszStr[cbI] - 'a' + 10;
		}
		else if (pszStr[cbI] >= 'A' && pszStr[cbI] <= 'F') {
			ulVal <<= 4;
			ulVal += pszStr[cbI] - 'a' + 10;
		}
		else {
			break;
		}
		cbI++;
	}
	return ulVal;
}
