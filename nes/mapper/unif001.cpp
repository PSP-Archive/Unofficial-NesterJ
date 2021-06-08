#ifdef _NES_MAPPER_CPP_

// UNIF   Novel
void NES_UNIFmapper1_Init()
{
	g_NESmapper.Reset = NES_UNIFmapper1_Reset;
	g_NESmapper.MemoryWrite = NES_UNIFmapper1_MemoryWrite;
}

void NES_UNIFmapper1_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);
	// set PPU bank pointers
	g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
}


void NES_UNIFmapper1_MemoryWrite(uint32 addr, uint8 data)
{
//	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);

	unsigned char n = (unsigned char)addr;
	unsigned char pn = n&7;
	pn <<= 3;
	g_NESmapper.set_PPU_banks8(pn,pn+1,pn+2,pn+3,pn+4,pn+5,pn+6,pn+7);
	n&=3;
	n<<=2;
	g_NESmapper.set_CPU_banks4(n,n+1,n+2,n+3);
}

#endif
