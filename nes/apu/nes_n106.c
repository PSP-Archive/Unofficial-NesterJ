/*
** Namco 106 ExSound by TAKEDA, toshiya
**
** original: s_n106.c in nezp0922
*/
#include "nes_apu.h"
#include "../nes_string.h"

extern apu_t g_apu_t;


__inline static void UPDATE(N106_WM *chp)
{
	if (chp->update & 3)
	{
		uint32 freq;
		freq  = ((int)chp->freql);
		freq += ((int)chp->freqm) << 8;
		freq += ((int)chp->freqh) << 16;
		chp->spd = freq & 0x3ffff;
	}
	if (chp->update & 2)
	{
		uint32 tlen;
		tlen = (0x20 - (chp->freqh & 0x1c)) << 18;
		if (chp->tlen != tlen)
		{
			chp->tlen = tlen;
			chp->phase = 0;
		}
	}
	if (chp->update & 4)
	{
		chp->logvol = LinearToLog((chp->vreg & 0x0f) << 2);
	}
	chp->update = 0;
}

static int32 N106SoundRender(void)
{
	N106_WM *chp;
	int32 accum = 0;
	for (chp = &g_apu_t.n106s.ch[8 - g_apu_t.n106s.chinuse]; chp < &g_apu_t.n106s.ch[8]; chp++)
	{
		uint32 cyclesspd = g_apu_t.n106s.chinuse << 20;
		if (chp->update) UPDATE(chp);
		chp->cycles -= g_apu_t.n106s.cps;
		while (chp->cycles < 0)
		{
			chp->cycles += cyclesspd;
			chp->phase += chp->spd;
		}
		while (chp->phase >= chp->tlen) chp->phase -= chp->tlen;
		if (chp->mute) continue;
		accum += LogToLinear(g_apu_t.n106s.tone[((chp->phase >> 18) + chp->tadr) & 0xff] + chp->logvol + g_apu_t.n106s.mastervolume, LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10);
	}
	return accum >> 8;
}

static void N106SoundVolume(uint32 volume)
{
	g_apu_t.n106s.mastervolume = (volume << (LOG_BITS - 8)) << 1;
}

static void N106SoundWriteAddr(uint32 address, uint8 value)
{
	g_apu_t.n106s.address     = value & 0x7f;
	g_apu_t.n106s.addressauto = (value & 0x80) ? 1 : 0;
}

static void N106SoundWriteData(uint32 address, uint8 value)
{
	g_apu_t.n106s.data[g_apu_t.n106s.address] = value;
	g_apu_t.n106s.tone[g_apu_t.n106s.address * 2]     = LinearToLog(((int)(value & 0xf) << 2) - 0x20);
	g_apu_t.n106s.tone[g_apu_t.n106s.address * 2 + 1] = LinearToLog(((int)(value >>  4) << 2) - 0x20);
	if (g_apu_t.n106s.address >= 0x40)
	{
		N106_WM *chp = &g_apu_t.n106s.ch[(g_apu_t.n106s.address - 0x40) >> 3];
		switch (g_apu_t.n106s.address & 7)
		{
			case 0:
				chp->update |= 1;
				chp->freql = value;
				break;
			case 2:
				chp->update |= 1;
				chp->freqm = value;
				break;
			case 4:
				chp->update |= 2;
				chp->freqh = value;
				break;
			case 6:
				chp->tadr = value & 0xff;
				break;
			case 7:
				chp->update |= 4;
				chp->vreg = value;
				chp->nazo = (value >> 4) & 0x07;
				if (chp == &g_apu_t.n106s.ch[7])
					g_apu_t.n106s.chinuse = 1 + chp->nazo;
				break;
		}
	}
	if (g_apu_t.n106s.addressauto)
	{
		g_apu_t.n106s.address = (g_apu_t.n106s.address + 1) & 0x7f;
	}
}

static void N106SoundWrite(uint32 address, uint8 value)
{
	if (address == 0x4800)
	{
		N106SoundWriteData(address, value);
	}
	else if (address == 0xF800)
	{
		N106SoundWriteAddr(address, value);
	}
}

static uint8 N106SoundReadData(uint32 address)
{
	uint8 ret = g_apu_t.n106s.data[g_apu_t.n106s.address];
	if (g_apu_t.n106s.addressauto)
	{
		g_apu_t.n106s.address = (g_apu_t.n106s.address + 1) & 0x7f;
	}
	return ret;
}

static uint8 N106SoundRead(uint32 address)
{
	if (address == 0x4800)
	{
		return N106SoundReadData(address);
	}
	else
	{
		return 0x00;
	}
}

static void N106SoundReset(void)
{
	int i;

	apu_t *apu;
	apu = apu_getcontext ();

	_memset (&g_apu_t.n106s, 0, sizeof(N106SOUND));
	for (i = 0; i < 8; i++)
	{
		g_apu_t.n106s.ch[i].tlen = 0x10 << 18;
		g_apu_t.n106s.ch[i].logvol = LinearToLog(0);
	}

	g_apu_t.n106s.addressauto = 1;
	g_apu_t.n106s.chinuse = 8;
	g_apu_t.n106s.cps = DivFix(NES_BASECYCLES, 45 * SAMPLE_RATE, 20);
}

static int N106SoundInit(void){
	N106SoundReset();
	N106SoundVolume(1);
	return 0;
}


static void N106SoundWriteDR(uint32 address, uint8 value)
{
	N106SoundRead(address & 0xFFFF);
}


static apu_memwrite n106_memwrite[] = {
   { 0x4800, 0x4800, N106SoundWrite },
   { 0xF800, 0xF800, N106SoundWrite },
   { 0x10000, 0x1FFFF, N106SoundWriteDR },
   {     -1,     -1, NULL }
};

apuext_t n106_ext = {
	N106SoundInit,		/* init */
	NULL,		/* shutdown */
	N106SoundReset,	/* reset */
	NULL,		/* paramschanged */
	N106SoundRender,	/* process */
	NULL,			/* mem_read */
	n106_memwrite,	/* mem_write */
	NULL			/* mem_writesync */
};

