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
**
** 圧縮ファイル対応のため、みかみかな によって、Win32 APIの File IO に
** 書きかえられています。
**
** 武田によってFAM,g_ROM.fds形式の読み込みに関するコードが追加されています。
**
** PSP移植のためCPPからCに変更しています。
*/

//#include <windows.h>
//#include <shlwapi.h>
#include "nes_rom.h"
//#include "arc.h"
#include "../debug/debug.h"
//#include "crc32.h"
#include "nes_string.h"
//#include "resource.h"
#include "nes_crc32.h"
#include "../loadromimage.h"
#include "nes_config.h"
#include "nes_string.h"
#include "fileio.h"



NES_ROM g_ROM;	// ROM
#define MAX_IMAGE_SIZE 1024*1024+40960	// 最大1M+40Kの読み込みバッファ
uint8 g_ReadRomImage[MAX_IMAGE_SIZE];

#define CopyMemIncPtr(o,i,s) \
{\
	_memcpy(o,i,s);\
	i+=s;\
}

NES_ROM *NES_ROM_LoadRom(const char* fn)
{
	uint8 *p = NULL;
	uint8 *buf = g_ReadRomImage;
	uint32 filesize;
	uint8 image_type;
	const char *nesExtensions[] = { "*.nes","*.fds","*.fam","*.unf","*.nsf",NULL };

	_memset(g_ROM.trainer, 0x00, sizeof(g_ROM.trainer));
	_memset(g_ROM.rom_name, 0x00, sizeof(g_ROM.rom_name));
	_memset(g_ROM.rom_path, 0x00, sizeof(g_ROM.rom_path));
	g_ROM.crc = 0;
	g_ROM.crc_all = 0;
	g_ROM.fds = 0;
	g_ROM.unif_mapper=0;
	g_ROM.GameTitle[0] =0;
	g_ROM.dbcorrect[0]=g_ROM.dbcorrect[1]=g_ROM.dbcorrect[2]=0;

	image_type = 0;

	// store filename and path
	NES_ROM_GetPathInfo( fn );
	DEBUGVALUE("filesize", filesize);

	filesize = load_rom_image(buf, sizeof(g_ReadRomImage), fn);
	if (!filesize) {
		DEBUG("load_rom_image failed");
		return NULL;
	}

	// Header check...

	p = buf;
	CopyMemIncPtr( &g_ROM.header, p, sizeof(NES_header) );
	if( ( !_strncmp( (char*)g_ROM.header.id, "NES", 3) && ( g_ROM.header.ctrl_z == 0x1A) ) ||
		    ( !_strncmp( (char*)g_ROM.header.id, "NEZ", 3) && ( g_ROM.header.ctrl_z == 0x1A) ) ||
		    ( !_strncmp( (char*)g_ROM.header.id, "FDS", 3) && ( g_ROM.header.ctrl_z == 0x1A) ) ||
		    ( g_ROM.header.id[0] <= 0x1A && g_ROM.header.id[1] == 0x00 && (g_ROM.header.id[2] || g_ROM.header.id[3] || g_ROM.header.ctrl_z)&& g_ROM.header.num_8k_vrom_banks == 0x00 ) ||
		    ( !_strncmp( (char*)g_ROM.header.id, "NES", 3) && (g_ROM.header.ctrl_z == 'M'  ) ||
		    ( !_strncmp( (char*)g_ROM.header.id, "UNIF", 4)))
	  ) {
		p=buf+sizeof(NES_header);
	}
	else
	{
		DEBUG("UNKNOWN FORMAT!");
		return NULL;
	}

	// internal check...

	if( !_strncmp( (char*)g_ROM.header.id, "NES", 3) && ( g_ROM.header.ctrl_z == 0x1A) ||
		    !_strncmp( (char*)g_ROM.header.id, "NEZ", 3) && ( g_ROM.header.ctrl_z == 0x1A) ) {
//		if(!g_NESConfig.preferences.DisableIPSPatch)
//			LoadIPSPatch((unsigned char *)p, (char *)fn);		//IPS PATCH

		// load g_ROM.trainer if present
		if( NES_ROM_has_trainer() )
		{
			if( p+TRAINER_LEN-buf > filesize ){
				DEBUG("Error reading Trainer");
				return NULL;
			}
			CopyMemIncPtr( g_ROM.trainer, p, TRAINER_LEN );
			g_ROM.crc = CrcCalc(g_ROM.trainer, TRAINER_LEN);
		}

		if( p + (16*1024) * g_ROM.header.num_16k_rom_banks - buf > filesize ){
			DEBUG("Error reading ROM banks");
			return NULL;
		}
		CopyMemIncPtr( g_ROM.ROM_banks, p, (16*1024) * g_ROM.header.num_16k_rom_banks );

		if( p + (8*1024) * g_ROM.header.num_8k_vrom_banks - buf > filesize ){
			DEBUG("Error reading VROM banks");
			return NULL;
		}
		CopyMemIncPtr( g_ROM.VROM_banks, p, (8*1024) * g_ROM.header.num_8k_vrom_banks );
		if(((g_ROM.header.flags_1 >> 4) | (g_ROM.header.flags_2 & 0xF0)) == 20)
		{
			uint32 i;
			uint8 disk_num;
			uint8 disk[0x10000];
			uint8 disk_header[15] =
				{
				    0x01,0x2A,0x4E,0x49,0x4E,0x54,0x45,0x4E,0x44,0x4F,0x2D,0x48,0x56,0x43,0x2A
				};
			image_type = 1;
			// convert NES disk image
			disk_num = g_ROM.header.num_16k_rom_banks >> 2;
			if(disk_num > 4)
			{
				disk_num = 4;
			}

			for (i = 0; i < disk_num; i++)
			{
				int rpos, wpos;
				wpos = i*65500+16;
				rpos = i*0x10000;

				_memcpy(disk, &g_ROM.ROM_banks[rpos], 65500);
				_memcpy(&g_ROM.ROM_banks[wpos], disk_header, 15);
				wpos+=15;
				_memcpy(&g_ROM.ROM_banks[wpos], disk, 65500-15);
			}

			//g_ROM.header.id[0] = 'N';
			//g_ROM.header.id[1] = 'E';
			//g_ROM.header.id[2] = 'S';
			//g_ROM.header.num_16k_rom_banks = disk_num*4;
			//g_ROM.header.num_8k_vrom_banks = 0;
			//g_ROM.header.flags_1 = 0x40;
			//g_ROM.header.flags_2 = 0x10;
			//g_ROM.header.reserved[0] = g_ROM.header.reserved[1] = g_ROM.header.reserved[2] = g_ROM.header.reserved[3] = 0;
			//g_ROM.header.reserved[4] = g_ROM.header.reserved[5] = g_ROM.header.reserved[6] = g_ROM.header.reserved[7] = 0;
			g_ROM.ROM_banks[0] = 'F';
			g_ROM.ROM_banks[1] = 'D';
			g_ROM.ROM_banks[2] = 'S';
			g_ROM.ROM_banks[3] = 0x1A;
			g_ROM.ROM_banks[4] = disk_num;
		}
	}
	else if( !_strncmp( (char*)g_ROM.header.id, "FDS", 3) && ( g_ROM.header.ctrl_z == 0x1A) )
	{
//		if(!g_NESConfig.preferences.DisableIPSPatch)
//			LoadIPSPatch((unsigned char *)p, (char *)fn);		//IPS PATCH
		uint8 disk_num;

		image_type = 1;
		disk_num = g_ROM.header.num_16k_rom_banks;

		g_ROM.header.id[0] = 'N';
		g_ROM.header.id[1] = 'E';
		g_ROM.header.id[2] = 'S';
		g_ROM.header.num_16k_rom_banks *= 4;
		g_ROM.header.num_8k_vrom_banks = 0;
		g_ROM.header.flags_1 = 0x40;
		g_ROM.header.flags_2 = 0x10;
		g_ROM.header.reserved[0] = g_ROM.header.reserved[1] = g_ROM.header.reserved[2] = g_ROM.header.reserved[3] = 0;
		g_ROM.header.reserved[4] = g_ROM.header.reserved[5] = g_ROM.header.reserved[6] = g_ROM.header.reserved[7] = 0;

		if( p + 65500 * disk_num - buf > filesize ){
			DEBUG("Error reading FDS Image");
			return NULL;
		}
		CopyMemIncPtr( g_ROM.ROM_banks + 16, p, 65500 * disk_num );

		g_ROM.ROM_banks[0] = 'F';
		g_ROM.ROM_banks[1] = 'D';
		g_ROM.ROM_banks[2] = 'S';
		g_ROM.ROM_banks[3] = 0x1A;
		g_ROM.ROM_banks[4] = disk_num;
	}
	else if(g_ROM.header.id[0] <= 0x1A && g_ROM.header.id[1] == 0x00 && g_ROM.header.num_8k_vrom_banks == 0x00)
	{
		uint8 fam[6];
		uint8 disk_num;
		image_type = 1;
		fam[0] = g_ROM.header.id[0];
		fam[1] = g_ROM.header.id[1];
		fam[2] = g_ROM.header.id[2];
		fam[3] = g_ROM.header.ctrl_z;
		fam[4] = g_ROM.header.num_16k_rom_banks;
		fam[5] = g_ROM.header.num_8k_vrom_banks;

		p = 6 + buf;

		while(!((fam[0] == 0x13 || fam[0] == 0x1A) && fam[1] == 0x00))
		{
			if(p + (uint32)fam[2]+((uint32)fam[3]<<8)+((uint32)fam[4]<<16)-6 - buf > filesize) {
				DEBUG("Error reading FAM image");
				return NULL;
			}
			p += (uint32)fam[2]+((uint32)fam[3]<<8)+((uint32)fam[4]<<16)-6;
			if(p + 6 - buf > filesize) {
				DEBUG("Error reading FAM image");
				return NULL;
			}
			CopyMemIncPtr( fam, p, 6 );
		}

		disk_num = (uint8)(((uint32)fam[2]+((uint32)fam[3]<<8)+((uint32)fam[4]<<16))/65500);

		g_ROM.header.id[0] = 'N';
		g_ROM.header.id[1] = 'E';
		g_ROM.header.id[2] = 'S';
		g_ROM.header.num_16k_rom_banks = disk_num*4;
		g_ROM.header.num_8k_vrom_banks = 0;
		g_ROM.header.flags_1 = 0x40;
		g_ROM.header.flags_2 = 0x10;
		g_ROM.header.reserved[0] = g_ROM.header.reserved[1] = g_ROM.header.reserved[2] = g_ROM.header.reserved[3] = 0;
		g_ROM.header.reserved[4] = g_ROM.header.reserved[5] = g_ROM.header.reserved[6] = g_ROM.header.reserved[7] = 0;

		if(fam[0] == 0x1A){
			if( p + 16 - buf > filesize ) {
				DEBUG("Error reading FAM image");
				return NULL;
			}
			p += 16;
		}

		if( p + 65500 * disk_num - buf > filesize ) {
			DEBUG("Error reading FAM image");
			return NULL;
		}
		CopyMemIncPtr( g_ROM.ROM_banks + 16, p, 65500 * disk_num );

		g_ROM.ROM_banks[0] = 'F';
		g_ROM.ROM_banks[1] = 'D';
		g_ROM.ROM_banks[2] = 'S';
		g_ROM.ROM_banks[3] = 0x1A;
		g_ROM.ROM_banks[4] = disk_num;
	}
	else if(!_strncmp((const char*)g_ROM.header.id, "NES", 3) && (g_ROM.header.ctrl_z == 'M'))
	{
		image_type = 2;

		if( filesize > 0x40000 ){
			DEBUG("NSF file is over 256 KB");
			return NULL;
		}
		CopyMemIncPtr( g_ROM.ROM_banks + 0x10, p, filesize - ( p - buf ) );

		*(int*)g_ROM.ROM_banks = filesize;
		g_ROM.ROM_banks[0x4] = g_ROM.header.num_16k_rom_banks;
		g_ROM.ROM_banks[0x5] = g_ROM.header.num_8k_vrom_banks;
		g_ROM.ROM_banks[0x6] = g_ROM.header.flags_1;
		g_ROM.ROM_banks[0x7] = g_ROM.header.flags_2;
		g_ROM.ROM_banks[0x8] = g_ROM.header.reserved[0];
		g_ROM.ROM_banks[0x9] = g_ROM.header.reserved[1];
		g_ROM.ROM_banks[0xA] = g_ROM.header.reserved[2];
		g_ROM.ROM_banks[0xB] = g_ROM.header.reserved[3];
		g_ROM.ROM_banks[0xC] = g_ROM.header.reserved[4];
		g_ROM.ROM_banks[0xD] = g_ROM.header.reserved[5];
		g_ROM.ROM_banks[0xE] = g_ROM.header.reserved[6];
		g_ROM.ROM_banks[0xF] = g_ROM.header.reserved[7];

		g_ROM.header.id[0] = 'N';
		g_ROM.header.id[1] = 'E';
		g_ROM.header.id[2] = 'S';
		g_ROM.header.ctrl_z = 0x1A;
		g_ROM.header.num_16k_rom_banks = 1;
		g_ROM.header.num_8k_vrom_banks = 0;
		g_ROM.header.flags_1 = 0xC0;
		g_ROM.header.flags_2 = 0x00;
		g_ROM.header.reserved[0] = g_ROM.header.reserved[1] = g_ROM.header.reserved[2] = g_ROM.header.reserved[3] = 0;
		g_ROM.header.reserved[4] = g_ROM.header.reserved[5] = g_ROM.header.reserved[6] = g_ROM.header.reserved[7] = 0;
	}
	else if(!_strncmp((char*)g_ROM.header.id, "UNIF", 4)){		// UNIF
		int promcn=0, cromcn=0, promtsize=0, cromtsize=0, unif_pos=0x10;
		unsigned char *prommp[16], *crommp[16];
		unsigned int prommsize[16], crommsize[16], promcsize[16], cromcsize[16];
		const char *mapperstr[4]={ "BMC-NovelDiamond9999999" ,"BTL-MARIO1-MALEE" , "BMC-Supervision16in1","BMC-Super24in1"};
		g_ROM.header.num_16k_rom_banks = g_ROM.header.num_8k_vrom_banks = 0;
		g_ROM.header.flags_1 = g_ROM.header.flags_2 = 0;
		g_ROM.unif_psize_16k = g_ROM.unif_csize_8k =0;
		_memset(prommp, 0, sizeof(prommp));
		_memset(crommp, 0, sizeof(crommp));

		image_type = 3;
		g_ROM.unif_mapper = 0xFF;
		while(unif_pos < (filesize-0x10)){
			uint32 ch = *((uint32 *)&p[unif_pos]);
			uint32 chunksize = *((uint32 *)&p[unif_pos+4]);
			if(ch==0x5250414D){						//MAPR
				if(chunksize>=23 && !_memcmp(&p[unif_pos+8], mapperstr[0], 23)){
					g_ROM.unif_mapper = 1;
					_strcpy(g_ROM.GameTitle, mapperstr[0]);
				}
				else if(chunksize>=16 && !_memcmp(&p[unif_pos+8], mapperstr[1], 16)){
					g_ROM.unif_mapper = 2;
					_strcpy(g_ROM.GameTitle, mapperstr[1]);
				}
				else if(chunksize>=20 && !_memcmp(&p[unif_pos+8], mapperstr[2], 20)){
					g_ROM.unif_mapper = 3;
					_strcpy(g_ROM.GameTitle, mapperstr[2]);
				}
				else if(chunksize>=14 && !_memcmp(&p[unif_pos+8], mapperstr[3], 14)){
					g_ROM.unif_mapper = 4;
					_strcpy(g_ROM.GameTitle, mapperstr[3]);
				}
			}
			else if(ch==0x5252494D){				//MIRR
			}
			else if(ch==0x52544142){				//BATR
			}
			else if((ch&0x00ffffff)==0x00475250){	//PRG?
				int msize, pagen;
				pagen = (ch>>24)-'0';
				if(chunksize<0x4000){
					msize=0x4000;
				}
				else{
					msize=chunksize&0xFFFFC000;
					if(chunksize&0x3FFF)
						msize+=0x4000;
				}
				g_ROM.unif_psize_16k += (unsigned char)(msize/0x4000);
				prommp[pagen] = &p[unif_pos+8];
				promcsize[pagen] = chunksize;
				prommsize[pagen] = msize;
				promtsize += msize;
			}
			else if((ch&0x00ffffff)==0x00524843){	//CHR?
				int msize, pagen;
				pagen = (ch>>24)-'0';
				if(chunksize<0x2000){
					msize=0x2000;
				}
				else{
					msize=chunksize&0xFFFFE000;
					if(chunksize&0x1FFF)
						msize+=0x2000;
				}
				g_ROM.unif_csize_8k += (unsigned char)(msize/0x2000);
				crommp[pagen] = &p[unif_pos+8];
				cromcsize[pagen] = chunksize;
				crommsize[pagen] = msize;
				cromtsize += msize;
			}
			unif_pos += (chunksize + 8);
		}
		{
			int i, tsize;

			for(tsize=0, i=0; prommp[i]&&i<16; i++){
				_memcpy(&g_ROM.ROM_banks[tsize], prommp[i], promcsize[i]);
				tsize+=prommsize[i];
			}
			for(tsize=0, i=0; crommp[i]&&i<16; i++){
				_memcpy(&g_ROM.VROM_banks[tsize], crommp[i], cromcsize[i]);
				tsize+=crommsize[i];
			}
		}
	}
	else
	{
		DEBUG("Unsupported File");
		return NULL;
	}

	//	uint32 j;
	g_ROM.screen_mode = 1;

	// figure out g_ROM.mapper number
	g_ROM.mapper = ( g_ROM.header.flags_1 >> 4);

	// if there is anything in the reserved bytes,
	// don't trust the high nybble of the mapper number
	//	for( i = 0; i < sizeof(g_ROM.header.reserved); i++ )
	//	{
	//		if(g_ROM.header.reserved[i] != 0x00) throw "Invalid NES g_ROM.header ($8-$F)";
	//	}
	g_ROM.mapper |= ( g_ROM.header.flags_2 & 0xF0 );

	g_ROM.dbcorrect[0]=0;
//	if(g_ROM.unif_mapper)
//		return;

	if(image_type == 1)
	{
		int i;
		//	g_ROM.screen_mode = 1;
		g_ROM.mapper = 20;

		g_ROM.fds = (g_ROM.ROM_banks[0x1f] << 24) | (g_ROM.ROM_banks[0x20] << 16) |
		      (g_ROM.ROM_banks[0x21] <<  8) | (g_ROM.ROM_banks[0x22] <<  0);
		for(i = 0; i < g_ROM.ROM_banks[4]; i++)
		{
			uint8 file_num = 0;
			uint32 pt = 16+65500*i+0x3a;
			while(g_ROM.ROM_banks[pt] == 0x03)
			{
				pt += 0x0d;
				pt += g_ROM.ROM_banks[pt] + g_ROM.ROM_banks[pt+1] * 256 + 4;
				file_num++;
			}
			g_ROM.ROM_banks[16+65500*i+0x39] = file_num;
		}
	}
	else if(image_type == 2)
	{
		//    g_ROM.screen_mode = 1;
		g_ROM.mapper = 12; // 12 is private g_ROM.mapper number
	}
	else if(image_type == 0)
	{
//		g_ROM.crc = CrcCalc(g_ROM.ROM_banks, g_ROM.header.num_16k_rom_banks * 0x4000);
		g_ROM.crc = CrcCalca(g_ROM.ROM_banks, g_ROM.header.num_16k_rom_banks * 0x4000, g_ROM.crc);
		g_ROM.crc_all = CrcCalca(g_ROM.VROM_banks, g_ROM.header.num_8k_vrom_banks * 0x2000, g_ROM.crc);
#if 1
		// get rom info from DataBase, and apply.
		if(g_NESConfig.preferences.UseRomDataBase)
		{
			HANDLE hFile;
			uint32 c, pt, i, db=2;
			char fn2[256], buf[512];
			unsigned char theader[2], dbflag=0;
			while(db){
				GetModulePath(fn2, sizeof(fn2));
				if(db==2){
					// JAPANESE
					_strcat(fn2, "famicom.dat");
				}
				else{
					// ENGLISH
					_strcat(fn2, "nesdbase.dat");
				}
				hFile = NES_fopen(fn2, FILE_MODE_READ);
				if(hFile > 0){
					while(NES_fgets(buf, sizeof(buf), hFile)){
						pt = 0;
						// All CRC
						for(;buf[pt] != ';' && buf[pt]!='\0';pt++);
						if(buf[pt]=='\0')
							continue;
						pt++;
						// PROM CRC
						c = 0;
						{
							char buf2[16];
							for(i = 0; i < 8 && buf[pt] != ';' && buf[pt]; i++, pt++){
								buf2[i] = buf[pt];
							}
							if(buf[pt]=='\0')
								continue;
							++pt;
							buf2[i] = 0;
							c = _atoh(buf2);
						}
						if(g_ROM.crc == c && g_ROM.crc != 0 && c != 0){
							// Title
							for(i=0; buf[pt] != ';' && buf[pt]!='\0'; ++i, ++pt){
								g_ROM.GameTitle[i] = buf[pt];
							}
							g_ROM.GameTitle[i]=0;
							//MystrFnIcut(GameTitle);
							pt++;
							char buf2[16];
							// Header 1
							i = 0;
							while(buf[pt] != ';') buf2[i++] = buf[pt++];
							pt++;
							buf2[i] = '\0';
							//						g_ROM.header.flags_1 = atoi(buf2);
							theader[0] = _atoi(buf2);
							// Header 2
							i = 0;
							while(buf[pt] != ';') buf2[i++] = buf[pt++];
							pt++;
							buf2[i] = '\0';
							//						header.flags_2 = atoi(buf2);
							theader[1] = _atoi(buf2);
							// PROM Size
							while(buf[pt] != ';') pt++;
							pt++;
							// CROM Size
							while(buf[pt] != ';') pt++;
							pt++;
							// Country
							if(/*buf[pt] == 'A' ||*/ buf[pt] == 'E' || (buf[pt] == 'P' && buf[pt+1] == 'D') || buf[pt] == 'S')
							{
								// Asia, Europe, PD, Swedish
								g_ROM.screen_mode = 2;
							}
							/*						else if(buf[pt] == 'J' || buf[pt] == 'U' || buf[pt] == 'V'){
														screen_mode = 1;
													}*/
							//						mapper = (header.flags_1 >> 4) | (header.flags_2 & 0xF0);
							for(i = 0; i < 8; i++) g_ROM.header.reserved[i] = 0;
							//						fseek(fp2, 0, SEEK_END);
							dbflag=1;
							break;
						}
					}
					NES_fclose(hFile);
				}
				if(dbflag)
					break;
				--db;
			}

			if(dbflag){
				if(theader[0]!=g_ROM.header.flags_1 || ((theader[1]&0xf0)!=(g_ROM.header.flags_2&0xf0))){
//					SHOW_ERRMSG(0, "DB HIT! Applying,,,", 0xFFFF, 30);
/*					PSPでは確認しません
					if(NESTER_settings.nes.preferences.AutoRomCorrect){
						char str2[64], str3[64];
						LoadString(g_main_instance, IDS_STRING_LMSG_05 , str2, 64);
						LoadString(g_main_instance, IDS_STRING_MSGB_10 , str3, 64);
						if(IDYES== MessageBox(main_window_handle,(LPCSTR)str3,(LPCSTR)str2, MB_YESNO))
						{
							dbcorrect[0]=1, dbcorrect[1]= header.flags_1, dbcorrect[2]= header.flags_2;
							header.flags_1 = theader[0];
							header.flags_2 = theader[1];
							mapper = (header.flags_1 >> 4) | (header.flags_2 & 0xF0);
						}
					}
					else{
*/
					g_ROM.dbcorrect[0]=1, g_ROM.dbcorrect[1]= g_ROM.header.flags_1, g_ROM.dbcorrect[2]= g_ROM.header.flags_2;
					g_ROM.header.flags_1 = theader[0];
					g_ROM.header.flags_2 = theader[1];
					g_ROM.mapper = (g_ROM.header.flags_1 >> 4) | (g_ROM.header.flags_2 & 0xF0);
				}
			}
		}
#endif
	}
	#include "nes_rom_correct.cpp"
	return &g_ROM;
}

mirroring_type NES_ROM_get_mirroring()
{
	if(g_ROM.header.flags_1 & MASK_4SCREEN_MIRRORING)
	{
		return NES_PPU_MIRROR_FOUR_SCREEN;
	}
	else if(g_ROM.header.flags_1 & MASK_VERTICAL_MIRRORING)
	{
		return NES_PPU_MIRROR_VERT;
	}
	else
	{
		return NES_PPU_MIRROR_HORIZ;
	}
}


uint8 NES_ROM_get_mapper_num() { return g_ROM.mapper; }
uint8 NES_ROM_get_unifmapper_num() { return g_ROM.unif_mapper; }

boolean  NES_ROM_has_save_RAM()   { return g_ROM.header.flags_1 & MASK_HAS_SAVE_RAM; }
boolean  NES_ROM_has_trainer()    { return g_ROM.header.flags_1 & MASK_HAS_TRAINER;  }
boolean  NES_ROM_is_VSUnisystem() { return g_ROM.header.flags_2 & 0x01;              }

uint8 NES_ROM_get_num_16k_ROM_banks() { return g_ROM.header.num_16k_rom_banks; }
uint8 NES_ROM_get_num_8k_VROM_banks() { return g_ROM.header.num_8k_vrom_banks; }

uint8* NES_ROM_get_trainer()    { return g_ROM.trainer;     }
uint8* NES_ROM_get_ROM_banks()  { return g_ROM.ROM_banks;   }
uint8* NES_ROM_get_VROM_banks() { return g_ROM.VROM_banks;  }

const char* NES_ROM_GetRomName() { return g_ROM.rom_name; }
const char* NES_ROM_GetRomPath() { return g_ROM.rom_path; }

uint8 NES_ROM_get_screen_mode() { return g_ROM.screen_mode; }
char *NES_ROM_get_GameTitleName(){return g_ROM.GameTitle; }

uint32 NES_ROM_crc32()       { return g_ROM.crc;  }
uint32 NES_ROM_crc32_all()       { return g_ROM.crc_all;  }
uint32 NES_ROM_fds_id()	{ return g_ROM.fds; }

// for Best Play - Pro Yakyuu Special
uint32 NES_ROM_get_size_SaveRAM() { return g_ROM.size_SaveRAM; }

uint32 NES_ROM_get_UNIF_psize_16k() { return g_ROM.unif_psize_16k;  }
uint32 NES_ROM_get_UNIF_csize_8k() { return g_ROM.unif_csize_8k;  }


void NES_ROM_GetPathInfo(const char* fn)
{
	_strcpy( g_ROM.rom_path, fn );
	NES_PathRemoveFileSpec( g_ROM.rom_path );
	NES_PathAddBackslash( g_ROM.rom_path );

	_strcpy( g_ROM.rom_name, PathFindFileName( fn ) );
	NES_PathRemoveExtension( g_ROM.rom_name );
}


void NES_ROM_GetROMInfoStr(char *h){
	char headerflag[5],headerflag2[5];
	int i, j;
	unsigned char *th = (unsigned char *)&g_ROM.header;
//	h[0x10]= g_ROM.dbcorrect[0], h[0x11]= g_ROM.dbcorrect[1], h[0x12]= g_ROM.dbcorrect[2];
	for(i=0,j=1; i<4; ++i, j<<=1){
		if(th[6] & j)
			headerflag[i]='1';
		else
			headerflag[i]='0';
		if(th[0x11] & j)
			headerflag2[i]='1';
		else
			headerflag2[i]='0';
	}
	headerflag[4]=0, headerflag2[4]=0;
/*	if(g_ROM.dbcorrect[0]){
		wsprintf(h, " Mapper [ %u -> %u ], PROM %uKB, CROM %uKB, FLAG[ %s -> %s ], PROM g_ROM.crc 0x%08X, ROM g_ROM.crc 0x%08X", (g_ROM.dbcorrect[1] >> 4)|(g_ROM.dbcorrect[2] & 0xF0),(th[6] >> 4)|(th[7] & 0xF0), th[4]*16, th[5]*8, headerflag2, headerflag, g_ROM.crc, g_ROM.crc_all);
	}
	else{
		wsprintf(h, " Mapper [ %u ], PROM %uKB, CROM %uKB, FLAG %s, PROM g_ROM.crc 0x%08X, ROM g_ROM.crc 0x%08X", (th[6] >> 4)|(th[7] & 0xF0), th[4]*16, th[5]*8, headerflag, g_ROM.crc, g_ROM.crc_all);
	}*/
}
