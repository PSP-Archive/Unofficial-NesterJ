#ifdef _NES_MAPPER_CPP_

/////////////////////////////////////////////////////////////////////
// Mapper 252
void NES_mapper252_Init()
{
	g_NESmapper.Reset = NES_mapper252_Reset;
	g_NESmapper.MemoryWrite = NES_mapper252_MemoryWrite;
}

void NES_mapper252_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0, 1, g_NESmapper.num_8k_ROM_banks-2, g_NESmapper.num_8k_ROM_banks-1);
	g_NESmapper.Mapper251.regs[0]=0;
	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}
}


void NES_mapper252_MemoryWrite(uint32 addr, uint8 data)
{
	//	char f=0;
	switch(addr&0xff0f){
	case 0x8000:
		g_NESmapper.set_CPU_bank4(data);
		break;
	case 0x8400:
		g_NESmapper.set_CPU_bank5(data);
		break;
	case 0x9000:
		//			set_CPU_bank6(data);
		break;
	case 0xA400:
		//			set_CPU_bank7(data);
		break;
		/*		case 0xB000:
					set_PPU_bank0(data);
					break;
				case 0xB004:
					set_PPU_bank1(data);
					break;
				case 0xB008:
					set_PPU_bank2(data);
					break;
				case 0xB00C:
					set_PPU_bank3(data);
					break;
				case 0xC000:
					set_PPU_bank0(data);
					break;
				case 0xC004:
					set_PPU_bank1(data);
					break;
				case 0xC008:
					set_PPU_bank2(data);
					break;
				case 0xC00C:
					set_PPU_bank3(data);
					break;
*/		case 0xD000:
		g_NESmapper.set_PPU_bank0(data);
		break;
	case 0xD004:
		g_NESmapper.set_PPU_bank1(data);
		break;
	case 0xD008:
		g_NESmapper.set_PPU_bank2(data);
		break;
	case 0xD00C:
		g_NESmapper.set_PPU_bank3(data);
		break;
	case 0xE000:
		g_NESmapper.set_PPU_bank4(data);
		break;
	case 0xE004:
		g_NESmapper.set_PPU_bank5(data);
		break;
	case 0xE008:
		g_NESmapper.set_PPU_bank6(data);
		break;
	case 0xE00C:
		g_NESmapper.set_PPU_bank7(data);
		break;
		/*		case 0xE807:
					set_CPU_bank6(data);
					break;
				case 0xF000:
					set_PPU_bank7(data);
					break;
				case 0xF004:
					set_PPU_bank7(data);
					break;
		*/	}
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(g_NESmapper.Mapper251.regs[0],2) << endl);

}

#if 0
void NES_mapper252_HSync(uint32 scanline)
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
