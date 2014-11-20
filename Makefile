OBJS = audio.o usf.o cpu.o ops.o memory.o audio_ucode1.o audio_ucode2.o audio_ucode3.o audio_ucode3mp3.o audio_hle_main.o psftag.o cpu_hle.o os.o audiolib.o

CFLAGS = -c -w -m32
# -fcommon -ffunction-cse -fexpensive-optimizations -ffast-math -floop-optimize -msse
GCC = gcc
GPP = g++
LD = g++

OPTS = -O3 -funroll-loops -march=pentium3 -mtune=pentium3

josh-usf : $(OBJS)
	$(LD) -o josh-usf -m32 $^

josh-usf-win32 : $(OBJS)
	$(LD) -o josh-usf $^ -lwinmm

.c.o:
	$(GCC) $(CFLAGS) $(OPTS) $*.c

.cpp.o:
	$(GPP) $(CFLAGS) $(OPTS) $*.cpp

clean:
	rm -f $(OBJS) > /dev/null

all: josh-usf

win32: josh-usf-win32
