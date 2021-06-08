
/////////////////////////////////////////////////////////////////////
// Mapper 4
// much of this is based on the DarcNES source. thanks, nyef :)
void NES_mapper4_Init()
{
	g_NESmapper.Reset = NES_mapper4_Reset;
	g_NESmapper.MemoryReadLow = NES_mapper4_MemoryReadLow;
	g_NESmapper.MemoryWrite = NES_mapper4_MemoryWrite;
	g_NESmapper.HSync = NES_mapper4_HSync;
}

void NES_mapper4_Reset()
{
	int i;
	g_NESmapper.Mapper4.patch = 0;
	if(NES_crc32() == 0xdebea5a6 || // Ninja Ryukenden 2 - Ankoku no Jashin Ken
	        NES_crc32() == 0xc5fea9f2) // Dai2Ji - Super Robot Taisen
	{
		g_NESmapper.Mapper4.patch = 1;
	}

	if(NES_crc32() == 0xd7a97b38 )   // Chou Jinrou Senki - Warwolf
	{
		g_NESmapper.Mapper4.patch = 2;
	}

	if(NES_crc32() == 0xeb2dba63 )   // VS TKO Boxing
	{
		g_NESmapper.Mapper4.patch = 3;
		g_NESmapper.Mapper4.vs_index = 0;
	}
	if(NES_crc32() == 0x135adf7c)   // VS Atari RBI Baseball
	{
		g_NESmapper.Mapper4.patch = 4;
		g_NESmapper.Mapper4.vs_index = 0;
	}
	/*
	  if(NES_crc32() == 0xb42feeb4 ){
	    g_NESmapper.Mapper4.patch = 5;
	    g_NES.frame_irq_disenabled = 1;
	  }
	*/


	// clear registers FIRST!!!
	for(i = 0; i < 8; i++) g_NESmapper.Mapper4.regs[i] = 0x00;

	// set CPU bank pointers
	g_NESmapper.Mapper4.prg0 = 0;
	g_NESmapper.Mapper4.prg1 = 1;
	NES_mapper4_MMC3_set_CPU_banks();

	// set VROM banks
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.Mapper4.chr01 = 0;
		g_NESmapper.Mapper4.chr23 = 2;
		g_NESmapper.Mapper4.chr4  = 4;
		g_NESmapper.Mapper4.chr5  = 5;
		g_NESmapper.Mapper4.chr6  = 6;
		g_NESmapper.Mapper4.chr7  = 7;
		NES_mapper4_MMC3_set_PPU_banks();
	}
	else
	{
		g_NESmapper.Mapper4.chr01 = g_NESmapper.Mapper4.chr23 = g_NESmapper.Mapper4.chr4 =
			g_NESmapper.Mapper4.chr5 = g_NESmapper.Mapper4.chr6 = g_NESmapper.Mapper4.chr7 = 0;
	}

	g_NESmapper.Mapper4.irq_enabled = 0;
	g_NESmapper.Mapper4.irq_counter = 0;
	g_NESmapper.Mapper4.irq_latch = 0;
}

uint8 NES_mapper4_MemoryReadLow(uint32 addr)
{
	if(g_NESmapper.Mapper4.patch == 3)
	{
		// VS TKO Boxing security
		if(addr == 0x5E00)
		{
			g_NESmapper.Mapper4.vs_index = 0;
			return 0x00;
		}
		else if(addr == 0x5E01)
		{
			uint8 security_data[32] =
			    {
			        0xff, 0xbf, 0xb7, 0x97, 0x97, 0x17, 0x57, 0x4f,
			        0x6f, 0x6b, 0xeb, 0xa9, 0xb1, 0x90, 0x94, 0x14,
			        0x56, 0x4e, 0x6f, 0x6b, 0xeb, 0xa9, 0xb1, 0x90,
			        0xd4, 0x5c, 0x3e, 0x26, 0x87, 0x83, 0x13, 0x00
			    };
			return security_data[(g_NESmapper.Mapper4.vs_index++) & 0x1F];
		}
	}
	else if(g_NESmapper.Mapper4.patch == 4)
	{
		// VS Atari RBI Baseball security
		if(addr == 0x5E00)
		{
			g_NESmapper.Mapper4.vs_index = 0;
			return 0xFF;
		}
		else if(addr == 0x5E01)
		{
			switch(g_NESmapper.Mapper4.vs_index++)
			{
			case 0x09:
				return 0x6F;
			default:
				return 0xB4;
			}
		}
	}
	return  (uint8)(addr >> 8);
}

void NES_mapper4_MemoryWrite(uint32 addr, uint8 data)
{
	switch(addr & 0xE001)
	{
	case 0x8000:
		{
			g_NESmapper.Mapper4.regs[0] = data;
			NES_mapper4_MMC3_set_PPU_banks();
			NES_mapper4_MMC3_set_CPU_banks();
		}
		break;

	case 0x8001:
		{
			uint32 bank_num;

			g_NESmapper.Mapper4.regs[1] = data;
			bank_num = g_NESmapper.Mapper4.regs[1];

			switch(g_NESmapper.Mapper4.regs[0] & 0x07)
			{
			case 0x00:
				{
					//if(num_1k_VROM_banks)
					{
						bank_num &= 0xfe;
						g_NESmapper.Mapper4.chr01 = bank_num;
						NES_mapper4_MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x01:
				{
					//if(num_1k_VROM_banks)
					{
						bank_num &= 0xfe;
						g_NESmapper.Mapper4.chr23 = bank_num;
						NES_mapper4_MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x02:
				{
					//if(num_1k_VROM_banks)
					{
						g_NESmapper.Mapper4.chr4 = bank_num;
						NES_mapper4_MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x03:
				{
					//if(num_1k_VROM_banks)
					{
						g_NESmapper.Mapper4.chr5 = bank_num;
						NES_mapper4_MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x04:
				{
					//if(num_1k_VROM_banks)
					{
						g_NESmapper.Mapper4.chr6 = bank_num;
						NES_mapper4_MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x05:
				{
					//if(num_1k_VROM_banks)
					{
						g_NESmapper.Mapper4.chr7 = bank_num;
						NES_mapper4_MMC3_set_PPU_banks();
					}
				}
				break;

			case 0x06:
				{
					g_NESmapper.Mapper4.prg0 = bank_num;
					NES_mapper4_MMC3_set_CPU_banks();
				}
				break;

			case 0x07:
				{
					g_NESmapper.Mapper4.prg1 = bank_num;
					NES_mapper4_MMC3_set_CPU_banks();
				}
				break;
			}
		}
		break;

	case 0xA000:
		{
			g_NESmapper.Mapper4.regs[2] = data;

			if(data & 0x40)
			{
				LOG("MAP4 MIRRORING: 0x40 ???" << endl);
			}

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
			g_NESmapper.Mapper4.regs[3] = data;

			if(data & 0x80)
			{
				// enable save RAM $6000-$7FFF
			}
			else
			{
				// disable save RAM $6000-$7FFF
			}
		}
		break;

	case 0xC000:
		g_NESmapper.Mapper4.regs[4] = data;
		g_NESmapper.Mapper4.irq_counter = g_NESmapper.Mapper4.regs[4];
		break;

	case 0xC001:
		g_NESmapper.Mapper4.regs[5] = data;
		g_NESmapper.Mapper4.irq_latch = g_NESmapper.Mapper4.regs[5];
		break;

	case 0xE000:
		g_NESmapper.Mapper4.regs[6] = data;
		g_NESmapper.Mapper4.irq_enabled = 0;
		break;

	case 0xE001:
		g_NESmapper.Mapper4.regs[7] = data;
		g_NESmapper.Mapper4.irq_enabled = 1;
		break;

	default:
		LOG("MAP4: UNKNOWN: " << HEX(addr,4) << " = " << HEX(data) << endl);
		break;

	}
}

void NES_mapper4_HSync(uint32 scanline)
{
	if(g_NESmapper.Mapper4.irq_enabled)
	{
		if((scanline >= 0) && (scanline <= 239))
		{
			if(NES_PPU_spr_enabled() || NES_PPU_bg_enabled())
			{
				if(g_NESmapper.Mapper4.patch == 1)
				{
					if(!(--g_NESmapper.Mapper4.irq_counter))
					{
						g_NESmapper.Mapper4.irq_counter = g_NESmapper.Mapper4.irq_latch;
						NES6502_DoIRQ();
					}
				}
				else if(g_NESmapper.Mapper4.patch == 2)
				{
					if(--g_NESmapper.Mapper4.irq_counter == 0x01)
					{
						g_NESmapper.Mapper4.irq_counter = g_NESmapper.Mapper4.irq_latch;
						NES6502_DoIRQ();
					}
				}
				else
				{
					if(!(g_NESmapper.Mapper4.irq_counter--))
					{
						g_NESmapper.Mapper4.irq_counter = g_NESmapper.Mapper4.irq_latch;
						NES6502_DoIRQ();
					}
				}
			}
		}
	}
}

void NES_mapper4_MMC3_set_CPU_banks()
{
	if(g_NESmapper.Mapper4.regs[0] & 0x40)
	{
		g_NESmapper.set_CPU_banks4(g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.Mapper4.prg1,
									g_NESmapper.Mapper4.prg0,g_NESmapper.num_8k_ROM_banks-1);
	}
	else
	{
		g_NESmapper.set_CPU_banks4(g_NESmapper.Mapper4.prg0,g_NESmapper.Mapper4.prg1,
									g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
	}
}

void NES_mapper4_MMC3_set_PPU_banks()
{
	if(g_NESmapper.num_1k_VROM_banks)
	{
		if(g_NESmapper.Mapper4.regs[0] & 0x80)
		{
			g_NESmapper.set_PPU_banks8(g_NESmapper.Mapper4.chr4,g_NESmapper.Mapper4.chr5,g_NESmapper.Mapper4.chr6,g_NESmapper.Mapper4.chr7,
				g_NESmapper.Mapper4.chr01,g_NESmapper.Mapper4.chr01+1,g_NESmapper.Mapper4.chr23,g_NESmapper.Mapper4.chr23+1);
		}
		else
		{
			g_NESmapper.set_PPU_banks8(g_NESmapper.Mapper4.chr01,g_NESmapper.Mapper4.chr01+1,g_NESmapper.Mapper4.chr23,g_NESmapper.Mapper4.chr23+1,
				g_NESmapper.Mapper4.chr4,g_NESmapper.Mapper4.chr5,g_NESmapper.Mapper4.chr6,g_NESmapper.Mapper4.chr7);
		}
	}
	else
	{
		if(g_NESmapper.Mapper4.regs[0] & 0x80)
		{
			g_NESmapper.set_VRAM_bank(0, g_NESmapper.Mapper4.chr4);
			g_NESmapper.set_VRAM_bank(1, g_NESmapper.Mapper4.chr5);
			g_NESmapper.set_VRAM_bank(2, g_NESmapper.Mapper4.chr6);
			g_NESmapper.set_VRAM_bank(3, g_NESmapper.Mapper4.chr7);
			g_NESmapper.set_VRAM_bank(4, g_NESmapper.Mapper4.chr01+0);
			g_NESmapper.set_VRAM_bank(5, g_NESmapper.Mapper4.chr01+1);
			g_NESmapper.set_VRAM_bank(6, g_NESmapper.Mapper4.chr23+0);
			g_NESmapper.set_VRAM_bank(7, g_NESmapper.Mapper4.chr23+1);
		}
		else
		{
			g_NESmapper.set_VRAM_bank(0, g_NESmapper.Mapper4.chr01+0);
			g_NESmapper.set_VRAM_bank(1, g_NESmapper.Mapper4.chr01+1);
			g_NESmapper.set_VRAM_bank(2, g_NESmapper.Mapper4.chr23+0);
			g_NESmapper.set_VRAM_bank(3, g_NESmapper.Mapper4.chr23+1);
			g_NESmapper.set_VRAM_bank(4, g_NESmapper.Mapper4.chr4);
			g_NESmapper.set_VRAM_bank(5, g_NESmapper.Mapper4.chr5);
			g_NESmapper.set_VRAM_bank(6, g_NESmapper.Mapper4.chr6);
			g_NESmapper.set_VRAM_bank(7, g_NESmapper.Mapper4.chr7);
		}
	}
}

#define MAP4_ROM(ptr)  (((ptr)-NES_ROM_get_ROM_banks())  >> 13)
#define MAP4_VROM(ptr) (((ptr)-NES_ROM_get_VROM_banks()) >> 10)
#define MAP4_VRAM(ptr) (((ptr)-NES_PPU_get_patt()) >> 10)

void NES_mapper4_SNSS_fixup() // HACK HACK HACK HACK
{
	nes6502_context context;
	NES6502_GetContext(&context);

	g_NESmapper.Mapper4.prg0 = MAP4_ROM(context.mem_page[(g_NESmapper.Mapper4.regs[0] & 0x40) ? 6 : 4]);
	g_NESmapper.Mapper4.prg1 = MAP4_ROM(context.mem_page[5]);
	if(g_NESmapper.num_1k_VROM_banks)
	{
		if(g_NESmapper.Mapper4.regs[0] & 0x80)
		{
			g_NESmapper.Mapper4.chr01 = MAP4_VROM(g_PPU.PPU_VRAM_banks[4]);
			g_NESmapper.Mapper4.chr23 = MAP4_VROM(g_PPU.PPU_VRAM_banks[6]);
			g_NESmapper.Mapper4.chr4  = MAP4_VROM(g_PPU.PPU_VRAM_banks[0]);
			g_NESmapper.Mapper4.chr5  = MAP4_VROM(g_PPU.PPU_VRAM_banks[1]);
			g_NESmapper.Mapper4.chr6  = MAP4_VROM(g_PPU.PPU_VRAM_banks[2]);
			g_NESmapper.Mapper4.chr7  = MAP4_VROM(g_PPU.PPU_VRAM_banks[3]);
		}
		else
		{
			g_NESmapper.Mapper4.chr01 = MAP4_VROM(g_PPU.PPU_VRAM_banks[0]);
			g_NESmapper.Mapper4.chr23 = MAP4_VROM(g_PPU.PPU_VRAM_banks[2]);
			g_NESmapper.Mapper4.chr4  = MAP4_VROM(g_PPU.PPU_VRAM_banks[4]);
			g_NESmapper.Mapper4.chr5  = MAP4_VROM(g_PPU.PPU_VRAM_banks[5]);
			g_NESmapper.Mapper4.chr6  = MAP4_VROM(g_PPU.PPU_VRAM_banks[6]);
			g_NESmapper.Mapper4.chr7  = MAP4_VROM(g_PPU.PPU_VRAM_banks[7]);
		}
	}
	else
	{
		if(g_NESmapper.Mapper4.regs[0] & 0x80)
		{
			g_NESmapper.Mapper4.chr01 = MAP4_VRAM(g_PPU.PPU_VRAM_banks[4]);
			g_NESmapper.Mapper4.chr23 = MAP4_VRAM(g_PPU.PPU_VRAM_banks[6]);
			g_NESmapper.Mapper4.chr4  = MAP4_VRAM(g_PPU.PPU_VRAM_banks[0]);
			g_NESmapper.Mapper4.chr5  = MAP4_VRAM(g_PPU.PPU_VRAM_banks[1]);
			g_NESmapper.Mapper4.chr6  = MAP4_VRAM(g_PPU.PPU_VRAM_banks[2]);
			g_NESmapper.Mapper4.chr7  = MAP4_VRAM(g_PPU.PPU_VRAM_banks[3]);
		}
		else
		{
			g_NESmapper.Mapper4.chr01 = MAP4_VRAM(g_PPU.PPU_VRAM_banks[0]);
			g_NESmapper.Mapper4.chr23 = MAP4_VRAM(g_PPU.PPU_VRAM_banks[2]);
			g_NESmapper.Mapper4.chr4  = MAP4_VRAM(g_PPU.PPU_VRAM_banks[4]);
			g_NESmapper.Mapper4.chr5  = MAP4_VRAM(g_PPU.PPU_VRAM_banks[5]);
			g_NESmapper.Mapper4.chr6  = MAP4_VRAM(g_PPU.PPU_VRAM_banks[6]);
			g_NESmapper.Mapper4.chr7  = MAP4_VRAM(g_PPU.PPU_VRAM_banks[7]);
		}
	}
}
/////////////////////////////////////////////////////////////////////

