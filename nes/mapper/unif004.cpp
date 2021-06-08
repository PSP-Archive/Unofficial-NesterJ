#ifdef _NES_MAPPER_CPP_

// UNIF   Super24 in 1
void NES_UNIFmapper4_Init()
{
	g_NESmapper.Reset = NES_UNIFmapper4_Reset;
	g_NESmapper.MemoryWrite = NES_UNIFmapper4_MemoryWrite;
	g_NESmapper.MemoryWriteLow = NES_UNIFmapper4_MemoryWriteLow;
	g_NESmapper.HSync = NES_UNIFmapper4_HSync;
}
void NES_UNIFmapper4_Reset()
{
	g_NESmapper.NES_UNIFmapper4.irq_counter = g_NESmapper.NES_UNIFmapper4.irq_latch =0;
	g_NESmapper.NES_UNIFmapper4.irq_enabled = 0;
	_memset(g_NESmapper.NES_UNIFmapper4.dregs, 0, 8);
	g_NESmapper.NES_UNIFmapper4.regs[0] = g_NESmapper.NES_UNIFmapper4.regs[1] =0;
	g_NESmapper.NES_UNIFmapper4.regs[2]=0x24;
	g_NESmapper.NES_UNIFmapper4.regs[3]=159;
	g_NESmapper.NES_UNIFmapper4.regs[4]=0;
	g_NESmapper.NES_UNIFmapper4.dregs[7]=1;
//	NES_CYCLES_PER_LINE=120;
	NES_UNIFmapper4_oMMC3PRG(0);
	NES_UNIFmapper4_oMMC3CHR(0);
//	set_mirroring(NES_PPU_MIRROR_VERT);
}


void NES_UNIFmapper4_setchr1(uint8 pn, uint8 data)
{
	if(g_NESmapper.NES_UNIFmapper4.regs[2]&0x20){
//		set_VRAM_bank_unif(data, pn);
	}
	else{
		uint32 tdata= (uint32)data;
		tdata+=(g_NESmapper.NES_UNIFmapper4.regs[4]<<3);
		g_NESmapper.set_VRAM_bank_unif(tdata/*&0x7FF*/, pn);
	}
}

void NES_UNIFmapper4_setchr2(uint8 pn, uint8 data)
{
	if(g_NESmapper.NES_UNIFmapper4.regs[2]&0x20){
//		set_VRAM_bank_unif(data, pn);
//		set_VRAM_bank_unif(data+1, pn+1);
	}
	else{
		uint32 tdata = (uint32)data;
		tdata+=(g_NESmapper.NES_UNIFmapper4.regs[4]<<3);
		g_NESmapper.set_VRAM_bank_unif(tdata/*&0x7FF*/, (uint8)pn);
		g_NESmapper.set_VRAM_bank_unif(tdata+1/*&0x7FF+1*/, (uint8)(pn+1));
	}
}


static unsigned char mask8[8]={63,31,15,1,3,0,0,0};


void NES_UNIFmapper4_oMMC3PRG(uint8 data)
{
	uint32 tmp;

	tmp = g_NESmapper.NES_UNIFmapper4.dregs[7];
	tmp&=mask8[g_NESmapper.NES_UNIFmapper4.regs[2]&7];
	tmp|=(g_NESmapper.NES_UNIFmapper4.regs[3]<<1);
	g_NESmapper.set_CPU_bank_unif(tmp, 5);
	tmp = 0xff;
	tmp&=mask8[g_NESmapper.NES_UNIFmapper4.regs[2]&7];
	tmp|=(g_NESmapper.NES_UNIFmapper4.regs[3]<<1);
	g_NESmapper.set_CPU_bank_unif(tmp, 7);
	if(data&0x40){
		tmp = g_NESmapper.NES_UNIFmapper4.dregs[6];
		tmp&=mask8[g_NESmapper.NES_UNIFmapper4.regs[2]&7];
		tmp|=(g_NESmapper.NES_UNIFmapper4.regs[3]<<1);
		g_NESmapper.set_CPU_bank_unif(tmp, 6);

		tmp = 0xfe;
		tmp&=mask8[g_NESmapper.NES_UNIFmapper4.regs[2]&7];
		tmp|=(g_NESmapper.NES_UNIFmapper4.regs[3]<<1);
		g_NESmapper.set_CPU_bank_unif(tmp, 4);
	}
	else{
		tmp = g_NESmapper.NES_UNIFmapper4.dregs[6];
		tmp&=mask8[g_NESmapper.NES_UNIFmapper4.regs[2]&7];
		tmp|=(g_NESmapper.NES_UNIFmapper4.regs[3]<<1);
		g_NESmapper.set_CPU_bank_unif(tmp, 4);	//

		tmp = 0xfe;
		tmp&=mask8[g_NESmapper.NES_UNIFmapper4.regs[2]&7];
		tmp|=(g_NESmapper.NES_UNIFmapper4.regs[3]<<1);
		g_NESmapper.set_CPU_bank_unif(tmp, 6);	//
	}
}

void NES_UNIFmapper4_oMMC3CHR(uint8 data)
{
	uint8 cm=(data&0x80)>>5;
	NES_UNIFmapper4_setchr2((uint8)(cm^0),(uint8)g_NESmapper.NES_UNIFmapper4.dregs[0]);
	NES_UNIFmapper4_setchr2((uint8)(cm^2),(uint8)g_NESmapper.NES_UNIFmapper4.dregs[1]);
	NES_UNIFmapper4_setchr1((uint8)(cm^4),(uint8)g_NESmapper.NES_UNIFmapper4.dregs[2]);
	NES_UNIFmapper4_setchr1((uint8)(cm^5),(uint8)g_NESmapper.NES_UNIFmapper4.dregs[3]);
	NES_UNIFmapper4_setchr1((uint8)(cm^6),(uint8)g_NESmapper.NES_UNIFmapper4.dregs[4]);
	NES_UNIFmapper4_setchr1((uint8)(cm^7),(uint8)g_NESmapper.NES_UNIFmapper4.dregs[5]);
}

void NES_UNIFmapper4_MemoryWriteLow(uint32 addr, uint8 data)
{
	switch(addr){
		case 0x5ff0:
			g_NESmapper.NES_UNIFmapper4.regs[2]=data;
			NES_UNIFmapper4_oMMC3PRG(g_NESmapper.NES_UNIFmapper4.regs[1]);
			NES_UNIFmapper4_oMMC3CHR(g_NESmapper.NES_UNIFmapper4.regs[1]);
			break;
		case 0x5FF1:
			g_NESmapper.NES_UNIFmapper4.regs[3]=data;
			NES_UNIFmapper4_oMMC3PRG(g_NESmapper.NES_UNIFmapper4.regs[1]);
			break;
		case 0x5FF2:
			g_NESmapper.NES_UNIFmapper4.regs[4]=data;
			NES_UNIFmapper4_oMMC3CHR(g_NESmapper.NES_UNIFmapper4.regs[1]);
			break;
	}
}


void NES_UNIFmapper4_MemoryWrite(uint32 addr, uint8 data)
{
//	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
	switch(addr&0xE001){
		case 0x8000:
			if((data&0x40) != (g_NESmapper.NES_UNIFmapper4.regs[1]&0x40))
				NES_UNIFmapper4_oMMC3PRG(data);
			if((data&0x80) != (g_NESmapper.NES_UNIFmapper4.regs[1]&0x80))
				NES_UNIFmapper4_oMMC3CHR(data);
			g_NESmapper.NES_UNIFmapper4.regs[1] = data;
			break;
		case 0x8001:
		{
			uint8 pm = (g_NESmapper.NES_UNIFmapper4.regs[1]&0x80)>>5;
			g_NESmapper.NES_UNIFmapper4.dregs[g_NESmapper.NES_UNIFmapper4.regs[1]&0x7] = data;
			switch(g_NESmapper.NES_UNIFmapper4.regs[1]&0x07){
				case 0:
					NES_UNIFmapper4_setchr2((uint8)(pm^0),data);
					break;
				case 1:
					NES_UNIFmapper4_setchr2((uint8)(pm^2), data);
					break;
				case 2:
					NES_UNIFmapper4_setchr1((uint8)(pm^4), data);
					break;
				case 3:
					NES_UNIFmapper4_setchr1((uint8)(pm^5), data);
					break;
				case 4:
					NES_UNIFmapper4_setchr1((uint8)(pm^6), data);
					break;
				case 5:
					NES_UNIFmapper4_setchr1((uint8)(pm^7), data);
					break;
				case 6:
					if(g_NESmapper.NES_UNIFmapper4.regs[1]&0x40){
						uint32 tmp = data;
						tmp&=mask8[g_NESmapper.NES_UNIFmapper4.regs[2]&7];
						tmp|=(g_NESmapper.NES_UNIFmapper4.regs[3]<<1);
						g_NESmapper.set_CPU_bank_unif(tmp, 6);	//
					}
					else{
						uint32 tmp = data;
						tmp&=mask8[g_NESmapper.NES_UNIFmapper4.regs[2]&7];
						tmp|=(g_NESmapper.NES_UNIFmapper4.regs[3]<<1);
						g_NESmapper.set_CPU_bank_unif(tmp, 4);
					}
					break;
				case 7:
					{
						uint32 tmp = data;
						tmp&=mask8[g_NESmapper.NES_UNIFmapper4.regs[2]&7];
						tmp|=(g_NESmapper.NES_UNIFmapper4.regs[3]<<1);
						g_NESmapper.set_CPU_bank_unif(tmp, 5);
					}
					break;
			}
		}
			break;
		case 0xA000:
			if(data&1)
				g_NESmapper.set_mirroring2(NES_PPU_MIRROR_HORIZ);
			else
				g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
			break;

		case 0xc000:
			g_NESmapper.NES_UNIFmapper4.irq_latch=data;
			if(g_NESmapper.NES_UNIFmapper4.regs[0]==1)
				g_NESmapper.NES_UNIFmapper4.irq_counter=g_NESmapper.NES_UNIFmapper4.irq_latch;
			break;
		case 0xc001:
			g_NESmapper.NES_UNIFmapper4.regs[0]=1;
			g_NESmapper.NES_UNIFmapper4.irq_counter=g_NESmapper.NES_UNIFmapper4.irq_latch;
			break;
		case 0xE000:
			g_NESmapper.NES_UNIFmapper4.irq_enabled=0;
			if(g_NESmapper.NES_UNIFmapper4.regs[0]==1)
				g_NESmapper.NES_UNIFmapper4.irq_counter=g_NESmapper.NES_UNIFmapper4.irq_latch;
			break;
		case 0xE001:
			g_NESmapper.NES_UNIFmapper4.irq_enabled=1;
			if(g_NESmapper.NES_UNIFmapper4.regs[0]==1)
				g_NESmapper.NES_UNIFmapper4.irq_counter=g_NESmapper.NES_UNIFmapper4.irq_latch;
			break;
	}
}


void NES_UNIFmapper4_HSync(uint32 scanline)
{
	if(g_NESmapper.NES_UNIFmapper4.irq_enabled){
		if(NES_PPU_spr_enabled() || NES_PPU_bg_enabled()){
			if(scanline >= 0 && scanline <= 240){
				g_NESmapper.NES_UNIFmapper4.regs[0]=0;
				if(g_NESmapper.NES_UNIFmapper4.irq_counter-- == 0){
					NES6502_DoPendingIRQ();
					g_NESmapper.NES_UNIFmapper4.regs[0]=1;
//					g_NESmapper.NES_UNIFmapper4.irq_counter = g_NESmapper.NES_UNIFmapper4.irq_latch;
//					g_NESmapper.NES_UNIFmapper4.irq_enabled = 0;
				}
			}
		}
	}
}

#endif
