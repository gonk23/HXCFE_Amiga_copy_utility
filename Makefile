CC=bin/m68k-amigaos-gcc
AS=bin/m68k-amigaos-as
CFLAGS= -O3 -I . -noixemul -I ./fat32 -m68000
LDFLAGS=-s  -Wl,-Map,foo.map   -noixemul  -amiga-debug-hunk -m68000
EXEC=hxcfe

all: $(EXEC)

$(EXEC): hxcfe.o amiga_hw.o crc.o fat_access.o fat_filelib.o fat_misc.o fat_string.o fat_table.o fat_write.o fat_cache.o
	$(CC) -o $@ $^ $(LDFLAGS)

hxcfe.o: hxcfe.c
	$(CC) -o $@ -c $< $(CFLAGS)

amiga_hw.o: amiga_hw.c
	$(CC) -o $@ -c $< $(CFLAGS)

crc.o: crc.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_access.o: ./fat32/fat_access.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_filelib.o: ./fat32/fat_filelib.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_misc.o: ./fat32/fat_misc.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_string.o: ./fat32/fat_string.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_table.o: ./fat32/fat_table.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_write.o: ./fat32/fat_write.c
	$(CC) -o $@ -c $< $(CFLAGS)

fat_cache.o: ./fat32/fat_cache.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf $(EXEC)

.PHONY: clean
