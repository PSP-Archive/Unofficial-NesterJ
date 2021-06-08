/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#include "nes_6502.h"
#include "../nes.h"
#include "../../debug/debug.h"

// ‚‘¬‰»‚Ìˆ×ƒƒ‚ƒŠƒnƒ“ƒhƒ‰”pŽ~ ruka
/*
static void NES_write(uint32 address, uint8 value)
{
  NES6502_MemoryWrite(address, value);
}

static uint8 NES_read(uint32 address)
{
  return NES6502_MemoryRead(address);
}

static nes6502_memread NESReadHandler[] =
{
   // $0 - $7FF is RAM
   { 0x0800, 0xFFFF, NES_read },
   { -1,     -1,     NULL }
};

static nes6502_memwrite NESWriteHandler[] =
{
   // $0 - $7FF is RAM
   { 0x0800, 0xFFFF, NES_write },
   { -1,     -1,     NULL}
};
*/
void NES6502_Reset()                          { nes6502_reset(); }
int  NES6502_Execute(int total_cycles)        { return nes6502_execute(total_cycles); }
void NES6502_DoNMI(void)                      { nes6502_nmi(); }
void NES6502_DoIRQ(void)                      { nes6502_irq(); }
void NES6502_DoPendingIRQ(void)               { nes6502_pending_irq(); }
void NES6502_SetDMA(int cycles)               { nes6502_burn(cycles); }
uint8  NES6502_GetByte(uint32 address)        { return nes6502_getbyte(address); }
uint32 NES6502_GetCycles(boolean reset_flag)  { return nes6502_getcycles(reset_flag); }

// Context get/set
void NES6502_SetContext(nes6502_context *cpu)
{
  ASSERT(0x00000000 == (cpu->pc_reg & 0xFFFF0000));
//  cpu->read_handler = NESReadHandler;
//  cpu->write_handler = NESWriteHandler;
  nes6502_setcontext(cpu);
}

void NES6502_GetContext(nes6502_context *cpu)
{
  nes6502_getcontext(cpu);
//  cpu->read_handler = NESReadHandler;
//  cpu->write_handler = NESWriteHandler;
}
/*
uint8 NES6502_MemoryRead(uint32 addr)
{
  return NES_MemoryRead(addr);
}

void NES6502_MemoryWrite(uint32 addr, uint8 data)
{
  NES_MemoryWrite(addr, data);
}
*/
