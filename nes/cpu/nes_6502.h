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

// NES_6502
// created to cut down on extra work required for retro-fitting of
// new releases of Matt Conte's nes6502

#ifndef _NES_6502_H_
#define _NES_6502_H_

#include "../types.h"
#include "../nes.h"
#include "nes6502.h"

#ifdef __cplusplus
extern "C" {
#endif

void NES6502_Reset();
int  NES6502_Execute(int total_cycles);
void NES6502_DoNMI(void);
void NES6502_DoIRQ(void);
void NES6502_DoPendingIRQ(void);
void NES6502_SetDMA(int cycles);
uint8  NES6502_GetByte(uint32 address);
uint32 NES6502_GetCycles(boolean reset_flag);

uint8 NES6502_MemoryRead(uint32 addr);
void  NES6502_MemoryWrite(uint32 addr, uint8 data);

// Context get/set
void NES6502_SetContext(nes6502_context *cpu);
void NES6502_GetContext(nes6502_context *cpu);

#ifdef __cplusplus
}
#endif

#endif
