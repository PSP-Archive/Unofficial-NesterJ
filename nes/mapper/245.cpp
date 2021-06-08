#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 245
void NES_mapper245_Init()
{
	g_NESmapper.Reset = NES_mapper245_Reset;
	g_NESmapper.MemoryWrite = NES_mapper245_MemoryWrite;
	g_NESmapper.HSync = NES_mapper245_HSync;
}

void NES_mapper245_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);

	g_NESmapper.Mapper245.regs[0] = 0;
	g_NESmapper.Mapper245.irq_counter = g_NESmapper.Mapper245.irq_latch = g_NESmapper.Mapper245.irq_enabled = 0;
}

void NES_mapper245_MemoryWrite(uint32 addr, uint8 data)
{
	switch(addr&0xF007)
	{
	case 0x8000:
		{
			g_NESmapper.Mapper245.regs[0] = data;
		}
		break;

	case 0x8001:
		{
			switch(g_NESmapper.Mapper245.regs[0] & 7)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				break;

			case 6:
				{
					g_NESmapper.set_CPU_bank4(data);
				}
				break;

			case 7:
				{
					g_NESmapper.set_CPU_bank5(data);
				}
				break;
			}
		}
		break;
	case 0xA000:
		break;
	case 0xA001:
		g_NESmapper.Mapper245.irq_enabled=data;
		break;
	case 0xE000:
		g_NESmapper.Mapper245.irq_counter= (g_NESmapper.Mapper245.irq_counter&0xff00)|data;
		//			set_CPU_bank4(data);
		break;
	case 0xE001:
		g_NESmapper.Mapper245.irq_counter=(g_NESmapper.Mapper245.irq_counter&0xff)|(data<<8);
		//			set_CPU_bank5(data);
		break;
	}
//	if(addr>=0x8000 ){
//		LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(g_NESmapper.Mapper245.regs[0],2) << endl);
//	}
}

void NES_mapper245_HSync(uint32 scanline)
{
#if 1
	if(g_NESmapper.Mapper245.irq_enabled)
	{
		if(scanline<241 /*&& (NES_PPU_spr_enabled() || NES_PPU_bg_enabled())*/){
			g_NESmapper.Mapper245.irq_counter-=114;
			if(g_NESmapper.Mapper245.irq_counter<=0){
				NES6502_DoIRQ();
				g_NESmapper.Mapper245.irq_enabled = 0;
				//			  g_NESmapper.Mapper245.irq_counter = g_NESmapper.Mapper245.irq_latch;
			}
		}
	}
#endif
}
/////////////////////////////////////////////////////////////////////

#endif
