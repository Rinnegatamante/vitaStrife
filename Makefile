TARGET		:= vitaStrife
SOURCES	:= source
INCLUDES	:= include

LIBS = -lSDL_mixer -lSDL -lvorbisfile -lvorbis -logg -lvita2d -lSceLibKernel_stub -lScePvf_stub \
	-lSceHid_stub -lSceJpegEnc_stub -lSceAppMgr_stub -lSceCtrl_stub -lScePgf_stub \
	-lScePromoterUtil_stub -lm -lSceNet_stub -lSceNetCtl_stub -lSceAppUtil_stub \
	-ljpeg -lfreetype -lc -lScePower_stub -lSceCommonDialog_stub -lpng16 -lz \
	-lSceAudio_stub -lSceGxm_stub -lSceDisplay_stub -lSceSysmodule_stub

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX     = $(PREFIX)-g++
CFLAGS  = -fno-lto -g -Wl,-q -O3 -ffast-math -fno-unit-at-a-time -fdiagnostics-show-option \
		-I$(VITASDK)/$(PREFIX)/include/SDL
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11 -fpermissive
ASFLAGS = $(CFLAGS)

all: $(TARGET).velf

%.velf: %.elf
	cp $< $<.unstripped.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@
	vita-make-fself -a 0x2800000000000001 $@ eboot_unsafe.bin
	vita-make-fself -s $@ eboot_safe.bin

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS)
