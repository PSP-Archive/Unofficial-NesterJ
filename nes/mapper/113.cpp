
/////////////////////////////////////////////////////////////////////
// Mapper 113
void NES_mapper113_Init()
{
	g_NESmapper.Reset = NES_mapper113_Reset;
	g_NESmapper.MemoryWriteLow = NES_mapper113_MemoryWriteLow;
	g_NESmapper.MemoryWrite = NES_mapper113_MemoryWrite;
}

void NES_mapper113_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);

	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}
}

void NES_mapper113_MemoryWriteLow(uint32 addr, uint8 data)
{
	switch(addr)
	{
	case 0x4100:
	case 0x4111:
	case 0x4120:
	case 0x4900:
		{
			uint8 prg_bank, chr_bank;

			prg_bank = data >> 3;
			if (g_NESmapper.num_8k_ROM_banks <= 8 && g_NESmapper.num_1k_VROM_banks == 8*16)
			{
				chr_bank = ((data >> 3) & 0x08) + (data & 0x07);
			}
			else
			{
				chr_bank = data & 0x07;
			}
			//uint8 prg_bank = (data & 0x03) | ((data & 0x80) >> 5);
			//uint8 chr_bank = (data & 0x70) >> 4;

			g_NESmapper.set_CPU_bank4(prg_bank*4+0);
			g_NESmapper.set_CPU_bank5(prg_bank*4+1);
			g_NESmapper.set_CPU_bank6(prg_bank*4+2);
			g_NESmapper.set_CPU_bank7(prg_bank*4+3);

			g_NESmapper.set_PPU_bank0(chr_bank*8+0);
			g_NESmapper.set_PPU_bank1(chr_bank*8+1);
			g_NESmapper.set_PPU_bank2(chr_bank*8+2);
			g_NESmapper.set_PPU_bank3(chr_bank*8+3);
			g_NESmapper.set_PPU_bank4(chr_bank*8+4);
			g_NESmapper.set_PPU_bank5(chr_bank*8+5);
			g_NESmapper.set_PPU_bank6(chr_bank*8+6);
			g_NESmapper.set_PPU_bank7(chr_bank*8+7);
		}
		break;
	}
}

void NES_mapper113_MemoryWrite(uint32 addr, uint8 data)
{
	switch(addr)
	{
	case 0x8008:
	case 0x8009:
		{
			uint8 prg_bank, chr_bank;

			prg_bank = data >> 3;
			if (g_NESmapper.num_8k_ROM_banks <= 8 && g_NESmapper.num_1k_VROM_banks == 8*16)
			{
				chr_bank = ((data >> 3) & 0x08) + (data & 0x07);
			}
			else
			{
				chr_bank = data & 0x07;
			}
			//uint8 prg_bank = (data & 0x03) | ((data & 0x80) >> 5);
			//uint8 chr_bank = (data & 0x70) >> 4;

			g_NESmapper.set_CPU_bank4(prg_bank*4+0);
			g_NESmapper.set_CPU_bank5(prg_bank*4+1);
			g_NESmapper.set_CPU_bank6(prg_bank*4+2);
			g_NESmapper.set_CPU_bank7(prg_bank*4+3);

			g_NESmapper.set_PPU_bank0(chr_bank*8+0);
			g_NESmapper.set_PPU_bank1(chr_bank*8+1);
			g_NESmapper.set_PPU_bank2(chr_bank*8+2);
			g_NESmapper.set_PPU_bank3(chr_bank*8+3);
			g_NESmapper.set_PPU_bank4(chr_bank*8+4);
			g_NESmapper.set_PPU_bank5(chr_bank*8+5);
			g_NESmapper.set_PPU_bank6(chr_bank*8+6);
			g_NESmapper.set_PPU_bank7(chr_bank*8+7);
		}
		break;
	}
}
/////////////////////////////////////////////////////////////////////

