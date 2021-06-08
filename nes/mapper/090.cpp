
/////////////////////////////////////////////////////////////////////
// Mapper 90
void NES_mapper90_Init()
{
	g_NESmapper.Reset = NES_mapper90_Reset;
	g_NESmapper.MemoryReadLow = NES_mapper90_MemoryReadLow;
	g_NESmapper.MemoryWriteLow = NES_mapper90_MemoryWriteLow;
	g_NESmapper.MemoryWrite = NES_mapper90_MemoryWrite;
	g_NESmapper.HSync = NES_mapper90_HSync;
}

void NES_mapper90_Reset()
{
	uint8 i;
	// set CPU bank pointers
	g_NESmapper.set_CPU_bank4(g_NESmapper.num_8k_ROM_banks-4);
	g_NESmapper.set_CPU_bank5(g_NESmapper.num_8k_ROM_banks-3);
	g_NESmapper.set_CPU_bank6(g_NESmapper.num_8k_ROM_banks-2);
	g_NESmapper.set_CPU_bank7(g_NESmapper.num_8k_ROM_banks-1);

	// set PPU bank pointers
	g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);

	g_NESmapper.Mapper90.irq_counter = 0;
	g_NESmapper.Mapper90.irq_latch = 0;
	g_NESmapper.Mapper90.irq_enabled = 0;

	g_NESmapper.Mapper90.chr_bank_size=0;
	g_NESmapper.Mapper90.prg_bank_size=0;

	for(i = 0; i < 4; i++)
	{
		g_NESmapper.Mapper90.prg_reg[i] = g_NESmapper.num_8k_ROM_banks-4+i;
		g_NESmapper.Mapper90.nam_low_reg[i] = 0;
		g_NESmapper.Mapper90.nam_high_reg[i] = 0;
		g_NESmapper.Mapper90.chr_low_reg[i] = i;
		g_NESmapper.Mapper90.chr_high_reg[i] = 0;
		g_NESmapper.Mapper90.chr_low_reg[i+4] = i+4;
		g_NESmapper.Mapper90.chr_high_reg[i+4] = 0;
	}
	g_NESmapper.Mapper90.value1^=1;
	g_NESmapper.Mapper90.mode = 0;
}

uint8 NES_mapper90_MemoryReadLow(uint32 addr)
{
#if 0
	if(addr == 0x5000)
	{
		return (uint8)(g_NESmapper.Mapper90.value1*g_NESmapper.Mapper90.value2 & 0x00FF);
	}
	else
	{
		return (uint8)(addr >> 8);
	}
#else
	if(g_NESmapper.Mapper90.value1&1)
		return 0xFF;
	else
		return 0;
#endif
}

void NES_mapper90_MemoryWriteLow(uint32 addr, uint8 data)
{
#if 0
	if(addr == 0x5000)
	{
		g_NESmapper.Mapper90.value1 = data;
	}
	else if(addr == 0x5001)
	{
		g_NESmapper.Mapper90.value2 = data;
	}
#endif
}

void NES_mapper90_MemoryWrite(uint32 addr, uint8 data)
{
	switch(addr&0xF007)
	{
	case 0x8000:
	case 0x8001:
	case 0x8002:
	case 0x8003:
		{
			g_NESmapper.Mapper90.prg_reg[addr & 0x03] = data;
			NES_mapper90_Sync_Prg_Banks();
			//	  LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(g_NESmapper.Mapper90.prg_bank_size,2) << endl);
		}
		break;

	case 0x9000:
	case 0x9001:
	case 0x9002:
	case 0x9003:
	case 0x9004:
	case 0x9005:
	case 0x9006:
	case 0x9007:
		{
			g_NESmapper.Mapper90.chr_low_reg[addr & 0x07] = data;
			NES_mapper90_Sync_Chr_Banks();
		}
		break;

	case 0xA000:
	case 0xA001:
	case 0xA002:
	case 0xA003:
	case 0xA004:
	case 0xA005:
	case 0xA006:
	case 0xA007:
		{
			g_NESmapper.Mapper90.chr_high_reg[addr & 0x07] = data;
			NES_mapper90_Sync_Chr_Banks();
		}
		break;

	case 0xB000:
	case 0xB001:
	case 0xB002:
	case 0xB003:
		{
			g_NESmapper.Mapper90.nam_low_reg[addr & 0x03] = data;
			NES_mapper90_Sync_Mirror();
		}
		break;

	case 0xB004:
	case 0xB005:
	case 0xB006:
	case 0xB007:
		{
			g_NESmapper.Mapper90.nam_high_reg[addr & 0x03] = data;
			NES_mapper90_Sync_Mirror();
		}
		break;

	case 0xC002:
		{
			g_NESmapper.Mapper90.irq_enabled = 0;
		}
		break;

	case 0xC003:
	case 0xC004:
		{
			if(g_NESmapper.Mapper90.irq_enabled == 0)
			{
				g_NESmapper.Mapper90.irq_enabled = 1;
				g_NESmapper.Mapper90.irq_counter = g_NESmapper.Mapper90.irq_latch;
			}
		}
		break;

	case 0xC005:
		{
			g_NESmapper.Mapper90.irq_counter = data;
			g_NESmapper.Mapper90.irq_latch = data;
		}
		break;

	case 0xD000:
		{
			g_NESmapper.Mapper90.prg_bank_6000 = data & 0x80;
			g_NESmapper.Mapper90.prg_bank_e000 = data & 0x04;
			g_NESmapper.Mapper90.prg_bank_size = data & 0x03;
			g_NESmapper.Mapper90.chr_bank_size = (data & 0x18) >> 3;
			g_NESmapper.Mapper90.mirror_mode = data & 0x20;
			NES_mapper90_Sync_Prg_Banks();
			NES_mapper90_Sync_Chr_Banks();
			NES_mapper90_Sync_Mirror();
			g_NESmapper.Mapper90.mode = data;
		}
		break;

	case 0xD001:
		{
			g_NESmapper.Mapper90.mirror_type = data & 0x03;
			NES_mapper90_Sync_Mirror();
		}
		break;

	case 0xD003:
		{
			// bank page
		}
		break;
	}
	//  LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << "  " << HEX(g_NESmapper.Mapper90.prg_bank_size,2) << endl);
}

void NES_mapper90_HSync(uint32 scanline)
{
	if((scanline >= 0) && (scanline <= 239)) //239
	{
		if(NES_PPU_spr_enabled() || NES_PPU_bg_enabled())
		{
			if(--g_NESmapper.Mapper90.irq_counter == 0)
			{
				if(g_NESmapper.Mapper90.irq_enabled)
				{
					NES6502_DoIRQ();
				}
				//		 g_NESmapper.Mapper90.irq_counter = g_NESmapper.Mapper90.irq_latch;
				//       g_NESmapper.Mapper90.irq_latch = 0;
				//         g_NESmapper.Mapper90.irq_enabled = 0;
			}
			else
			{
				//		  g_NESmapper.Mapper90.irq_counter--;
			}
		}
	}
}

void NES_mapper90_Sync_Mirror()
{
	uint8 i;
	uint32 nam_bank[4];

	for(i = 0; i < 4; i++)
	{
		nam_bank[i] = ((uint32)g_NESmapper.Mapper90.nam_high_reg[i] << 8) | (uint32)g_NESmapper.Mapper90.nam_low_reg[i];
	}

	if(g_NESmapper.Mapper90.mirror_mode)
	{
		for(i = 0; i < 4; i++)
		{
			if(!g_NESmapper.Mapper90.nam_high_reg[i] && (g_NESmapper.Mapper90.nam_low_reg[i] == i))
			{
				g_NESmapper.Mapper90.mirror_mode = 0;
			}
		}

		if(g_NESmapper.Mapper90.mirror_mode)
		{
			g_NESmapper.set_PPU_bank8(nam_bank[0]);
			g_NESmapper.set_PPU_bank9(nam_bank[1]);
			g_NESmapper.set_PPU_bank10(nam_bank[2]);
			g_NESmapper.set_PPU_bank11(nam_bank[3]);
		}
	}
	else
	{
		if(g_NESmapper.Mapper90.mirror_type == 0)
		{
			g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
		}
		else if(g_NESmapper.Mapper90.mirror_type == 1)
		{
			g_NESmapper.set_mirroring2(NES_PPU_MIRROR_HORIZ);
		}
		else
		{
			g_NESmapper.set_mirroring(0,0,0,0);
		}
	}
}

void NES_mapper90_Sync_Chr_Banks()
{
	uint8 i;
	uint32 chr_bank[8];

	for(i = 0; i < 8; i++)
	{
		chr_bank[i] = ((uint32)g_NESmapper.Mapper90.chr_high_reg[i] << 8) | (uint32)g_NESmapper.Mapper90.chr_low_reg[i];
	}

	if(g_NESmapper.Mapper90.chr_bank_size == 0)
	{
		g_NESmapper.set_PPU_bank0(chr_bank[0]*8+0);
		g_NESmapper.set_PPU_bank1(chr_bank[0]*8+1);
		g_NESmapper.set_PPU_bank2(chr_bank[0]*8+2);
		g_NESmapper.set_PPU_bank3(chr_bank[0]*8+3);
		g_NESmapper.set_PPU_bank4(chr_bank[0]*8+4);
		g_NESmapper.set_PPU_bank5(chr_bank[0]*8+5);
		g_NESmapper.set_PPU_bank6(chr_bank[0]*8+6);
		g_NESmapper.set_PPU_bank7(chr_bank[0]*8+7);
	}
	else if(g_NESmapper.Mapper90.chr_bank_size == 1)
	{
		g_NESmapper.set_PPU_bank0(chr_bank[0]*4+0);
		g_NESmapper.set_PPU_bank1(chr_bank[0]*4+1);
		g_NESmapper.set_PPU_bank2(chr_bank[0]*4+2);
		g_NESmapper.set_PPU_bank3(chr_bank[0]*4+3);
		g_NESmapper.set_PPU_bank4(chr_bank[4]*4+0);
		g_NESmapper.set_PPU_bank5(chr_bank[4]*4+1);
		g_NESmapper.set_PPU_bank6(chr_bank[4]*4+2);
		g_NESmapper.set_PPU_bank7(chr_bank[4]*4+3);
	}
	else if(g_NESmapper.Mapper90.chr_bank_size == 2)
	{
		g_NESmapper.set_PPU_bank0(chr_bank[0]*2+0);
		g_NESmapper.set_PPU_bank1(chr_bank[0]*2+1);
		g_NESmapper.set_PPU_bank2(chr_bank[2]*2+0);
		g_NESmapper.set_PPU_bank3(chr_bank[2]*2+1);
		g_NESmapper.set_PPU_bank4(chr_bank[4]*2+0);
		g_NESmapper.set_PPU_bank5(chr_bank[4]*2+1);
		g_NESmapper.set_PPU_bank6(chr_bank[6]*2+0);
		g_NESmapper.set_PPU_bank7(chr_bank[6]*2+1);
	}
	else
	{
		g_NESmapper.set_PPU_bank0(chr_bank[0]);
		g_NESmapper.set_PPU_bank1(chr_bank[1]);
		g_NESmapper.set_PPU_bank2(chr_bank[2]);
		g_NESmapper.set_PPU_bank3(chr_bank[3]);
		g_NESmapper.set_PPU_bank4(chr_bank[4]);
		g_NESmapper.set_PPU_bank5(chr_bank[5]);
		g_NESmapper.set_PPU_bank6(chr_bank[6]);
		g_NESmapper.set_PPU_bank7(chr_bank[7]);
	}
}

void NES_mapper90_Sync_Prg_Banks()
{
#if 1
	if(g_NESmapper.Mapper90.prg_bank_size == 0)
	{
		g_NESmapper.set_CPU_bank4(g_NESmapper.num_8k_ROM_banks-4);
		g_NESmapper.set_CPU_bank5(g_NESmapper.num_8k_ROM_banks-3);
		g_NESmapper.set_CPU_bank6(g_NESmapper.num_8k_ROM_banks-2);
		g_NESmapper.set_CPU_bank7(g_NESmapper.num_8k_ROM_banks-1);
	}
	else if(g_NESmapper.Mapper90.prg_bank_size == 1)
	{
		g_NESmapper.set_CPU_bank4(g_NESmapper.Mapper90.prg_reg[1]*2);
		g_NESmapper.set_CPU_bank5(g_NESmapper.Mapper90.prg_reg[1]*2+1);
		g_NESmapper.set_CPU_bank6(g_NESmapper.num_8k_ROM_banks-2);
		g_NESmapper.set_CPU_bank7(g_NESmapper.num_8k_ROM_banks-1);
	}
	else if(g_NESmapper.Mapper90.prg_bank_size == 2)
	{
		if(g_NESmapper.Mapper90.prg_bank_e000)
		{
			g_NESmapper.set_CPU_bank4(g_NESmapper.Mapper90.prg_reg[0]);
			g_NESmapper.set_CPU_bank5(g_NESmapper.Mapper90.prg_reg[1]);
			g_NESmapper.set_CPU_bank6(g_NESmapper.Mapper90.prg_reg[2]);
			g_NESmapper.set_CPU_bank7(g_NESmapper.Mapper90.prg_reg[3]);
		}
		else
		{
			if(g_NESmapper.Mapper90.prg_bank_6000)
			{
				g_NESmapper.set_CPU_bank3(g_NESmapper.Mapper90.prg_reg[3]);
			}
			g_NESmapper.set_CPU_bank4(g_NESmapper.Mapper90.prg_reg[0]);
			g_NESmapper.set_CPU_bank5(g_NESmapper.Mapper90.prg_reg[1]);
			g_NESmapper.set_CPU_bank6(g_NESmapper.Mapper90.prg_reg[2]);
			g_NESmapper.set_CPU_bank7(g_NESmapper.num_8k_ROM_banks-1);
		}
	}
	else
	{
		// 8k in reverse g_NESmapper.Mapper90.mode?
		g_NESmapper.set_CPU_bank4(g_NESmapper.Mapper90.prg_reg[3]);
		g_NESmapper.set_CPU_bank5(g_NESmapper.Mapper90.prg_reg[2]);
		g_NESmapper.set_CPU_bank6(g_NESmapper.Mapper90.prg_reg[1]);
		g_NESmapper.set_CPU_bank7(g_NESmapper.Mapper90.prg_reg[0]);
	}
#else
	switch(g_NESmapper.Mapper90.mode&3){
	case 0:
		if(g_NESmapper.Mapper90.mode&4){
			uint32 bn = g_NESmapper.Mapper90.prg_reg[3]<<2;
			g_NESmapper.set_CPU_banks(bn, bn+1, bn+2, bn+3);
		}
		else{
			g_NESmapper.set_CPU_bank4(g_NESmapper.num_8k_ROM_banks-4);
			g_NESmapper.set_CPU_bank5(g_NESmapper.num_8k_ROM_banks-3);
			g_NESmapper.set_CPU_bank6(g_NESmapper.num_8k_ROM_banks-2);
			g_NESmapper.set_CPU_bank7(g_NESmapper.num_8k_ROM_banks-1);
		}
		if(g_NESmapper.Mapper90.mode&0x80){
			g_NESmapper.set_CPU_bank3((g_NESmapper.Mapper90.prg_reg[3]<<2)|3);
		}
		break;
	case 1:
		g_NESmapper.set_CPU_bank4(g_NESmapper.Mapper90.prg_reg[1]<<1);
		g_NESmapper.set_CPU_bank5(g_NESmapper.Mapper90.prg_reg[1]<<1+1);
		if(g_NESmapper.Mapper90.mode&4){
			g_NESmapper.set_CPU_bank6(g_NESmapper.Mapper90.prg_reg[3]<<1);
			g_NESmapper.set_CPU_bank7(g_NESmapper.Mapper90.prg_reg[3]<<1+1);
		}
		else{
			g_NESmapper.set_CPU_bank6(g_NESmapper.num_8k_ROM_banks-2);
			g_NESmapper.set_CPU_bank7(g_NESmapper.num_8k_ROM_banks-1);
		}
		if(g_NESmapper.Mapper90.mode&0x80){
			g_NESmapper.set_CPU_bank3((g_NESmapper.Mapper90.prg_reg[3]<<1)|1);
		}
		break;
	case 2:
		g_NESmapper.set_CPU_bank4(g_NESmapper.Mapper90.prg_reg[0]);
		g_NESmapper.set_CPU_bank5(g_NESmapper.Mapper90.prg_reg[1]);
		g_NESmapper.set_CPU_bank6(g_NESmapper.Mapper90.prg_reg[2]);

		if(g_NESmapper.Mapper90.mode & 0x04){
			g_NESmapper.set_CPU_bank7(g_NESmapper.Mapper90.prg_reg[3]);
		}
		else{
			g_NESmapper.set_CPU_bank7(g_NESmapper.num_8k_ROM_banks-1);
		}
		if(g_NESmapper.Mapper90.mode & 0x80){
			g_NESmapper.set_CPU_bank3(g_NESmapper.Mapper90.prg_reg[3]);
		}
		break;
	case 3:
		g_NESmapper.set_CPU_bank4(g_NESmapper.Mapper90.prg_reg[2]);
		g_NESmapper.set_CPU_bank5(g_NESmapper.Mapper90.prg_reg[3]);
		g_NESmapper.set_CPU_bank6(g_NESmapper.Mapper90.prg_reg[0]);

		if(g_NESmapper.Mapper90.mode & 0x04){
			g_NESmapper.set_CPU_bank7(g_NESmapper.Mapper90.prg_reg[1]);
		}
		else{
			g_NESmapper.set_CPU_bank7(g_NESmapper.num_8k_ROM_banks-1);
		}
		if(g_NESmapper.Mapper90.mode & 0x80){
			g_NESmapper.set_CPU_bank3(g_NESmapper.Mapper90.prg_reg[3]);
		}
		break;
		break;
	}
#endif
}
/////////////////////////////////////////////////////////////////////

