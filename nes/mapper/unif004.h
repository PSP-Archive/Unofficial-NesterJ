
//
void NES_UNIFmapper4_Reset();
void NES_UNIFmapper4_setchr1(uint8 pn, uint8 data);
void NES_UNIFmapper4_setchr2(uint8 pn, uint8 data);
void NES_UNIFmapper4_oMMC3PRG(uint8 data);
void NES_UNIFmapper4_oMMC3CHR(uint8 data);
void NES_UNIFmapper4_MemoryWriteLow(uint32 addr, uint8 data);
void NES_UNIFmapper4_MemoryWrite(uint32 addr, uint8 data);
void NES_UNIFmapper4_HSync(uint32 scanline);

void NES_UNIFmapper4_Init();
/////////////////////////////////////////////////////////////////////

