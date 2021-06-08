
/////////////////////////////////////////////////////////////////////
// Mapper 187
void NES_mapper187_Init()
{
	g_NESmapper.Reset = NES_mapper187_Reset;
	g_NESmapper.MemoryWrite = NES_mapper187_MemoryWrite;
	g_NESmapper.HSync = NES_mapper187_HSync;
}

void NES_mapper187_Reset()
{
	// clear registers FIRST!!!
	int i;
	for(i = 0; i < 8; i++){ g_NESmapper.Mapper187.regs[i] = 0x00; g_NESmapper.Mapper187.bregs[i] =0;}

	// set CPU bank pointers
	g_NESmapper.Mapper187.prg0 = 0;
	g_NESmapper.Mapper187.prg1 = 1;

	// set VROM banks
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.Mapper187.chr01 = 0;
		g_NESmapper.Mapper187.chr23 = 2;
		g_NESmapper.Mapper187.chr4  = 4;
		g_NESmapper.Mapper187.chr5  = 5;
		g_NESmapper.Mapper187.chr6  = 6;
		g_NESmapper.Mapper187.chr7  = 7;
		NES_mapper187_MMC3_set_PPU_banks();
	}
	else
	{
		g_NESmapper.Mapper187.chr01 = g_NESmapper.Mapper187.chr23 = g_NESmapper.Mapper187.chr4 = g_NESmapper.Mapper187.chr5 = g_NESmapper.Mapper187.chr6 = g_NESmapper.Mapper187.chr7 = 0;
	}

	g_NESmapper.Mapper187.ex_prg0 = 0;
	g_NESmapper.Mapper187.ex_prg1 = 1;
	g_NESmapper.Mapper187.ex_bank_enabled = g_NESmapper.Mapper187.ex_bank_mode = 0;
	NES_mapper187_MMC3_set_CPU_banks();
//	g_NESmapper.set_CPU_banks4(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);

	g_NESmapper.Mapper187.irq_enabled = 0;
	g_NESmapper.Mapper187.irq_counter = 0;
	g_NESmapper.Mapper187.irq_latch = 0;
}

void NES_mapper187_MemoryWriteLow(uint32 addr, uint8 data)
{
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
	if(addr == 0x5000)
	{
//		NES_mapper187_MMC3_set_CPU_banks();

		if(data & 0x80)
		{
			if(data & 0x20)
			{
				g_NESmapper.set_CPU_bank4((data & 0x1E)*2+0);
				g_NESmapper.set_CPU_bank5((data & 0x1E)*2+1);
				g_NESmapper.set_CPU_bank6((data & 0x1E)*2+2);
				g_NESmapper.set_CPU_bank7((data & 0x1E)*2+3);
			}
			else
			{
				g_NESmapper.set_CPU_bank5((data & 0x1F)*2+0);
				g_NESmapper.set_CPU_bank6((data & 0x1F)*2+1);
			}
		}
		else
		{
			g_NESmapper.set_CPU_banks4(g_NESmapper.Mapper187.ex_prg0,g_NESmapper.Mapper187.ex_prg1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
		}
		g_NESmapper.Mapper187.ex_bank_mode = data;
	}
}

void NES_mapper187_MemoryWrite(uint32 addr, uint8 data)
{
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
	switch(addr/*&0xF003*/)
	{
#if 1
	case 0x8000:
		{
			g_NESmapper.Mapper187.ex_bank_enabled = 0;
			g_NESmapper.Mapper187.regs[0] = data;
			NES_mapper187_MMC3_set_PPU_banks();
			//MMC3_set_CPU_banks();
		}
		break;

	case 0x8001:
		{
			uint32 bank_num;
			g_NESmapper.Mapper187.regs[1] = data;
			bank_num = g_NESmapper.Mapper187.regs[1];

			if(g_NESmapper.Mapper187.ex_bank_enabled)
			{
				if(g_NESmapper.Mapper187.regs[0] == 0x2A)
				{
					g_NESmapper.set_CPU_bank5(0x0F);
				}
				if(g_NESmapper.Mapper187.regs[0] == 0x28)
				{
					g_NESmapper.set_CPU_bank6(0x17);
				}
			}
			else
			{
				switch(g_NESmapper.Mapper187.regs[0] & 0x07)
				{
				case 0x00:
					{
						if(g_NESmapper.num_1k_VROM_banks)
						{
							bank_num &= 0xfe;
							g_NESmapper.Mapper187.chr01 = bank_num;
							NES_mapper187_MMC3_set_PPU_banks();
						}
					}
					break;

				case 0x01:
					{
						if(g_NESmapper.num_1k_VROM_banks)
						{
							bank_num &= 0xfe;
							g_NESmapper.Mapper187.chr23 = bank_num;
							NES_mapper187_MMC3_set_PPU_banks();
						}
					}
					break;

				case 0x02:
					{
						if(g_NESmapper.num_1k_VROM_banks)
						{
							g_NESmapper.Mapper187.chr4 = bank_num;
							NES_mapper187_MMC3_set_PPU_banks();
						}
					}
					break;

				case 0x03:
					{
						if(g_NESmapper.num_1k_VROM_banks)
						{
							g_NESmapper.Mapper187.chr5 = bank_num;
							NES_mapper187_MMC3_set_PPU_banks();
						}
					}
					break;

				case 0x04:
					{
						if(g_NESmapper.num_1k_VROM_banks)
						{
							g_NESmapper.Mapper187.chr6 = bank_num;
							NES_mapper187_MMC3_set_PPU_banks();
						}
					}
					break;

				case 0x05:
					{
						if(g_NESmapper.num_1k_VROM_banks)
						{
							g_NESmapper.Mapper187.chr7 = bank_num;
							NES_mapper187_MMC3_set_PPU_banks();
						}
					}
					break;

				case 0x06:
					{
						if((g_NESmapper.Mapper187.ex_bank_mode & 0xA0) != 0xA0)
						{
							g_NESmapper.Mapper187.prg0 = bank_num;
							NES_mapper187_MMC3_set_CPU_banks();
						}
						g_NESmapper.Mapper187.ex_prg0 = data;
					}
					break;

				case 0x07:
					{
						if((g_NESmapper.Mapper187.ex_bank_mode & 0xA0) != 0xA0)
						{
							g_NESmapper.Mapper187.prg1 = bank_num;
							NES_mapper187_MMC3_set_CPU_banks();
						}
						g_NESmapper.Mapper187.ex_prg1 = data;
					}
					break;
				}
			}
//			if((g_NESmapper.Mapper187.regs[0] & 0x07) == 0x06) g_NESmapper.Mapper187.ex_prg0 = data;
//			if((g_NESmapper.Mapper187.regs[0] & 0x07) == 0x07) g_NESmapper.Mapper187.ex_prg1 = data;
		}
		break;

	case 0x8003:
		{
			g_NESmapper.Mapper187.ex_bank_enabled = 1;
			if(!(data & 0xF0))
			{
				g_NESmapper.set_CPU_bank6(g_NESmapper.num_8k_ROM_banks-2);
			}
		}
		break;
#else
	case 0x8000:
		g_NESmapper.Mapper187.ex_bank_enabled = 0;
		g_NESmapper.Mapper187.regs[0] = data;
		break;
	case 0x8001:
		bg_NESmapper.Mapper187.regs[g_NESmapper.Mapper187.regs[0]&7] = data;
		NES_mapper187_MMC3_set_CPU_banks();
		break;
	case 0x8003:
		g_NESmapper.Mapper187.ex_bank_enabled = 1;
		break;
#endif
	case 0xA000:
		{
			g_NESmapper.Mapper187.regs[2] = data;

			if(NES_ROM_get_mirroring() != NES_PPU_MIRROR_FOUR_SCREEN)
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
		}
		break;

	case 0xA001:
		{
			g_NESmapper.Mapper187.regs[3] = data;
		}
		break;

	case 0xC000:
		g_NESmapper.Mapper187.regs[4] = data;
		g_NESmapper.Mapper187.irq_counter = g_NESmapper.Mapper187.regs[4];
		break;

	case 0xC001:
		g_NESmapper.Mapper187.regs[5] = data;
		g_NESmapper.Mapper187.irq_latch = g_NESmapper.Mapper187.regs[5];
		break;

	case 0xE000:
	case 0xE002:
		g_NESmapper.Mapper187.regs[6] = data;
		g_NESmapper.Mapper187.irq_enabled = 0;
		break;

	case 0xE001:
	case 0xE003:
		g_NESmapper.Mapper187.regs[7] = data;
		g_NESmapper.Mapper187.irq_enabled = 1;
		break;
	}
}

void NES_mapper187_HSync(uint32 scanline)
{
	if(g_NESmapper.Mapper187.irq_enabled)
	{
		if((scanline >= 0) && (scanline <= 239))
		{
			if(NES_PPU_spr_enabled() || NES_PPU_bg_enabled())
			{
				if(!(g_NESmapper.Mapper187.irq_counter--))
				{
					g_NESmapper.Mapper187.irq_counter = g_NESmapper.Mapper187.irq_latch;
					NES6502_DoIRQ();
				}
			}
		}
	}
}

void NES_mapper187_MMC3_set_CPU_banks()
{
#if 1
	if(g_NESmapper.Mapper187.regs[0] & 0x40)
	{
		g_NESmapper.set_CPU_banks4(g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.Mapper187.prg1,g_NESmapper.Mapper187.prg0,g_NESmapper.num_8k_ROM_banks-1);
	}
	else
	{
		g_NESmapper.set_CPU_banks4(g_NESmapper.Mapper187.prg0,g_NESmapper.Mapper187.prg1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
	}
#else
	if(g_NESmapper.Mapper187.ex_bank_mode & 0x80){
		if(g_NESmapper.Mapper187.ex_bank_mode & 0x20){
			uint32 bn = (g_NESmapper.Mapper187.ex_bank_mode&0x1f)<<1;
			set_CPU_banks(bn,bn+1,bn+2,bn+3);
		}else{
			uint32 bn = (g_NESmapper.Mapper187.ex_bank_mode&0x1F)<<1;
			set_CPU_banks(g_NESmapper.Mapper187.bregs[6],g_NESmapper.Mapper187.bregs[7],bn,bn+1);
		}
	}
	else{
		set_CPU_banks(g_NESmapper.Mapper187.bregs[6],bg_NESmapper.Mapper187.regs[7],g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
	}

#endif
}

void NES_mapper187_MMC3_set_PPU_banks()
{
	if(g_NESmapper.num_1k_VROM_banks)
	{
		if(g_NESmapper.Mapper187.regs[0] & 0x80)
		{
			g_NESmapper.set_PPU_banks8(g_NESmapper.Mapper187.chr4,g_NESmapper.Mapper187.chr5,g_NESmapper.Mapper187.chr6,g_NESmapper.Mapper187.chr7,g_NESmapper.Mapper187.chr01,g_NESmapper.Mapper187.chr01+1,g_NESmapper.Mapper187.chr23,g_NESmapper.Mapper187.chr23+1);
		}
		else
		{
			g_NESmapper.set_PPU_banks8(g_NESmapper.Mapper187.chr01,g_NESmapper.Mapper187.chr01+1,g_NESmapper.Mapper187.chr23,g_NESmapper.Mapper187.chr23+1,g_NESmapper.Mapper187.chr4,g_NESmapper.Mapper187.chr5,g_NESmapper.Mapper187.chr6,g_NESmapper.Mapper187.chr7);
		}
	}
}
/////////////////////////////////////////////////////////////////////

