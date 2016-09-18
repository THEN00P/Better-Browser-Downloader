TITLE_ID = VPKMIRROR
TARGET = VPKMirror
OBJS   = src/main.o src/font.o src/graphics.o src/init.o src/net.o src/io_process.o \
	src/package_installer.o src/archive.o src/file.o src/message_dialog.o \
	src/config.o src/utils.o src/sha1.o minizip/unzip.o minizip/ioapi.o \
	src/bm.o src/sfo.o src/vita_sqlite.o sqlite-3.6.23.1/sqlite3.o

LIBS = -lvita2d -lScePgf_stub -lSceDisplay_stub -lSceGxm_stub -lSceSysmodule_stub -lSceNet_stub \
	-lSceCtrl_stub -lSceNetCtl_stub -lSceHttp_stub -lSceAppMgr_stub -lSceAppUtil_stub \
	-lSceCommonDialog_stub -lScePower_stub libpromoter/libScePromoterUtil_stub.a -lz

DEFINES = -DSQLITE_OS_OTHER=1 -DSQLITE_TEMP_STORE=3 -DSQLITE_THREADSAFE=0

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CFLAGS  = -Wl,-q -O3 $(DEFINES)
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: eboot.bin
	vita-mksfoex -s TITLE_ID=$(TITLE_ID) "VPK Mirror" param.sfo
	vita-pack-vpk -s param.sfo -b eboot.bin -a res/icon0.png=sce_sys/icon0.png $@

eboot.bin: $(TARGET).velf
	vita-make-fself $< eboot.bin

$(TARGET).velf: $(TARGET).elf
	vita-elf-create $< $@ libpromoter/promoterutil.json

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf *.velf *.elf *.vpk $(OBJS) param.sfo eboot.bin

vpksend: $(TARGET).vpk
	curl -T $(TARGET).vpk ftp://$(PSVITAIP):1337/ux0:/
	@echo "Sent."

send: eboot.bin
	curl -T eboot.bin ftp://$(PSVITAIP):1337/ux0:/app/$(TITLE_ID)/
	@echo "Sent."
