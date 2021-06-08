BINARY = out

# モジュールの配列順が重要なので注意のこと。最初のほうにあるとgp相対が使える。 - LCK
OBJECTS = nes/cpu/nes6502.o nes/cpu/nes_6502.o nes/ppu/nes_ppu.o \
	  nes/apu/fdssnd.o nes/apu/nes_apu.o nes/apu/nes_apu_wrapper.o nes/apu/nes_exsound.o nes/apu/nes_fme7.o\
          nes/apu/nes_mmc5.o nes/apu/nes_n106.o nes/apu/nes_vrc6.o nes/apu/nes_vrc7.o\
	  nes/nes.o nes/nes_config.o nes/nes_mapper.o nes/nes_rom.o nes/snss.o\
          nes/fileio.o nes/nes_crc32.o nes/libsnss/libsnss.o emu_main.o loadromimage.o\
	  soundmanager.o screenmanager.o inputmanager.o\
	  startup.o main.o pg.o menu.o filer.o sound.o debug/debug.o string.o menu_submenu.o
LIBRARY = unzip/unziplib.a

all: $(BINARY)

$(BINARY): $(OBJECTS)
	ee-ld -s -O3 $(OBJECTS) $(LIBRARY) -M -Ttext 8900000 -q -g -o $@ > NesterJ.map
	outpatch USERPROG
	elf2pbp outp "uoNesterJ NES Emulator 2b" ICON0.PNG


%.o : %.c
	ee-gcc -march=r4000 -O3 -fomit-frame-pointer -g -mgp32 -mlong32 -c $< -o $@

%.o : %.s
	ee-gcc -march=r4000 -g -mgp32 -c -xassembler -O -o $@ $<

clean:
	del /s /f *.o *.map
