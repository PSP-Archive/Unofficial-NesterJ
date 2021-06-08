#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 115
void NES_mapper115_Init()
{
	g_NESmapper.Reset = NES_mapper115_Reset;
	g_NESmapper.MemoryWrite = NES_mapper115_MemoryWrite;
	g_NESmapper.HSync = NES_mapper115_HSync;
}

void NES_mapper115_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0, 1, g_NESmapper.num_8k_ROM_banks-2, g_NESmapper.num_8k_ROM_banks-1);
	g_NESmapper.Mapper115.regs[0]=0;
	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}
	g_NESmapper.Mapper115.irq_latch=g_NESmapper.Mapper115.irq_counter=g_NESmapper.Mapper115.irq_enabled=0;
}


void NES_mapper115_MemoryWrite(uint32 addr, uint8 data)
{
	char f=0;
	switch(addr&0xf001){
	case 0x8000:
		g_NESmapper.Mapper115.regs[0]=data;
		f=1;
		break;
	case 0x8001:
		switch(g_NESmapper.Mapper115.regs[0]&7){
		case 0:
			g_NESmapper.set_PPU_bank0(data&0x1f);
			g_NESmapper.set_PPU_bank1(data&0x1f+1);
			break;
		case 1:
			g_NESmapper.set_PPU_bank2(data&0x1f);
			g_NESmapper.set_PPU_bank3(data&0x1f+1);
			break;
		case 2:
			g_NESmapper.set_PPU_bank4(data&0x1f);
			break;
		case 3:
			g_NESmapper.set_PPU_bank5(data&0x1f);
			break;
		case 4:
			g_NESmapper.set_PPU_bank6(data&0x1f);
			break;
		case 5:
			g_NESmapper.set_PPU_bank7(data&0x1f);
			break;
		case 6:
			g_NESmapper.set_CPU_bank4(data&0x0f);
			break;
		case 7:
			g_NESmapper.set_CPU_bank5(data&0x0f);
			break;
		}
		f=1;
		break;
	case 0xA000:
		//			g_NESmapper.Mapper115.irq_enabled=data;
		break;
	case 0xC000:
		data&=0x1f;
		//			g_NESmapper.Mapper115.irq_counter=data;
		//			set_PPU_bank1(data);
		break;
	case 0xE000:
		//			set_CPU_bank4(data&0x0f);
		g_NESmapper.set_PPU_banks8(data, data+1, data+2, data+3, data+4, data+5, data+6, data+7);
		break;
	}
	if(f==0){
		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(g_NESmapper.Mapper115.regs[0],2) << endl);
	}

}


void NES_mapper115_HSync(uint32 scanline)
{
	if(g_NESmapper.Mapper115.irq_enabled)
	{
		if(scanline<241){
			g_NESmapper.Mapper115.irq_counter--;
			if(g_NESmapper.Mapper115.irq_counter<=0){
				NES6502_DoIRQ();
				g_NESmapper.Mapper115.irq_enabled = 0;
				g_NESmapper.Mapper115.irq_counter = g_NESmapper.Mapper115.irq_latch;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////

#endif
