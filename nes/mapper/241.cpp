#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 241
void NES_mapper241_Init()
{
	g_NESmapper.Reset = NES_mapper241_Reset;
	g_NESmapper.MemoryWrite = NES_mapper241_MemoryWrite;
}

void NES_mapper241_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);
	g_NESmapper.Mapper241.regs[0]=0;
	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}
}


void NES_mapper241_MemoryWrite(uint32 addr, uint8 data)
{
//	LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(g_NESmapper.Mapper241.regs[0],2) << endl);
	//	char f=0;
	switch(addr/*&0xF801*/){
	case 0x8000:
		{
			uint32 n = data<<2;
			g_NESmapper.set_CPU_banks4(n,n+1,n+2,n+3);
		}
		break;
	case 0x8001:
		break;
	case 0xF000:
//		set_PPU_bank7(data);
		break;
	}

}

#if 0
void NES_mapper241_HSync(uint32 scanline)
{
	if(irq_enabled)
	{
		if(scanline<241){
			irq_counter--;
			if(irq_counter<=0){
				NES6502_DoIRQ();
				irq_enabled = 0;
				irq_counter = irq_latch;
			}
		}
	}
}
#endif

/////////////////////////////////////////////////////////////////////

#endif
