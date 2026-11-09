# Chrono Crystal - GBDK-2020 build
# ใช้ MBC5 + SRAM + battery สำหรับระบบเซฟ (Phase 6)

GBDK_HOME ?= C:/gbdk
LCC = $(GBDK_HOME)/bin/lcc

LCCFLAGS = -Wa-l -Wl-m -Wl-j -Wl-yt0x1B -Wm-ya1
SRC = main.c assets.c screen.c player.c map.c battle.c sfx.c save.c
HDR = game.h assets.h screen.h player.h map.h battle.h sfx.h save.h

all: game.gb

game.gb: $(SRC) $(HDR)
	$(LCC) $(LCCFLAGS) -o $@ $(SRC)

clean:
	rm -f game.gb *.o *.lst *.map *.sym *.asm *.rel *.ihx *.noi *.cdb *.rst

.PHONY: all clean
