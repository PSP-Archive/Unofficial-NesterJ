/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
** NNNeserJでの継承クラス構造をCで再現しています。 ruka
**
*/



#define _NES_MAPPER_CPP_

#include "nes_mapper.h"
#include "nes.h"

#include "../debug/debug.h"

NES_mapper g_NESmapper;

#define MASK_BANK(bank,mask) (bank) = ((bank) & (mask))

#ifdef NESTER_DEBUG
  #define VALIDATE_ROM_BANK(bank) \
    MASK_BANK(bank,g_NESmapper.ROM_mask); \
    ASSERT((bank) < g_NESmapper.num_8k_ROM_banks) \
    if((bank) >= g_NESmapper.num_8k_ROM_banks) \
    { \
      DEBUGVALUE("Illegal ROM bank switch: ",bank); \
      DEBUGVALUE("g_NESmapper.num_8k_ROM_banks", g_NESmapper.num_8k_ROM_banks); \
      return; \
    }

  #define VALIDATE_VROM_BANK(bank) \
    MASK_BANK(bank,g_NESmapper.VROM_mask); \
    ASSERT((bank) < g_NESmapper.num_1k_VROM_banks) \
    if((bank) >= g_NESmapper.num_1k_VROM_banks) \
    { \
      DEBUGVALUE("Illegal VROM bank switch: ",bank); \
      DEBUGVALUE("g_NESmapper.num_1k_VROM_banks", g_NESmapper.num_1k_VROM_banks); \
      return; \
    }
#else
  #define VALIDATE_ROM_BANK(bank) \
    MASK_BANK(bank,g_NESmapper.ROM_mask); \
    if((bank) >= g_NESmapper.num_8k_ROM_banks) return;
  #define VALIDATE_VROM_BANK(bank) \
    MASK_BANK(bank,g_NESmapper.VROM_mask); \
    if((bank) >= g_NESmapper.num_1k_VROM_banks) return;
#endif

// NES mapper base
void  NES_mapper_base_Reset() {};
uint8 NES_mapper_base_MemoryReadLow(uint32 addr) {return (uint8)(addr >> 8);}
void  NES_mapper_base_WriteHighRegs(uint32 addr, uint8 data) {}
void  NES_mapper_base_MemoryWrite(uint32 addr, uint8 data) {}
void  NES_mapper_base_MemoryWriteLow(uint32 addr, uint8 data) {}
void  NES_mapper_base_MemoryWriteSaveRAM(uint32 addr, uint8 data) {}
void  NES_mapper_base_MemoryReadSaveRAM(uint32 addr) {}
void  NES_mapper_base_HSync(uint32 scanline)  {}
void  NES_mapper_base_VSync() {}
// for mmc2 & mmc5 & Oekakidds
void  NES_mapper_base_PPU_Latch_FDFE(uint32 addr) {}
uint8 NES_mapper_base_PPU_Latch_RenderScreen(uint8 mode, uint32 addr) {return 0;}
void  NES_mapper_base_PPU_Latch_Address(uint32 addr) {}
// for disk system (#20)
uint8 NES_mapper_base_GetDiskSideNum() {return 0;}
uint8 NES_mapper_base_GetDiskSide() {return 0;}
void  NES_mapper_base_SetDiskSide(uint8 side) {}
uint8 *NES_mapper_base_GetDiskDatap() {return 0;}
uint8 NES_mapper_base_DiskAccessed() {return 0;}
// for Datach Barcode Battler
void NES_mapper_base_SetBarcodeValue(uint32 value_low, uint32 value_high) {}

void NES_mapper_base_set_CPU_banks4(uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
	nes6502_context context;

	VALIDATE_ROM_BANK(bank4_num);
	VALIDATE_ROM_BANK(bank5_num);
	VALIDATE_ROM_BANK(bank6_num);
	VALIDATE_ROM_BANK(bank7_num);

	/*
	  LOG("Setting CPU banks " << bank4_num << " " << bank5_num << " " <<
	                              bank6_num << " " << bank7_num << endl);
	*/

	NES6502_GetContext(&context);
	context.mem_page[4] = g_NESmapper.ROM_banks + (bank4_num << 13); // * 0x2000
	context.mem_page[5] = g_NESmapper.ROM_banks + (bank5_num << 13);
	context.mem_page[6] = g_NESmapper.ROM_banks + (bank6_num << 13);
	context.mem_page[7] = g_NESmapper.ROM_banks + (bank7_num << 13);

	NES6502_SetContext(&context);
	g_NESmapper.set_genie();
}

void NES_mapper_base_set_CPU_bank4(uint32 bank_num)
{
	nes6502_context context;

	VALIDATE_ROM_BANK(bank_num);

	NES6502_GetContext(&context);
	context.mem_page[4] = g_NESmapper.ROM_banks + (bank_num << 13); // * 0x2000
	NES6502_SetContext(&context);

	g_NESmapper.set_genie();
}

void NES_mapper_base_set_CPU_bank5(uint32 bank_num)
{
	nes6502_context context;

	VALIDATE_ROM_BANK(bank_num);

	NES6502_GetContext(&context);
	context.mem_page[5] = g_NESmapper.ROM_banks + (bank_num << 13); // * 0x2000
	NES6502_SetContext(&context);
	g_NESmapper.set_genie();
}

void NES_mapper_base_set_CPU_bank6(uint32 bank_num)
{
	nes6502_context context;

	VALIDATE_ROM_BANK(bank_num);

	NES6502_GetContext(&context);
	context.mem_page[6] = g_NESmapper.ROM_banks + (bank_num << 13); // * 0x2000
	NES6502_SetContext(&context);
	g_NESmapper.set_genie();
}

void NES_mapper_base_set_CPU_bank7(uint32 bank_num)
{
	nes6502_context context;

	VALIDATE_ROM_BANK(bank_num);

	NES6502_GetContext(&context);
	context.mem_page[7] = g_NESmapper.ROM_banks + (bank_num << 13); // * 0x2000
	NES6502_SetContext(&context);

	g_NESmapper.set_genie();
}


// for UNIF
void NES_mapper_base_set_CPU_bank_unif(uint32 kn, uint8 pagen){
	nes6502_context context;
	VALIDATE_ROM_BANK(kn);
//	LOG("ROM bank switch: " << HEX(blockn,2) << ", " << HEX(kn,2) << endl);
	NES6502_GetContext(&context);
	context.mem_page[pagen] = g_NESmapper.ROM_banks + (kn << 13); // * 0x2000
	NES6502_SetContext(&context);
	g_NESmapper.set_genie();
}

void NES_mapper_base_set_VRAM_bank_unif(uint32 kn, uint8 pagen){
	VALIDATE_VROM_BANK(kn);
	g_PPU.PPU_VRAM_banks[pagen] = g_NESmapper.VROM_banks + (kn << 10); // * 0x400
//	NES_PPU_set_pattype(0, 1);
}


// for mapper 40 /////////////////////////////////////////////////////////
void NES_mapper_base_set_CPU_banks5(uint32 bank3_num,
                               uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
	nes6502_context context;

	VALIDATE_ROM_BANK(bank3_num);
	VALIDATE_ROM_BANK(bank4_num);
	VALIDATE_ROM_BANK(bank5_num);
	VALIDATE_ROM_BANK(bank6_num);
	VALIDATE_ROM_BANK(bank7_num);

	NES6502_GetContext(&context);
	context.mem_page[3] = g_NESmapper.ROM_banks + (bank3_num << 13); // * 0x2000
	context.mem_page[4] = g_NESmapper.ROM_banks + (bank4_num << 13);
	context.mem_page[5] = g_NESmapper.ROM_banks + (bank5_num << 13);
	context.mem_page[6] = g_NESmapper.ROM_banks + (bank6_num << 13);
	context.mem_page[7] = g_NESmapper.ROM_banks + (bank7_num << 13);
	NES6502_SetContext(&context);

	g_NESmapper.set_genie();
}

void NES_mapper_base_set_CPU_bank3(uint32 bank_num)
{
	nes6502_context context;

	VALIDATE_ROM_BANK(bank_num);

	NES6502_GetContext(&context);
	context.mem_page[3] = g_NESmapper.ROM_banks + (bank_num << 13); // * 0x2000
	NES6502_SetContext(&context);
}
//////////////////////////////////////////////////////////////////////////


void NES_mapper_base_set_PPU_banks8(uint32 bank0_num, uint32 bank1_num,
                               uint32 bank2_num, uint32 bank3_num,
                               uint32 bank4_num, uint32 bank5_num,
                               uint32 bank6_num, uint32 bank7_num)
{
	VALIDATE_VROM_BANK(bank0_num);
	VALIDATE_VROM_BANK(bank1_num);
	VALIDATE_VROM_BANK(bank2_num);
	VALIDATE_VROM_BANK(bank3_num);
	VALIDATE_VROM_BANK(bank4_num);
	VALIDATE_VROM_BANK(bank5_num);
	VALIDATE_VROM_BANK(bank6_num);
	VALIDATE_VROM_BANK(bank7_num);

	g_PPU.PPU_VRAM_banks[0] = g_NESmapper.VROM_banks + (bank0_num << 10); // * 0x400
	g_PPU.PPU_VRAM_banks[1] = g_NESmapper.VROM_banks + (bank1_num << 10);
	g_PPU.PPU_VRAM_banks[2] = g_NESmapper.VROM_banks + (bank2_num << 10);
	g_PPU.PPU_VRAM_banks[3] = g_NESmapper.VROM_banks + (bank3_num << 10);
	g_PPU.PPU_VRAM_banks[4] = g_NESmapper.VROM_banks + (bank4_num << 10);
	g_PPU.PPU_VRAM_banks[5] = g_NESmapper.VROM_banks + (bank5_num << 10);
	g_PPU.PPU_VRAM_banks[6] = g_NESmapper.VROM_banks + (bank6_num << 10);
	g_PPU.PPU_VRAM_banks[7] = g_NESmapper.VROM_banks + (bank7_num << 10);

	NES_PPU_set_pattype(0, 1);
	NES_PPU_set_pattype(1, 1);
	NES_PPU_set_pattype(2, 1);
	NES_PPU_set_pattype(3, 1);
	NES_PPU_set_pattype(4, 1);
	NES_PPU_set_pattype(5, 1);
	NES_PPU_set_pattype(6, 1);
	NES_PPU_set_pattype(7, 1);
}

void NES_mapper_base_set_PPU_bank0(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[0] = g_NESmapper.VROM_banks + (bank_num << 10); // * 0x400
	NES_PPU_set_pattype(0, 1);
}

void NES_mapper_base_set_PPU_bank1(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[1] = g_NESmapper.VROM_banks + (bank_num << 10); // * 0x400
	NES_PPU_set_pattype(1, 1);
}

void NES_mapper_base_set_PPU_bank2(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[2] = g_NESmapper.VROM_banks + (bank_num << 10); // * 0x400
	NES_PPU_set_pattype(2, 1);
}

void NES_mapper_base_set_PPU_bank3(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[3] = g_NESmapper.VROM_banks + (bank_num << 10); // * 0x400
	NES_PPU_set_pattype(3, 1);
}

void NES_mapper_base_set_PPU_bank4(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[4] = g_NESmapper.VROM_banks + (bank_num << 10); // * 0x400
	NES_PPU_set_pattype(4, 1);
}

void NES_mapper_base_set_PPU_bank5(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[5] = g_NESmapper.VROM_banks + (bank_num << 10); // * 0x400
	NES_PPU_set_pattype(5, 1);
}

void NES_mapper_base_set_PPU_bank6(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[6] = g_NESmapper.VROM_banks + (bank_num << 10); // * 0x400
	NES_PPU_set_pattype(6, 1);
}

void NES_mapper_base_set_PPU_bank7(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[7] = g_NESmapper.VROM_banks + (bank_num << 10); // * 0x400
	NES_PPU_set_pattype(7, 1);
}

// for mapper 19,68,90
void NES_mapper_base_set_PPU_bank8(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[8] = g_NESmapper.VROM_banks + (bank_num << 10);
}
void NES_mapper_base_set_PPU_bank9(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[9] = g_NESmapper.VROM_banks + (bank_num << 10);
}
void NES_mapper_base_set_PPU_bank10(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[10] = g_NESmapper.VROM_banks + (bank_num << 10);
}
void NES_mapper_base_set_PPU_bank11(uint32 bank_num)
{
	VALIDATE_VROM_BANK(bank_num);
	g_PPU.PPU_VRAM_banks[11] = g_NESmapper.VROM_banks + (bank_num << 10);
}

// for mapper 1,4,5,6,13,19,80,85,96,119
void NES_mapper_base_set_VRAM_bank(uint8 bank, uint32 bank_num)
{
	if(bank < 8)
	{
		g_PPU.PPU_VRAM_banks[bank] = NES_PPU_get_patt() + ((bank_num & 0x0f) << 10);
		NES_PPU_set_pattype(bank, 0);
	}
	else if(bank < 12)
	{
		g_PPU.PPU_VRAM_banks[bank] = NES_PPU_get_namt() + ((bank_num & 0x03) << 10);
	}
}

void NES_mapper_base_set_mirroring(uint32 nt0, uint32 nt1, uint32 nt2, uint32 nt3)
{
	ASSERT(nt0 < 4);
	ASSERT(nt1 < 4);
	ASSERT(nt2 < 4);
	ASSERT(nt3 < 4);

	NES_PPU_set_mirroring(nt0,nt1,nt2,nt3);
}

void NES_mapper_base_set_mirroring2(mirroring_type m)
{
	NES_PPU_set_mirroring2(m);
}

void NES_mapper_base_set_genie()
{
	nes6502_context context;
	uint8 genie_num;
	uint8 i;

	NES6502_GetContext(&context);

	genie_num = NES_GetGenieCodeNum();

	for(i = 0; i < genie_num; i++)
	{
		uint32 genie_code = NES_GetGenieCode(i);
		uint32 addr = ((genie_code & 0x7FFF0000) >> 16) | 0x8000;
		uint8 data1 = genie_code & 0x000000FF;
		uint8 data2 = (genie_code & 0x0000FF00) >> 8;
		uint8 data3 = context.mem_page[addr >> 13][addr & 0x1FFF];
		if(!(genie_code & 0x80000000) || data2 == data3)
		{
			context.mem_page[addr >> 13][addr & 0x1FFF] = data1;
		}
	}
}

/////////////////////////////////////////////////////////////////////

#include "mapper/000.cpp"
#include "mapper/001.cpp"
#include "mapper/002.cpp"
#include "mapper/003.cpp"
#include "mapper/004.cpp"
#include "mapper/005.cpp"
#include "mapper/006.cpp"
#include "mapper/007.cpp"
#include "mapper/008.cpp"
#include "mapper/009.cpp"
#include "mapper/010.cpp"
#include "mapper/011.cpp"
//#include "mapper/012.cpp"
#include "mapper/013.cpp"
//#include "mapper/014.cpp"
#include "mapper/015.cpp"
#include "mapper/016.cpp"
#include "mapper/017.cpp"
#include "mapper/018.cpp"
#include "mapper/019.cpp"
#include "mapper/020.cpp"
#include "mapper/021.cpp"
#include "mapper/022.cpp"
#include "mapper/023.cpp"
#include "mapper/024.cpp"
#include "mapper/025.cpp"
#include "mapper/026.cpp"
//#include "mapper/027.cpp"
//#include "mapper/028.cpp"
//#include "mapper/029.cpp"
//#include "mapper/030.cpp"
//#include "mapper/031.cpp"
#include "mapper/032.cpp"
#include "mapper/033.cpp"
#include "mapper/034.cpp"
//#include "mapper/035.cpp"
//#include "mapper/036.cpp"
//#include "mapper/037.cpp"
//#include "mapper/038.cpp"
//#include "mapper/039.cpp"
#include "mapper/040.cpp"
#include "mapper/041.cpp"
#include "mapper/042.cpp"
#include "mapper/043.cpp"
#include "mapper/044.cpp"
#include "mapper/045.cpp"
#include "mapper/046.cpp"
#include "mapper/047.cpp"
#include "mapper/048.cpp"
#include "mapper/049.cpp"
#include "mapper/050.cpp"
#include "mapper/051.cpp"
#include "mapper/052.cpp"
//#include "mapper/053.cpp"
//#include "mapper/054.cpp"
//#include "mapper/055.cpp"
//#include "mapper/056.cpp"
#include "mapper/057.cpp"
#include "mapper/058.cpp"
//#include "mapper/059.cpp"
#include "mapper/060.cpp"
//#include "mapper/061.cpp"
#include "mapper/062.cpp"
//#include "mapper/063.cpp"
#include "mapper/064.cpp"
#include "mapper/065.cpp"
#include "mapper/066.cpp"
#include "mapper/067.cpp"
#include "mapper/068.cpp"
#include "mapper/069.cpp"
#include "mapper/070.cpp"
#include "mapper/071.cpp"
#include "mapper/072.cpp"
#include "mapper/073.cpp"
#include "mapper/074.cpp"
#include "mapper/075.cpp"
#include "mapper/076.cpp"
#include "mapper/077.cpp"
#include "mapper/078.cpp"
#include "mapper/079.cpp"
#include "mapper/080.cpp"
//#include "mapper/081.cpp"
#include "mapper/082.cpp"
#include "mapper/083.cpp"
//#include "mapper/084.cpp"
#include "mapper/085.cpp"
#include "mapper/086.cpp"
#include "mapper/087.cpp"
#include "mapper/088.cpp"
#include "mapper/089.cpp"
#include "mapper/090.cpp"
#include "mapper/091.cpp"
#include "mapper/092.cpp"
#include "mapper/093.cpp"
#include "mapper/094.cpp"
#include "mapper/095.cpp"
#include "mapper/096.cpp"
#include "mapper/097.cpp"
//#include "mapper/098.cpp"
#include "mapper/099.cpp"
#include "mapper/100.cpp"
#include "mapper/101.cpp"
//#include "mapper/102.cpp"
//#include "mapper/103.cpp"
//#include "mapper/104.cpp"
#include "mapper/105.cpp"
//#include "mapper/106.cpp"
//#include "mapper/107.cpp"
//#include "mapper/108.cpp"
//#include "mapper/109.cpp"
//#include "mapper/110.cpp"
//#include "mapper/111.cpp"
#include "mapper/112.cpp"
#include "mapper/113.cpp"
#include "mapper/114.cpp"
#include "mapper/115.cpp"
//#include "mapper/116.cpp"
#include "mapper/117.cpp"
#include "mapper/118.cpp"
#include "mapper/119.cpp"
//#include "mapper/120.cpp"
//#include "mapper/121.cpp"
#include "mapper/122.cpp"
//#include "mapper/123.cpp"
//#include "mapper/124.cpp"
//#include "mapper/125.cpp"
//#include "mapper/126.cpp"
//#include "mapper/127.cpp"
//#include "mapper/128.cpp"
//#include "mapper/129.cpp"
//#include "mapper/130.cpp"
//#include "mapper/131.cpp"
//#include "mapper/132.cpp"
//#include "mapper/133.cpp"
//#include "mapper/134.cpp"
//#include "mapper/135.cpp"
//#include "mapper/136.cpp"
//#include "mapper/137.cpp"
//#include "mapper/138.cpp"
//#include "mapper/139.cpp"
#include "mapper/140.cpp"
//#include "mapper/141.cpp"
//#include "mapper/142.cpp"
//#include "mapper/143.cpp"
//#include "mapper/144.cpp"
//#include "mapper/145.cpp"
//#include "mapper/146.cpp"
//#include "mapper/147.cpp"
//#include "mapper/148.cpp"
//#include "mapper/149.cpp"
//#include "mapper/150.cpp"
#include "mapper/151.cpp"
//#include "mapper/152.cpp"
//#include "mapper/153.cpp"
//#include "mapper/154.cpp"
//#include "mapper/155.cpp"
//#include "mapper/156.cpp"
//#include "mapper/157.cpp"
//#include "mapper/158.cpp"
//#include "mapper/159.cpp"
#include "mapper/160.cpp"
//#include "mapper/161.cpp"
//#include "mapper/162.cpp"
//#include "mapper/163.cpp"
//#include "mapper/164.cpp"
//#include "mapper/165.cpp"
//#include "mapper/166.cpp"
//#include "mapper/167.cpp"
//#include "mapper/168.cpp"
//#include "mapper/169.cpp"
//#include "mapper/170.cpp"
//#include "mapper/171.cpp"
//#include "mapper/172.cpp"
//#include "mapper/173.cpp"
//#include "mapper/174.cpp"
//#include "mapper/175.cpp"
//#include "mapper/176.cpp"
//#include "mapper/177.cpp"
//#include "mapper/178.cpp"
//#include "mapper/179.cpp"
#include "mapper/180.cpp"
#include "mapper/181.cpp"
#include "mapper/182.cpp"
#include "mapper/183.cpp"
//#include "mapper/184.cpp"
#include "mapper/185.cpp"
//#include "mapper/186.cpp"
#include "mapper/187.cpp"
#include "mapper/188.cpp"
#include "mapper/189.cpp"
//#include "mapper/190.cpp"
//#include "mapper/191.cpp"
//#include "mapper/192.cpp"
//#include "mapper/193.cpp"
//#include "mapper/194.cpp"
//#include "mapper/195.cpp"
//#include "mapper/196.cpp"
//#include "mapper/197.cpp"
//#include "mapper/198.cpp"
//#include "mapper/199.cpp"
//#include "mapper/200.cpp"
//#include "mapper/201.cpp"
//#include "mapper/202.cpp"
//#include "mapper/203.cpp"
//#include "mapper/204.cpp"
//#include "mapper/205.cpp"
//#include "mapper/206.cpp"
//#include "mapper/207.cpp"
//#include "mapper/208.cpp"
//#include "mapper/209.cpp"
//#include "mapper/210.cpp"
//#include "mapper/211.cpp"
//#include "mapper/212.cpp"
//#include "mapper/213.cpp"
//#include "mapper/214.cpp"
//#include "mapper/215.cpp"
//#include "mapper/216.cpp"
//#include "mapper/217.cpp"
//#include "mapper/218.cpp"
//#include "mapper/219.cpp"
//#include "mapper/220.cpp"
//#include "mapper/221.cpp"
#include "mapper/222.cpp"
//#include "mapper/223.cpp"
//#include "mapper/224.cpp"
#include "mapper/225.cpp"
#include "mapper/226.cpp"
#include "mapper/227.cpp"
#include "mapper/228.cpp"
#include "mapper/229.cpp"
#include "mapper/230.cpp"
#include "mapper/231.cpp"
#include "mapper/232.cpp"
#include "mapper/233.cpp"
#include "mapper/234.cpp"
#include "mapper/235.cpp"
#include "mapper/236.cpp"
#include "mapper/237.cpp"
//#include "mapper/238.cpp"
//#include "mapper/239.cpp"
#include "mapper/240.cpp"
#include "mapper/241.cpp"
#include "mapper/242.cpp"
#include "mapper/243.cpp"
#include "mapper/244.cpp"
#include "mapper/245.cpp"
#include "mapper/246.cpp"
//#include "mapper/247.cpp"
#include "mapper/248.cpp"
#include "mapper/249.cpp"
#include "mapper/250.cpp"
#include "mapper/251.cpp"
#include "mapper/252.cpp"
//#include "mapper/253.cpp"
#include "mapper/254.cpp"
#include "mapper/255.cpp"
#include "mapper/NSF.cpp"

#include "mapper/unif001.cpp"
#include "mapper/unif002.cpp"
#include "mapper/unif003.cpp"
#include "mapper/unif004.cpp"


/////////////////////////////////////////////////////////////////////
//

/////////////////////////////////////////////////////////////////////
// g_NESmapperを初期化する。
void NES_mapper_Init(void)
{
	uint32 probe;
	// 一応初期化
	_memset(&g_NESmapper, 00, sizeof(g_NESmapper));

	if(NES_ROM_get_unifmapper_num()){
		g_NESmapper.num_16k_ROM_banks = NES_ROM_get_UNIF_psize_16k();
		g_NESmapper.num_8k_ROM_banks = g_NESmapper.num_16k_ROM_banks<<1;
		g_NESmapper.num_1k_VROM_banks = NES_ROM_get_UNIF_csize_8k()<<3;
	}
	else{
		//num_16k_ROM_banks = NES_ROM_get_num_16k_ROM_banks();
		g_NESmapper.num_8k_ROM_banks = 2 * NES_ROM_get_num_16k_ROM_banks();
		g_NESmapper.num_1k_VROM_banks = 8 * NES_ROM_get_num_8k_VROM_banks();
	}

	g_NESmapper.ROM_banks  = NES_ROM_get_ROM_banks();
	g_NESmapper.VROM_banks = NES_ROM_get_VROM_banks();


	g_NESmapper.ROM_mask  = 0xFFFF;
	g_NESmapper.VROM_mask = 0xFFFF;

	for(probe = 0x8000; probe; probe >>= 1)
	{
		if((g_NESmapper.num_8k_ROM_banks-1) & probe) break;
		g_NESmapper.ROM_mask >>= 1;
	}
	for(probe = 0x8000; probe; probe >>= 1)
	{
		if((g_NESmapper.num_1k_VROM_banks-1) & probe) break;
		g_NESmapper.VROM_mask >>= 1;
	}

	//  LOG(HEX(g_NESmapper.ROM_mask,2) << " " << HEX(g_NESmapper.VROM_mask,2) << endl);

	// ポインタをベースに戻す
	g_NESmapper.Reset = NES_mapper_base_Reset;

	g_NESmapper.MemoryReadLow = NES_mapper_base_MemoryReadLow;
	g_NESmapper.WriteHighRegs = NES_mapper_base_WriteHighRegs;
	g_NESmapper.MemoryWrite = NES_mapper_base_MemoryWrite;
	g_NESmapper.MemoryWriteLow = NES_mapper_base_MemoryWriteLow;
	g_NESmapper.MemoryWriteSaveRAM = NES_mapper_base_MemoryWriteSaveRAM;

	g_NESmapper.MemoryReadSaveRAM = NES_mapper_base_MemoryReadSaveRAM;

	g_NESmapper.HSync = NES_mapper_base_HSync;
	g_NESmapper.VSync = NES_mapper_base_VSync;

	// for mmc2 & mmc5 & Oekakidds
	g_NESmapper.PPU_Latch_FDFE = NES_mapper_base_PPU_Latch_FDFE;
	g_NESmapper.PPU_Latch_RenderScreen = NES_mapper_base_PPU_Latch_RenderScreen;
	g_NESmapper.PPU_Latch_Address = NES_mapper_base_PPU_Latch_Address;

	// for disk system (#20)
	g_NESmapper.GetDiskSideNum = NES_mapper_base_GetDiskSideNum;
	g_NESmapper.GetDiskSide = NES_mapper_base_GetDiskSide;
	g_NESmapper.SetDiskSide = NES_mapper_base_SetDiskSide;
	g_NESmapper.GetDiskDatap = NES_mapper_base_GetDiskDatap;
	g_NESmapper.DiskAccessed = NES_mapper_base_DiskAccessed;
	// for Datach Barcode Battler
	g_NESmapper.SetBarcodeValue = NES_mapper_base_SetBarcodeValue;

	g_NESmapper.set_CPU_banks4 = NES_mapper_base_set_CPU_banks4;
	g_NESmapper.set_CPU_bank4 = NES_mapper_base_set_CPU_bank4;
	g_NESmapper.set_CPU_bank5 = NES_mapper_base_set_CPU_bank5;
	g_NESmapper.set_CPU_bank6 = NES_mapper_base_set_CPU_bank6;
	g_NESmapper.set_CPU_bank7 = NES_mapper_base_set_CPU_bank7;
	// for UNIF
	g_NESmapper.set_CPU_bank_unif = NES_mapper_base_set_CPU_bank_unif;
	g_NESmapper.set_VRAM_bank_unif = NES_mapper_base_set_VRAM_bank_unif;
	// for mapper 40
	g_NESmapper.set_CPU_banks5 = NES_mapper_base_set_CPU_banks5;
	g_NESmapper.set_CPU_bank3 = NES_mapper_base_set_CPU_bank3;
	g_NESmapper.set_PPU_banks8 = NES_mapper_base_set_PPU_banks8;
	g_NESmapper.set_PPU_bank0 = NES_mapper_base_set_PPU_bank0;
	g_NESmapper.set_PPU_bank1 = NES_mapper_base_set_PPU_bank1;
	g_NESmapper.set_PPU_bank2 = NES_mapper_base_set_PPU_bank2;
	g_NESmapper.set_PPU_bank3 = NES_mapper_base_set_PPU_bank3;
	g_NESmapper.set_PPU_bank4 = NES_mapper_base_set_PPU_bank4;
	g_NESmapper.set_PPU_bank5 = NES_mapper_base_set_PPU_bank5;
	g_NESmapper.set_PPU_bank6 = NES_mapper_base_set_PPU_bank6;
	g_NESmapper.set_PPU_bank7 = NES_mapper_base_set_PPU_bank7;
	// for mapper 19,68,90
	g_NESmapper.set_PPU_bank8 = NES_mapper_base_set_PPU_bank8;
	g_NESmapper.set_PPU_bank9 = NES_mapper_base_set_PPU_bank9;
	g_NESmapper.set_PPU_bank10 = NES_mapper_base_set_PPU_bank10;
	g_NESmapper.set_PPU_bank11 = NES_mapper_base_set_PPU_bank11;

	// for mapper 1,4,5,6,13,19,80,85,96,119
	g_NESmapper.set_VRAM_bank = NES_mapper_base_set_VRAM_bank;

	g_NESmapper.set_mirroring = NES_mapper_base_set_mirroring;
	g_NESmapper.set_mirroring2 = NES_mapper_base_set_mirroring2;

	g_NESmapper.set_genie = NES_mapper_base_set_genie;
}


/////////////////////////////////////////////////////////////////////
// mapper factory
NES_mapper* GetMapper(void)
{
	// 初期化
	NES_mapper_Init();

	if(NES_ROM_get_unifmapper_num()){
		switch(NES_ROM_get_unifmapper_num()){
			case 1:
				 NES_UNIFmapper1_Init(); return &g_NESmapper;
			case 2:
				 NES_UNIFmapper2_Init(); return &g_NESmapper;
			case 3:
				 NES_UNIFmapper3_Init(); return &g_NESmapper;
			case 4:
				 NES_UNIFmapper4_Init(); return &g_NESmapper;
			default:
				return NULL;
		}
	}

	switch(NES_ROM_get_mapper_num())
	{
	case 0:
		NES_mapper0_Init(); return &g_NESmapper;
	case 1:
		NES_mapper1_Init(); return &g_NESmapper;
	case 2:
		NES_mapper2_Init(); return &g_NESmapper;
	case 3:
		NES_mapper3_Init(); return &g_NESmapper;
	case 4:
		NES_mapper4_Init(); return &g_NESmapper;
	case 5:
		NES_mapper5_Init(); return &g_NESmapper;
	case 6:
		NES_mapper6_Init(); return &g_NESmapper;
	case 7:
		NES_mapper7_Init(); return &g_NESmapper;
	case 8:
		NES_mapper8_Init(); return &g_NESmapper;
	case 9:
		NES_mapper9_Init(); return &g_NESmapper;
	case 10:
		NES_mapper10_Init(); return &g_NESmapper;
	case 11:
		NES_mapper11_Init(); return &g_NESmapper;
	case 13:
		NES_mapper13_Init(); return &g_NESmapper;
	case 15:
		NES_mapper15_Init(); return &g_NESmapper;
	case 16:
		NES_mapper16_Init(); return &g_NESmapper;
	case 17:
		NES_mapper17_Init(); return &g_NESmapper;
	case 18:
		NES_mapper18_Init(); return &g_NESmapper;
	case 19:
		NES_mapper19_Init(); return &g_NESmapper;
	case 20:
		NES_mapper20_Init(); return &g_NESmapper;
	case 21:
		NES_mapper21_Init(); return &g_NESmapper;
	case 22:
		NES_mapper22_Init(); return &g_NESmapper;
	case 23:
		NES_mapper23_Init(); return &g_NESmapper;
	case 24:
		NES_mapper24_Init(); return &g_NESmapper;
	case 25:
		NES_mapper25_Init(); return &g_NESmapper;
	case 26:
		NES_mapper26_Init(); return &g_NESmapper;
	case 32:
		NES_mapper32_Init(); return &g_NESmapper;
	case 33:
		NES_mapper33_Init(); return &g_NESmapper;
	case 34:
		NES_mapper34_Init(); return &g_NESmapper;
	case 40:
		NES_mapper40_Init(); return &g_NESmapper;
	case 41:
		NES_mapper41_Init(); return &g_NESmapper;
	case 42:
		NES_mapper42_Init(); return &g_NESmapper;
	case 43:
		NES_mapper43_Init(); return &g_NESmapper;
	case 44:
		NES_mapper44_Init(); return &g_NESmapper;
	case 45:
		NES_mapper45_Init(); return &g_NESmapper;
	case 46:
		NES_mapper46_Init(); return &g_NESmapper;
	case 47:
		NES_mapper47_Init(); return &g_NESmapper;
	case 48:
		NES_mapper48_Init(); return &g_NESmapper;
	case 49:
		NES_mapper49_Init(); return &g_NESmapper;
	case 50:
		NES_mapper50_Init(); return &g_NESmapper;
	case 51:
		NES_mapper51_Init(); return &g_NESmapper;
	case 52:
		NES_mapper52_Init(); return &g_NESmapper;
	case 57:
		NES_mapper57_Init(); return &g_NESmapper;
	case 58:
		NES_mapper58_Init(); return &g_NESmapper;
	case 60:
		NES_mapper60_Init(); return &g_NESmapper;
	case 62:
		NES_mapper62_Init(); return &g_NESmapper;
	case 64:
		NES_mapper64_Init(); return &g_NESmapper;
	case 65:
		NES_mapper65_Init(); return &g_NESmapper;
	case 66:
		NES_mapper66_Init(); return &g_NESmapper;
	case 67:
		NES_mapper67_Init(); return &g_NESmapper;
	case 68:
		NES_mapper68_Init(); return &g_NESmapper;
	case 69:
		NES_mapper69_Init(); return &g_NESmapper;
	case 70:
		NES_mapper70_Init(); return &g_NESmapper;
	case 71:
		NES_mapper71_Init(); return &g_NESmapper;
	case 72:
		NES_mapper72_Init(); return &g_NESmapper;
	case 73:
		NES_mapper73_Init(); return &g_NESmapper;
	case 74:
		NES_mapper74_Init(); return &g_NESmapper;
	case 75:
		NES_mapper75_Init(); return &g_NESmapper;
	case 76:
		NES_mapper76_Init(); return &g_NESmapper;
	case 77:
		NES_mapper77_Init(); return &g_NESmapper;
	case 78:
		NES_mapper78_Init(); return &g_NESmapper;
	case 79:
		NES_mapper79_Init(); return &g_NESmapper;
	case 80:
		NES_mapper80_Init(); return &g_NESmapper;
	case 82:
		NES_mapper82_Init(); return &g_NESmapper;
	case 83:
		NES_mapper83_Init(); return &g_NESmapper;
	case 85:
		NES_mapper85_Init(); return &g_NESmapper;
	case 86:
		NES_mapper86_Init(); return &g_NESmapper;
	case 87:
		NES_mapper87_Init(); return &g_NESmapper;
	case 88:
		NES_mapper88_Init(); return &g_NESmapper;
	case 89:
		NES_mapper89_Init(); return &g_NESmapper;
	case 90:
		NES_mapper90_Init(); return &g_NESmapper;
	case 91:
		NES_mapper91_Init(); return &g_NESmapper;
	case 92:
		NES_mapper92_Init(); return &g_NESmapper;
	case 93:
		NES_mapper93_Init(); return &g_NESmapper;
	case 94:
		NES_mapper94_Init(); return &g_NESmapper;
	case 95:
		NES_mapper95_Init(); return &g_NESmapper;
	case 96:
		NES_mapper96_Init(); return &g_NESmapper;
	case 97:
		NES_mapper97_Init(); return &g_NESmapper;
	case 99:
		NES_mapper99_Init(); return &g_NESmapper;
	case 100:
		NES_mapper100_Init(); return &g_NESmapper;
	case 101:
		NES_mapper101_Init(); return &g_NESmapper;
	case 105:
		NES_mapper105_Init(); return &g_NESmapper;
	case 112:
		NES_mapper112_Init(); return &g_NESmapper;
	case 113:
		NES_mapper113_Init(); return &g_NESmapper;
	case 114:
		NES_mapper114_Init(); return &g_NESmapper;
	case 115:
		NES_mapper115_Init(); return &g_NESmapper;
	case 117:
		NES_mapper117_Init(); return &g_NESmapper;
	case 118:
		NES_mapper118_Init(); return &g_NESmapper;
	case 119:
		NES_mapper119_Init(); return &g_NESmapper;
	case 122:
		NES_mapper122_Init(); return &g_NESmapper;
	case 140:
		NES_mapper140_Init(); return &g_NESmapper;
	case 151:
		NES_mapper151_Init(); return &g_NESmapper;
	case 160:
		NES_mapper160_Init(); return &g_NESmapper;
	case 180:
		NES_mapper180_Init(); return &g_NESmapper;
	case 181:
		NES_mapper181_Init(); return &g_NESmapper;
	case 182:
		NES_mapper182_Init(); return &g_NESmapper;
	case 183:
		NES_mapper183_Init(); return &g_NESmapper;
	case 184:
//		NES_mapper184_Init(); return &g_NESmapper;
		NES_mapper122_Init(); return &g_NESmapper;
	case 185:
		NES_mapper185_Init(); return &g_NESmapper;
	case 187:
		NES_mapper187_Init(); return &g_NESmapper;
	case 188:
		NES_mapper188_Init(); return &g_NESmapper;
	case 189:
		NES_mapper189_Init(); return &g_NESmapper;
	case 222:
		NES_mapper222_Init(); return &g_NESmapper;
	case 225:
		NES_mapper225_Init(); return &g_NESmapper;
	case 226:
		NES_mapper226_Init(); return &g_NESmapper;
	case 227:
		NES_mapper227_Init(); return &g_NESmapper;
	case 228:
		NES_mapper228_Init(); return &g_NESmapper;
	case 229:
		NES_mapper229_Init(); return &g_NESmapper;
	case 230:
		NES_mapper230_Init(); return &g_NESmapper;
	case 231:
		NES_mapper231_Init(); return &g_NESmapper;
	case 232:
		NES_mapper232_Init(); return &g_NESmapper;
	case 233:
		NES_mapper233_Init(); return &g_NESmapper;
	case 234:
		NES_mapper234_Init(); return &g_NESmapper;
	case 235:
		NES_mapper235_Init(); return &g_NESmapper;
	case 236:
		NES_mapper236_Init(); return &g_NESmapper;
	case 237:
		NES_mapper237_Init(); return &g_NESmapper;
	case 240:
		NES_mapper240_Init(); return &g_NESmapper;
	case 241:
		NES_mapper241_Init(); return &g_NESmapper;
	case 242:
		NES_mapper242_Init(); return &g_NESmapper;
	case 243:
		NES_mapper243_Init(); return &g_NESmapper;
	case 244:
		NES_mapper244_Init(); return &g_NESmapper;
	case 245:
		NES_mapper245_Init(); return &g_NESmapper;
	case 246:
		NES_mapper246_Init(); return &g_NESmapper;
	case 248:
		NES_mapper248_Init(); return &g_NESmapper;
	case 249:
		NES_mapper249_Init(); return &g_NESmapper;
	case 250:
		NES_mapper250_Init(); return &g_NESmapper;
	case 251:
		NES_mapper251_Init(); return &g_NESmapper;
	case 252:
		NES_mapper252_Init(); return &g_NESmapper;
	case 254:
		NES_mapper254_Init(); return &g_NESmapper;
	case 255:
		NES_mapper255_Init(); return &g_NESmapper;
	case 12:
		NES_mapperNSF_Init(); return &g_NESmapper;

	default:
		return NULL;  // mapper not supported
	}
}
/////////////////////////////////////////////////////////////////////
