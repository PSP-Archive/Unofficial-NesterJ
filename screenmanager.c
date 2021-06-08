//
// NES emu と PSPの架け橋(スクリーン編)
//
#include "screenmanager.h"
#include "nes/nes.h"
#include "debug/debug.h"
#include "main.h"
#include "pg.h"

uint16 __attribute__((aligned(16))) g_Pal[256];	// palette index (using 0-63)
uint8  __attribute__((aligned(16))) g_ScrBuf[NES_SCREEN_HEIGHTMAX][NES_BACKBUF_WIDTH]; // Screen palette index buffer


// パレット通知 pal[NES_MAX_COLORS][3]
void Scr_AssertPalette(uint8 pal[][3])
{
	int cbI;
	DEBUG("Scr_AssertPalette");
	_memset(g_Pal, 0x00, sizeof(g_Pal));
	for (cbI = 0; cbI < NES_NUM_COLORS; cbI++) {
		g_Pal[NES_COLOR_BASE + cbI] = RGB(pal[cbI][0], pal[cbI][1], pal[cbI][2]);
	}
}

// 画面モード PAL or NTSC
void Scr_SetScreenMode(const unsigned char ScreenMode)
{
}

// 指定した色で塗りつぶし
void Scr_ClearScreen(const uint8 PalNum)
{
	int x,y;
	DEBUG("Scr_ClearScreen");
	_memset(g_ScrBuf, PalNum, sizeof(g_ScrBuf));
}

char g_szRenderMsg[80];
int g_nRenderMsgLen;
int g_nRenderMsgCount = 0;

// 画面にメッセージ表示用
void Scr_SetMessage(const char *pszMsg)
{
	_strncpy(g_szRenderMsg, pszMsg, sizeof(g_szRenderMsg) -1);
	g_szRenderMsg[79] = '\0';
	g_nRenderMsgLen=_strlen(g_szRenderMsg);
	g_nRenderMsgCount = 60;
}

// internal blt function - prototypes
void BitBlt_NormalScreen(void);
void BitBlt_FullScreen(void);
void BitBlt_X15(void);
void BitBlt_X15_CROPPED(void);

void BitBlt_GPU(int width,int height);

// 画面へ転送
// 新規モード追加時は上にプロトタイプ宣言し、Scr_BltScreenのswitch内に追加、
// 下のBltScreen internal functions のところに処理を書く
// screenmanager.h のenumとaszScreenNameに追加すればメニュー対応完了
void Scr_BltScreen(void)
{
	if(g_nRenderMsgCount==1 || g_nRenderMsgCount==2 || g_nRenderMsgCount==59 || g_nRenderMsgCount==60) pgFillBox(0,263,479,271,0);

	// パレットとバッファから画面へ描画処理をする
#if 1 // GPU
	switch(setting.screenmode) {
	case SCREEN_GPU_FULL:       BitBlt_GPU(480,272);    break;
    case SCREEN_GPU_X15:        BitBlt_GPU(363,272);    break;
    case SCREEN_GPU_NORMAL:     BitBlt_GPU(256,224);    break;
	case SCREEN_FULL:           BitBlt_FullScreen();    break;
	case SCREEN_X15:            BitBlt_X15();           break;
	case SCREEN_X15_CROPPED:    BitBlt_X15_CROPPED();   break;
	default:                    BitBlt_NormalScreen();  break; // SCREEN_NORMAL
	}
#else // GPU
	switch(setting.screenmode) {
	case SCREEN_FULL:           BitBlt_FullScreen();    break;
	case SCREEN_X15:            BitBlt_X15();           break;
	case SCREEN_X15_CROPPED:    BitBlt_X15_CROPPED();   break;
	default:                    BitBlt_NormalScreen();  break; // SCREEN_NORMAL
	}
#endif//GPU
    
	// draw message
	if(g_nRenderMsgCount>=3){
		pgFillBox(0,263,g_nRenderMsgLen*8,271,0);
		pgPrint(0,33,0xffff,g_szRenderMsg);
	}
	if(g_nRenderMsgCount>0) g_nRenderMsgCount--;
	// draw fps
	if (setting.showfps) {
		extern uint32 g_ulFPS;	// emu_main.c
		char szFPS[32];
		_itoa(g_ulFPS, szFPS);
		_strcat(szFPS, "FPS");
		pgFillBox(0,0,_strlen(szFPS)*8,8,0);
		pgPrint(0,0,0xffff,szFPS);
	}
}

// 描画ロック
boolean Scr_Lock(pixmap *p)
{
	p->height = NES_SCREEN_HEIGHTMAX;
	p->width  = NES_BACKBUF_WIDTH;
	p->pitch  = NES_BACKBUF_WIDTH;
	p->bitdepth = 0;
	p->Gbitmask = 0;
	p->data = &g_ScrBuf[0][0];
	return TRUE;
}

// 描画アンロック
void Scr_Unlock(void)
{
}







/////////////////////////////////////////////////
// BltScreen internal functions

// Parallel blend 1:1で合成
static inline unsigned long PBlend(unsigned long c0, unsigned long c1)
{
	return (c0 & c1) + (((c0 ^ c1) & 0x7bde7bde) >> 1);
}

// Parallel blend 3:1で合成
static inline unsigned long PBlend_3to1(unsigned long d0, unsigned long d1){
	return PBlend( PBlend(d0,d1), d0 );
//	return d0; // same old ver
//	return PBlend( PBlend(d0,0), d0 ); // like scanline
}

#ifdef __USE_MIPS32R2__

static inline void _asm_Blt_Normal(unsigned long *pVram, unsigned long *src)
{
	unsigned long x, y;
	unsigned long t0, t1, t2, t3;
	const unsigned long mask = 0x3f3f3f3f;
	y = 224;

	asm volatile (
		"	.set	push"				"\n"
		"	.set	noreorder"			"\n"

		"	.set	mips32r2"			"\n"

		//{
		"1:	li		%4, 64"				"\n"	// x=NES_SCREEN_WIDTH/4
		"	addiu	%5, -1"				"\n"	// y--
			//{
		"2:		lw		%3, 0(%7)"			"\n"	// t3=*src
		"		addiu	%4, -1"				"\n"	// x--
		"		addiu	%7, 4"				"\n"	// src++

		"		and		%3, %9"				"\n"	// t3 &= 0x3f3f3f3f
		"		sll		%3, 1"				"\n"	// t3 *= 2

		"		ext		%0, %3, 0, 7"		"\n"	// t0 =  t3&0x7f
		"		addu	%0, %8"				"\n"	// t0 += g_Pal
		"		lhu		%0, 0(%0)"			"\n"	// t0 = |0|A|

		"		ext		%2, %3, 8, 7"		"\n"	// t2 =  (t3>>8)&0x7f
		"		addu	%2, %8"				"\n"	// t2 += g_Pal
		"		lhu		%2, 0(%2)"			"\n"	// t2 = |0|B|

		"		ext		%1, %3, 16, 7"		"\n"	// t1 =  (t3>>16)&0x7f
		"		addu	%1, %8"				"\n"	// t1 += g_Pal
		"		lhu		%1, 0(%1)"			"\n"	// t1 = |0|C|

		"		srl 	%3, 24"				"\n"	// t3 =  (t3>>24)&0x7f
		"		addu 	%3, %8"				"\n"	// t3 += g_Pal
		"		lhu 	%3, 0(%3)"			"\n"	// t3 = |0|D|

		"		ins 	%0, %2, 16, 16"		"\n"	// t0 = |B|A|
		"		sw		%0, 0(%6) "			"\n"	// *pVram = t0

		"		ins		%1, %3, 16, 16"		"\n"	// t1 = |D|C|
		"		sw		%1, 4(%6) "			"\n"	// *(pVram+1) = t1

		"		bnez	%4, 2b "			"\n"
		"		addiu	%6, 8 "				"\n"	// [delay slot] pVram += 2
			//}

		"	addiu	%7, 16"				"\n"	// src+=SIDE_MARGIN*2/sizeof(long)

		"	bnez	%5, 1b "			"\n"
		"	addiu	%6, 512 "			"\n"	// [delay slot] pVram += (LINESIZE-NES_SCREEN_WIDTH)*2/sizeof(long)
		//}

		"	.set	pop"				"\n"

			:	"=&r" (t0),		// %0
				"=&r" (t1),		// %1
				"=&r" (t2),		// %2
				"=&r" (t3),		// %3
				"=&r" (x)		// %4
			:	"r" (y),		// %5
				"r" (pVram),	// %6
				"r" (src),		// %7
				"r" (g_Pal),	// %8
				"r" (mask)		// %9
			:	"memory"
	);
}
#endif //__USE_MIPS32R2__

void BitBlt_NormalScreen(void)
{
#ifdef __USE_MIPS32R2__

	unsigned long *pVram, *src;
	
	pVram = (unsigned long *)pgGetVramAddr(112, 30);
	src = (unsigned long*)&g_ScrBuf[8][8];
	
	_asm_Blt_Normal( pVram, src );
	
#else

	int x,y;
	unsigned short *pVram;
	pVram = (unsigned short *)pgGetVramAddr(0, 0) + 112 + 30 * LINESIZE;
	for (y = 0; y < 224; y++) {
		for (x = 0; x < 256; x++) {
			pVram[x] = g_Pal[g_ScrBuf[y + 8][x + 8]];
		}
		pVram += LINESIZE;
	}
	
#endif //__USE_MIPS32R2__
}

#if 1


/******************************************************************************/

static unsigned int GeInit[] = {
	0x01000000, 0x02000000,
	0x10000000, 0x12000000, 0x13000000, 0x15000000, 0x16000000, 0x17000000,
	0x18000000, 0x19000000, 0x1A000000, 0x1B000000, 0x1C000000, 0x1D000000,
	0x1E000000, 0x1F000000,
	0x20000000, 0x21000000, 0x22000000, 0x23000000, 0x24000000, 0x25000000,
	0x26000000, 0x27000000, 0x28000000, 0x2A000000, 0x2B000000, 0x2C000000,
	0x2D000000, 0x2E000000, 0x2F000000,
	0x30000000, 0x31000000, 0x32000000, 0x33000000, 0x36000000, 0x37000000,
	0x38000000, 0x3A000000, 0x3B000000, 0x3C000000, 0x3D000000, 0x3E000000,
	0x3F000000,
	0x40000000, 0x41000000, 0x42000000, 0x43000000, 0x44000000, 0x45000000,
	0x46000000, 0x47000000, 0x48000000, 0x49000000, 0x4A000000, 0x4B000000,
	0x4C000000, 0x4D000000,
	0x50000000, 0x51000000, 0x53000000, 0x54000000, 0x55000000, 0x56000000,
	0x57000000, 0x58000000, 0x5B000000, 0x5C000000, 0x5D000000, 0x5E000000,
	0x5F000000,
	0x60000000, 0x61000000, 0x62000000, 0x63000000, 0x64000000, 0x65000000,
	0x66000000, 0x67000000, 0x68000000, 0x69000000, 0x6A000000, 0x6B000000,
	0x6C000000, 0x6D000000, 0x6E000000, 0x6F000000,
	0x70000000, 0x71000000, 0x72000000, 0x73000000, 0x74000000, 0x75000000,
	0x76000000, 0x77000000, 0x78000000, 0x79000000, 0x7A000000, 0x7B000000,
	0x7C000000, 0x7D000000, 0x7E000000, 0x7F000000,
	0x80000000, 0x81000000, 0x82000000, 0x83000000, 0x84000000, 0x85000000,
	0x86000000, 0x87000000, 0x88000000, 0x89000000, 0x8A000000, 0x8B000000,
	0x8C000000, 0x8D000000, 0x8E000000, 0x8F000000,
	0x90000000, 0x91000000, 0x92000000, 0x93000000, 0x94000000, 0x95000000,
	0x96000000, 0x97000000, 0x98000000, 0x99000000, 0x9A000000, 0x9B000000,
	0x9C000000, 0x9D000000, 0x9E000000, 0x9F000000,
	0xA0000000, 0xA1000000, 0xA2000000, 0xA3000000, 0xA4000000, 0xA5000000,
	0xA6000000, 0xA7000000, 0xA8000000, 0xA9000000, 0xAA000000, 0xAB000000,
	0xAC000000, 0xAD000000, 0xAE000000, 0xAF000000,
	0xB0000000, 0xB1000000, 0xB2000000, 0xB3000000, 0xB4000000, 0xB5000000,
	0xB8000000, 0xB9000000, 0xBA000000, 0xBB000000, 0xBC000000, 0xBD000000,
	0xBE000000, 0xBF000000,
	0xC0000000, 0xC1000000, 0xC2000000, 0xC3000000, 0xC4000000, 0xC5000000,
	0xC6000000, 0xC7000000, 0xC8000000, 0xC9000000, 0xCA000000, 0xCB000000,
	0xCC000000, 0xCD000000, 0xCE000000, 0xCF000000,
	0xD0000000, 0xD2000000, 0xD3000000, 0xD4000000, 0xD5000000, 0xD6000000,
	0xD7000000, 0xD8000000, 0xD9000000, 0xDA000000, 0xDB000000, 0xDC000000,
	0xDD000000, 0xDE000000, 0xDF000000,
	0xE0000000, 0xE1000000, 0xE2000000, 0xE3000000, 0xE4000000, 0xE5000000,
	0xE6000000, 0xE7000000, 0xE8000000, 0xE9000000, 0xEB000000, 0xEC000000,
	0xEE000000,
	0xF0000000, 0xF1000000, 0xF2000000, 0xF3000000, 0xF4000000, 0xF5000000,
	0xF6000000,	0xF7000000, 0xF8000000, 0xF9000000,
	0x0F000000, 0x0C000000,
    0 // stall address
  };

void pgGeInit()
{
	int qid;
	sceKernelDcacheWritebackAll();
    qid = sceGeListEnQueue(&GeInit[0], &GeInit[sizeof(GeInit)/sizeof(GeInit[0])], -1, 0);
	sceGeListSync(qid, 0);

    {
        static unsigned int GEcmd[64];
        // Draw Area
        GEcmd[ 0] = 0x15000000UL | (0 << 10) | 0;
        GEcmd[ 1] = 0x16000000UL | (271 << 10) | 479;
        // Tex Enable
        GEcmd[ 2] = 0x1E000000UL | 1;
        // Viewport
        GEcmd[ 3] = 0x42000000UL | (((int)((float)(480)) >> 8) & 0x00FFFFFF);
        GEcmd[ 4] = 0x43000000UL | (((int)((float)(-272)) >> 8) & 0x00FFFFFF);
        GEcmd[ 5] = 0x44000000UL | (((int)((float)(50000)) >> 8) & 0x00FFFFFF);
        GEcmd[ 6] = 0x45000000UL | (((int)((float)(2048)) >> 8) & 0x00FFFFFF);
        GEcmd[ 7] = 0x46000000UL | (((int)((float)(2048)) >> 8) & 0x00FFFFFF);
        GEcmd[ 8] = 0x47000000UL | (((int)((float)(60000)) >> 8) & 0x00FFFFFF);
        GEcmd[ 9] = 0x4C000000UL | (1024 << 4);
        GEcmd[10] = 0x4D000000UL | (1024 << 4);
        // Model Color
        GEcmd[11] = 0x54000000UL;
        GEcmd[12] = 0x55000000UL | 0xFFFFFF;
        GEcmd[13] = 0x56000000UL | 0xFFFFFF;
        GEcmd[14] = 0x57000000UL | 0xFFFFFF;
        GEcmd[15] = 0x58000000UL | 0xFF;
        // Depth Buffer
        GEcmd[16] = 0x9E000000UL | 0x88000;
        GEcmd[17] = 0x9F000000UL | (0x44 << 16) | 512;
        // Tex
        GEcmd[18] = 0xC2000000UL | (0 << 16) | (0 << 8) | 0;
        GEcmd[19] = 0xC3000000UL | 1;
        GEcmd[20] = 0xC6000000UL | (1 << 8) | 1;
        GEcmd[21] = 0xC7000000UL | (1 << 8) | 1;
        GEcmd[22] = 0xC9000000UL | (0 << 16) | (0 << 8) | 0;
        // Pixel Format
        GEcmd[23] = 0xD2000000UL | 1;
        // Scissor
        GEcmd[24] = 0xD4000000UL | (0 << 10) | 0;
        GEcmd[25] = 0xD5000000UL | (271 << 10) | 479;
        // Depth
        GEcmd[26] = 0xD6000000UL | 10000;
        GEcmd[27] = 0xD7000000UL | 50000;
        // List End
        GEcmd[28] = 0x0F000000UL;
        GEcmd[29] = 0x0C000000UL;
        GEcmd[30] = 0;
        sceKernelDcacheWritebackAll();
        qid = sceGeListEnQueue(&GEcmd[0], &GEcmd[30], -1, 0);
        sceGeListSync(qid, 0);
    }
}

typedef struct Vertex16 {
    unsigned short u, v;
	short x, y, z;
} Vertex16;


//-----------------------------------------------------------------------------
// GPU bitblt
//-----------------------------------------------------------------------------
static void blt_gpu(int scr_w,int scr_h,void *XBuf,int gfx_sx,int gfx_sy,int gfx_w,int gfx_h)
{
    int ScreenX,ScreenY;
    uint32   * GEcmd  = (uint32  *)0x041e0000; // 勝手にメモリ割当
    uint32   *pGEcmd  = GEcmd;
    Vertex16 * vertex = (Vertex16*)0x041f0000; // 勝手にメモリ割当

    vertex[0].u = gfx_sx;           // Texture展開開始位置:X
    vertex[0].v = gfx_sy;           // Texture展開終了位置:Y
    vertex[1].u = gfx_sx+gfx_w;     // Texture展開終了位置:X
    vertex[1].v = gfx_sy+gfx_h;     // Texture展開終了位置:Y

    // 画面中心に描画ポイントを合わせる
    ScreenX = (SCREEN_WIDTH -scr_w)/2;
    ScreenY = (SCREEN_HEIGHT-scr_h)/2;

    vertex[0].x = ScreenX;          // 画像展開位置:開始点 X
    vertex[0].y = ScreenY;          // 画像展開位置:開始点 Y
    vertex[1].x = ScreenX+scr_w;    // 画像展開位置:終了点 X
    vertex[1].y = ScreenY+scr_h;    // 画像展開位置:終了点 Y
    
    vertex[0].z = vertex[1].z = 0;
    
    // Set Draw Buffer
    *pGEcmd++ = 0x9C000000UL | ((u32)pgGetVramAddr(0,0) & 0x00FFFFFF);
    *pGEcmd++ = 0x9D000000UL |(((u32)pgGetVramAddr(0,0) & 0xFF000000) >> 8) | 512;

    // sceGuClutMode
    *pGEcmd++ = 0xC5000000UL | (1) | (0 << 2) | (0xff << 8) | (0 << 16);

    // sceGuClutLoad
    *pGEcmd++ = 0xB0000000UL | ((u32)g_Pal) & 0x00ffffff;
    *pGEcmd++ = 0xB1000000UL |(((u32)g_Pal) & 0xff000000)>>8;
    *pGEcmd++ = 0xC4000000UL | (256/8); // このパラメータは適当

    // sceGuTexMode
    *pGEcmd++ = 0xC2000000UL | 0;
    *pGEcmd++ = 0xC3000000UL | 5; // GU_PSM_T8;

    // sceGuTexImage
    *pGEcmd++ = 0xA0000000UL | ((u32)XBuf & 0x00FFFFFF);
    *pGEcmd++ = 0xA8000000UL |(((u32)XBuf & 0xFF000000) >> 8) | 256;
    *pGEcmd++ = 0xB8000000UL | (0x08<<8)/*HEIGHT=256(2y8)*/ | (8)/*WIDTH=256(2y8)*/;

    // Tex Flush
    *pGEcmd++ = 0xCB000000UL;

    // sceGuTexFunc
    *pGEcmd++ = 0xC9000000UL | (1<<8) | 3  ; // | 0x10000;
    
    // Set Vertex
    *pGEcmd++ = 0x12000000UL | (1 << 23) | (0 << 11) | (0 << 9) | (2 << 7) | (0 << 5) | (0 << 2) | 2;
    *pGEcmd++ = 0x10000000UL;
    *pGEcmd++ = 0x02000000UL;
    *pGEcmd++ = 0x10000000UL | (((u32)vertex & 0xFF000000) >> 8);
    *pGEcmd++ = 0x01000000UL | ( (u32)vertex & 0x00FFFFFF);
    
    // Draw Vertex
    *pGEcmd++ = 0x04000000UL | (6 << 16) | 2;
    
    // List End
    *pGEcmd++ = 0x0F000000UL;
    *pGEcmd++ = 0x0C000000UL;
    *pGEcmd   = 0;

    sceKernelDcacheWritebackAll();
	
	sceGeListEnQueue(GEcmd,pGEcmd,-1,NULL);
    sceGeDrawSync(0);
}



// GPU使って拡大
void BitBlt_GPU(int width,int height)
{
	static int gpu_init=0;
	register int x,y,w,h;
    uint8 * gpuframe = (uint8*)0x04100000; /* y * x * 16bit = 256 x 256 x 2として扱う */
	register uint8* gpf = (uint8*)gpuframe;
    
	if(gpu_init==0) {
		pgGeInit();
		gpu_init=1;
	}

	/* gpuframebufferに複写する */
	for(y=0;y<224;y++) {
		for(x=0;x<256;x++) {
            *gpf++=g_ScrBuf[y+8][x+8];
        }
	}
	
	/* PPU reg $2001 check : left 8pixel clip */
	x =   8;  y = 0;        /* 変数の再利用カコワルイ */
	w = 248;  h = 224;
	
	// SP/BGのどちらかが表示ONでCLIPOFFなら表示する
	if( ((g_PPU.LowRegs[1]&0x14)==0x14)   // SP-ON && SP-CLIPOFF
	||  ((g_PPU.LowRegs[1]&0x0a)==0x0a) ) // BG-ON && BG-CLIPOFF
	{
		x=0;
		w=256;
	}
	
	blt_gpu(width,height,gpuframe,x,y,w,h);
}


#endif

#if 1

//両端8ドットって拡大
void BitBlt_FullScreen(void)
{
	int x,y, dy = 0;
	unsigned long *pVram;
	pVram = (unsigned long *)pgGetVramAddr(0, 0);
	for (y = 0; y < 224; y++) {
		for (x = 0; x < 240; x+=2) {
			cpy2x(&pVram[x], (g_Pal[g_ScrBuf[y + 8][x + 16 + 1]] << 16) + g_Pal[g_ScrBuf[y + 8][x + 16]]);
		}
		pVram += (LINESIZE/2);
		dy += 21429;
		if (dy >= 100000) {
			dy-=100000;
			for (x = 0; x < 240; x+=2) {
				unsigned long ulColor = PBlend((g_Pal[g_ScrBuf[y + 8    ][x + 16 + 1]] << 16) + g_Pal[g_ScrBuf[y + 8    ][x + 16]],
									         (g_Pal[g_ScrBuf[y + 8 + 1][x + 16 + 1]] << 16) + g_Pal[g_ScrBuf[y + 8 + 1][x + 16]]);
				cpy2x(&pVram[x], ulColor);
			}
			pVram += (LINESIZE/2);
		}
	}
}
#endif /* 0 */

/* 4pixを1.5倍拡大して 6pixを出力
   input : |A|B|C|D| (注:メモリ上のイメージです)
   output: |A|@|B|C|#|D| ("@","#"は隣のpixを合成)*/
static inline void _X15_PIXEL(unsigned long *dist, unsigned char *src){
	unsigned long t0, t1, t2;

	t0  = g_Pal[src[0]];    // t0=|0|A| (注:レジスタ上のイメージです)
	t1  = g_Pal[src[1]];	// t1=|0|B|
	t1 |= g_Pal[src[2]]<<16;// t1=|C|B|
	t0 |= g_Pal[src[3]]<<16;// t0=|D|A|
	t2  = PBlend(t0, t1);	// t2=|#|@|
	*dist     = (t0&0xffff)|(t2<<16);       // |@|A|
	*(dist+1) = t1;                         // |C|B|
	*(dist+2) = (t2>>16)|(t0&0xffff0000);   // |D|#|
}

// x1.5 拡大
static inline unsigned char* _X15_1LINE(unsigned long *pVram, unsigned char *src){
	unsigned long x;
	
	for (x = NES_SCREEN_WIDTH/4; x > 0; x--) {
		_X15_PIXEL(pVram, src);
		pVram+=3;
		src+=4;
	}
	return src+2*SIDE_MARGIN;
}

/* 4lineを1.25倍拡大して 5lineを出力
input: line A,B,C,D
output:
line1 A
line2 (A + B*3)/4
line3 (B + C)/2
line4 (C*3 + D)/4
line5 D
*/
static inline void _X15_5LINE_X125(unsigned long *pVram, unsigned char *src){
	const unsigned long next=2*SIDE_MARGIN+NES_SCREEN_WIDTH;
	const unsigned long NVL = LINESIZE/2;/* next vram line*/
	unsigned long x;
	
	for(x = NES_SCREEN_WIDTH/4; x > 0; x--){
		unsigned long L0[3], L1[3];
		unsigned long t0, t1, t2;

		/* 1st line*/
		_X15_PIXEL(L0, src);
		*(pVram) 	= L0[0];
		*(pVram+1)	= L0[1];
		*(pVram+2)	= L0[2];

		/* 2nd line [Blend Ratio 3:1] */
		_X15_PIXEL(L1, src+next);
		*(pVram+NVL) 	= PBlend_3to1(L1[0], L0[0]);
		*(pVram+NVL+1)	= PBlend_3to1(L1[1], L0[1]);
		*(pVram+NVL+2)	= PBlend_3to1(L1[2], L0[2]);

		/* 3rd line [Blend Ratio 1:1] */
		_X15_PIXEL(L0, src+next*2);
		*(pVram+NVL*2) 		= PBlend(L0[0], L1[0]);
		*(pVram+NVL*2+1)	= PBlend(L0[1], L1[1]);
		*(pVram+NVL*2+2)	= PBlend(L0[2], L1[2]);

		/* 4th line [Blend Ratio 1:3] */
		_X15_PIXEL(L1, src+next*3);
		*(pVram+NVL*3)		= PBlend_3to1(L0[0], L1[0]);
		*(pVram+NVL*3+1)	= PBlend_3to1(L0[1], L1[1]);
		*(pVram+NVL*3+2)	= PBlend_3to1(L0[2], L1[2]);

		/* 5th line */
		*(pVram+NVL*4) 		= L1[0];
		*(pVram+NVL*4+1)	= L1[1];
		*(pVram+NVL*4+2)	= L1[2];
		/* END OF 5th line*/

		pVram+=3;
		src+=4;
	}
}

//WIDTH x1.5 HEIGHT x1.2142 224/14*(14+3)=272line
void BitBlt_X15(void)
{
	unsigned long x, y;
	unsigned long *pVram;
	unsigned char *src;

	pVram = (unsigned long *)pgGetVramAddr((480-256*3/2)/2, 0);
	src = (unsigned char*)&g_ScrBuf[8][8];
	
	for (y = 224/14; y > 0; y--) {

		_X15_5LINE_X125( pVram, src);
			src   += NES_BACKBUF_WIDTH*4;
			pVram += LINESIZE/2*5;

		_X15_1LINE( pVram, src);
			src   += NES_BACKBUF_WIDTH;
			pVram += LINESIZE/2;

		_X15_5LINE_X125( pVram, src);
			src   += NES_BACKBUF_WIDTH*4;
			pVram += LINESIZE/2*5;

		_X15_1LINE( pVram, src);
			src   += NES_BACKBUF_WIDTH;
			pVram += LINESIZE/2;

		_X15_5LINE_X125( pVram, src);
			src   += NES_BACKBUF_WIDTH*4;
			pVram += LINESIZE/2*5;
	}
}

//WIDTH x1.5 HEIGHT x1.25 上下3dotずつカット (224-4*2)/4*(4+1)+2=272line 
void BitBlt_X15_CROPPED(void)
{
	unsigned long x, y;
	unsigned long *pVram;
	unsigned char *src;
	
	pVram = (unsigned long *)pgGetVramAddr((480-256*3/2)/2, 0);
	src = (unsigned char*)&g_ScrBuf[8+3][8];
	
	_X15_1LINE( pVram, src);
		src   += NES_BACKBUF_WIDTH;
		pVram += LINESIZE/2;
	
	for (y = (224-4*2)/4; y > 0; y--) {
		_X15_5LINE_X125( pVram, src);
			src   += NES_BACKBUF_WIDTH*4;
			pVram += LINESIZE/2*5;
	}
	
	_X15_1LINE( pVram, src);
}








/*
/////////////////////////////////////////////////
// Deflate/Inflate

// サムネイル圧縮用に作ったがUnziplibのコードがコメントアウトされてた･･･
// unziplibのzip圧縮対応までコメントアウトとする

size_t DeflateData(void *pOut, void *pIn, size_t inSize)
{
	z_stream zStream;
	_memset(&zStream, 0x00, sizeof(zStream));
	deflateInit(&zStream, Z_DEFAULT_COMPRESSION);

	zStream.next_in = pIn;
	zStream.avail_in = inSize;
	zStream.next_out = pOut;
	zStream.avail_out = inSize;

	int nRet = deflate(&zStream, Z_FINISH);

	deflateEnd(&zStream);
	return zStream.avail_out;
}

size_t InflateData(void *pOut, size_t outSize, void *pIn, size_t inSize)
{
	z_stream zStream;
	_memset(&zStream, 0x00, sizeof(zStream));
	inflateInit(&zStream);

	zStream.next_in = pIn;
	zStream.avail_in = inSize;
	zStream.next_out = pOut;
	zStream.avail_out = outSize;

	int nRet = inflate(&zStream, Z_NO_FLUSH);
	deflateEnd(&zStream);
	return zStream.avail_out;
}
*/

/////////////////////////////////////////////////
// thumbnail
// 128x112、16Bit固定です。

#define THUMB_RGBMASK 0x1CE7	// = 0BBBBBGGGGGRRRRR = 0001110011100111B

// create thumbnail
boolean CreateThumnailFile(char *szThumbnailPath)
{
	// 小さいしローカルで(小さいか？)
	uint16 thumbnail[112][128];
	int x,y;
	// create thumbnail forom palette and index buffer.
	for (y = 0; y < 224; y+=2) {
		for (x = 0; x < 256; x+=2) {
			// 最近傍法は当然結果がアレだったので線形補間法に変更
			// 表示範囲飛び越えてるが元々表示範囲以上のバッファがあるので無視
			//thumbnail[y/2][x/2] = g_Pal[g_ScrBuf[y + 8][x + 8]];
			thumbnail[y/2][x/2] =
				((g_Pal[g_ScrBuf[y + 8  ][x + 8  ]] >> 2) & THUMB_RGBMASK)+
				((g_Pal[g_ScrBuf[y + 8  ][x + 8+1]] >> 2) & THUMB_RGBMASK)+
				((g_Pal[g_ScrBuf[y + 8+1][x + 8  ]] >> 2) & THUMB_RGBMASK)+
				((g_Pal[g_ScrBuf[y + 8+1][x + 8+1]] >> 2) & THUMB_RGBMASK);
		}
	}
	// output file
	HANDLE hFile = NES_fopen(szThumbnailPath, FILE_MODE_WRITE);
	if (hFile >= 0) {
		int nWriteSize = NES_fwrite(thumbnail, sizeof(thumbnail), 1, hFile);
		NES_fclose(hFile);
		if (nWriteSize == sizeof(thumbnail)) return TRUE;
		// 容量不足っぽいので、ゴミデータ破棄
		sceIoRemove(szThumbnailPath);
	}
	return FALSE;
}

// load thumbnail
boolean LoadThumnailFile(char *szThumbnailPath, uint16 *pBuf)
{
	// load file
	HANDLE hFile = NES_fopen(szThumbnailPath, FILE_MODE_READ);
	if (hFile >= 0) {
		NES_fread(pBuf, sizeof(uint16)*128*112, 1, hFile);
		// Readsize == sizeof(uint16)*128*112 なら非圧縮サムネイル
		NES_fclose(hFile);
		return TRUE;
	}
	return FALSE;
}
