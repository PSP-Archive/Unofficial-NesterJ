#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 60
void NES_mapper60_Init()
{
	g_NESmapper.Reset = NES_mapper60_Reset;
	g_NESmapper.MemoryWrite = NES_mapper60_MemoryWrite;
}

void NES_mapper60_Reset()
{
	g_NESmapper.set_CPU_banks4(0,1,2,3);
	g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
}

void NES_mapper60_MemoryWrite(uint32 addr, uint8 data)
{
	if(addr & 0x80)
	{
		g_NESmapper.set_CPU_bank4(2 * ((addr & 0x70) >> 4) + 0);
		g_NESmapper.set_CPU_bank5(2 * ((addr & 0x70) >> 4) + 1);
		g_NESmapper.set_CPU_bank6(2 * ((addr & 0x70) >> 4) + 0);
		g_NESmapper.set_CPU_bank7(2 * ((addr & 0x70) >> 4) + 1);
	}
	else
	{
		g_NESmapper.set_CPU_bank4(4 * ((addr & 0x70) >> 5) + 0);
		g_NESmapper.set_CPU_bank5(4 * ((addr & 0x70) >> 5) + 1);
		g_NESmapper.set_CPU_bank6(4 * ((addr & 0x70) >> 5) + 2);
		g_NESmapper.set_CPU_bank7(4 * ((addr & 0x70) >> 5) + 3);
	}

	g_NESmapper.set_PPU_bank0(8 * (addr & 0x07) + 0);
	g_NESmapper.set_PPU_bank1(8 * (addr & 0x07) + 1);
	g_NESmapper.set_PPU_bank2(8 * (addr & 0x07) + 2);
	g_NESmapper.set_PPU_bank3(8 * (addr & 0x07) + 3);
	g_NESmapper.set_PPU_bank4(8 * (addr & 0x07) + 4);
	g_NESmapper.set_PPU_bank5(8 * (addr & 0x07) + 5);
	g_NESmapper.set_PPU_bank6(8 * (addr & 0x07) + 6);
	g_NESmapper.set_PPU_bank7(8 * (addr & 0x07) + 7);

	if(data & 0x08)
	{
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_HORIZ);
	}
	else
	{
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
	}
}
/////////////////////////////////////////////////////////////////////

#endif
