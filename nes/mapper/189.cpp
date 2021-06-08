
/////////////////////////////////////////////////////////////////////
// Mapper 189
void NES_mapper189_Init()
{
	g_NESmapper.Reset = NES_mapper189_Reset;
	g_NESmapper.MemoryWriteLow = NES_mapper189_MemoryWriteLow;
	g_NESmapper.MemoryWrite = NES_mapper189_MemoryWrite;
	g_NESmapper.HSync = NES_mapper189_HSync;
}

void NES_mapper189_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);

	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}

	g_NESmapper.Mapper189.irq_counter = 0;
	g_NESmapper.Mapper189.irq_latch = 0;
	g_NESmapper.Mapper189.irq_enabled = 0;
}

void NES_mapper189_MemoryWriteLow(uint32 addr, uint8 data)
{
	if(addr >= 0x4100 && addr <= 0x41FF)
	{
		data = (data & 0x30) >> 4;
		g_NESmapper.set_CPU_banks4(data*4,data*4+1,data*4+2,data*4+3);
	}
}

void NES_mapper189_MemoryWrite(uint32 addr, uint8 data)
{
	switch(addr)
	{
	case 0x8000:
		{
			g_NESmapper.Mapper189.regs[0] = data;
		}
		break;

	case 0x8001:
		{
			switch(g_NESmapper.Mapper189.regs[0])
			{
			case 0x40:
				{
					g_NESmapper.set_PPU_bank0(data+0);
					g_NESmapper.set_PPU_bank1(data+1);
				}
				break;

			case 0x41:
				{
					g_NESmapper.set_PPU_bank2(data+0);
					g_NESmapper.set_PPU_bank3(data+1);
				}
				break;

			case 0x42:
				{
					g_NESmapper.set_PPU_bank4(data);
				}
				break;

			case 0x43:
				{
					g_NESmapper.set_PPU_bank5(data);
				}
				break;

			case 0x44:
				{
					g_NESmapper.set_PPU_bank6(data);
				}
				break;

			case 0x45:
				{
					g_NESmapper.set_PPU_bank7(data);
				}
				break;

			case 0x46:
				{
					g_NESmapper.set_CPU_bank6(data);
				}
				break;

			case 0x47:
				{
					g_NESmapper.set_CPU_bank5(data);
				}
				break;
			}
		}
		break;

	case 0xA000:
		{
			if(data & 0x01)
			{
				g_NESmapper.set_mirroring2(NES_PPU_MIRROR_HORIZ);
			}
			else
			{
				g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
			}
		}
		break;

	case 0xC000:
		{
			g_NESmapper.Mapper189.irq_counter = data;
		}
		break;

	case 0xC001:
		{
			g_NESmapper.Mapper189.irq_latch = data;
		}
		break;

	case 0xE000:
		{
			g_NESmapper.Mapper189.irq_enabled = 0;
		}
		break;

	case 0xE001:
		{
			g_NESmapper.Mapper189.irq_enabled = 1;
		}
		break;
	}
}

void NES_mapper189_HSync(uint32 scanline)
{
	if(g_NESmapper.Mapper189.irq_enabled)
	{
		if((scanline >= 0) && (scanline <= 239))
		{
			if(NES_PPU_spr_enabled() || NES_PPU_bg_enabled())
			{
				if(!(--g_NESmapper.Mapper189.irq_counter))
				{
					g_NESmapper.Mapper189.irq_counter = g_NESmapper.Mapper189.irq_latch;
					NES6502_DoIRQ();
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////

