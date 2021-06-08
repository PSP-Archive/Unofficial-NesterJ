#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 74
void NES_mapper74_Init()
{
	g_NESmapper.Reset = NES_mapper74_Reset;
	g_NESmapper.MemoryWrite = NES_mapper74_MemoryWrite;
}

void NES_mapper74_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0, 1, g_NESmapper.num_8k_ROM_banks-2, g_NESmapper.num_8k_ROM_banks-1);
//	set_CPU_banks(0, 1, 2, 3);
	g_NESmapper.Mapper74.regs[0]=0;
	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}
}


void NES_mapper74_MemoryWrite(uint32 addr, uint8 data)
{
	//	char f=0;
	switch(addr&0xff0f){
	case 0x8000:
		g_NESmapper.set_CPU_bank4(data&0x3f);
		break;
	case 0x8001:
		g_NESmapper.set_CPU_bank5(data&0x3f);
		break;
	case 0xE000:
		g_NESmapper.set_PPU_bank1(data);
		break;
	}
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(g_NESmapper.Mapper74.regs[0],2) << endl);

}

#if 0
void NES_mapper74_HSync(uint32 scanline)
{
	if(g_NESmapper.Mapper74.irq_enabled)
	{
		if(scanline<241){
			g_NESmapper.Mapper74.irq_counter--;
			if(g_NESmapper.Mapper74.irq_counter<=0){
				NES6502_DoIRQ();
				g_NESmapper.Mapper74.irq_enabled = 0;
				g_NESmapper.Mapper74.irq_counter = g_NESmapper.Mapper74.irq_latch;
			}
		}
	}
}
#endif

/////////////////////////////////////////////////////////////////////

#endif
