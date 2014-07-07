OUTDIR=
PRC=Warfare.prc
CC = m68k-palmos-gcc
CPP_DEFINES_GLOBAL=-fcheck-new -D__CPU_68K
CPP_INCS=-I ../inc -I palm
TOOLDIR=../bin
TARGETDEFINED=0
PRC_VERSION=12
BUILD-PRC-FLAGS=

ifdef MAP
MAPFLAGS=-Xlinker -Map -Xlinker ht.map
endif

# Debug is default

ifdef REL
CPP_DEBUG=
else
CPP_DEBUG=-DDEBUG -g
BUILD-PRC-FLAGS=
endif

ifdef ESD_BUILD
ifdef MERGE
BUILD-PRC-FLAGS=--backup --copy-prevention --version-number $(PRC_VERSION)
else
BUILD-PRC-FLAGS=--backup --version-number $(PRC_VERSION)
endif
CPP_DEFINES=$(CPP_DEFINES_GLOBAL) -DESD_BUILD
ifdef REL
OUTDIR=Palm_ESD_Release
else
OUTDIR=Palm_ESD_Debug
endif
TARGETDEFINED=1
endif

ifdef RETAIL_BUILD
BUILD-PRC-FLAGS=--backup --copy-prevention --version-number $(PRC_VERSION)
CPP_DEFINES=$(CPP_DEFINES_GLOBAL) -DRETAIL_BUILD
ifdef REL
OUTDIR=Palm_Retail_Release
else
OUTDIR=Palm_Retail_Debug
endif
TARGETDEFINED=1
endif

ifdef PROFILE
CPP_DEFINES=$(CPP_DEFINES_GLOBAL) -DDEV_BUILD
CPP_DEBUG=-mdebug-labels
OUTDIR=Palm_Profile
TARGETDEFINED=1
endif

# DEV_BUILD is default

ifeq ($(TARGETDEFINED),0)
DEV_BUILD=1
endif

ifdef DEV_BUILD
CPP_DEFINES=$(CPP_DEFINES_GLOBAL) -DDEV_BUILD
ifdef REL
OUTDIR=Palm_Release
else
OUTDIR=Palm_Debug
endif
TARGETDEFINED=1
endif

# Composite the flags together

CFLAGS=-fvtable-thunks -Os -DPIL $(CPP_DEFINES) $(CPP_DEBUG) $(CPP_INCS) -fno-rtti -fno-exceptions
CFLAGSARM=-Os -DPIL $(CPP_DEFINES) $(CPP_INCS) -fno-rtti -fno-exceptions

OBJS=game.o main.o host.o misc.o bitmap.o packfile.o rip.o form.o ini.o \
misccontrols.o tests.o timer.o event.o font.o ht-sections.o 68k.o display.o mqdisplay.o \
GameObjects.o Level.o Simulation.o tilemap.o StateMachine.o Animation.o \
fogmap.o SRInfantry.o LRInfantry.o terrainmap.o HRC.o \
Reactor.o Processor.o Miner.o Headquarters.o Research.o Radar.o VTS.o \
MobileBuilder.o Tank.o SpInfantry.o Overmind.o Builder.o MobileUnit.o \
memmgr.o tbitmap.o tbltcode.o compression.o decompress.o \
cachemgr.o copybits.o Multiplayer.o comm.o Player.o \
TransportMgr.o Warehouse.o Tower.o MobileHQ.o Unit.o Struct.o SimUI.o \
pwm2.o soundmgr.o pwmsounddev.o sounddev.o pa1lib.o pa1sounddev.o pa1encode.o \
os5sounddev.o updatemap.o mapdraw824.o mapdraw816.o updatescreen824.o updatescreen816.o blt.o \
GameOptions.o InputUI.o formmgr.o updatescreen416.o fill.o thunks.o armthunks.o loadsave.o savegame.o \
triggermgr.o TriggerConditions.o TriggerActions.o UnitGroupMgr.o BuildMgr.o \
RawBitmap.o Shell.o Ecom.o alertcontrol.o stringtable.o selection.o Andy.o \
Artillery.o Replicator.o Help.o CutScene.o drm.o PalmPins.o twdisplay.o

all:
	make $(OUTDIR)/$(PRC)

clean:
	rm -f $(OUTDIR)/*

$(OUTDIR)/$(PRC): $(OUTDIR)/ht palm/ht.def palm/ht.rcp $(OUTDIR)/tAIB03e8.bin $(OUTDIR)/armcode
	cp -f palm/ht.def $(OUTDIR)/ht.def
	cd $(OUTDIR) && build-prc $(BUILD-PRC-FLAGS) -o $(PRC) ht.def ht armcode *.bin && cp $(PRC) "Warfare Incorporated.prc" && cd ..
ifdef MERGE
	cd $(OUTDIR) && ../$(TOOLDIR)/packpdb2 -m foo.prc $(PRC) ..\\$(MERGE) ..\\htsfx.pdb && mv foo.prc "Warfare Incorporated_merged.prc" && cd ..
endif

$(OUTDIR)/tAIB03e8.bin: palm/ht.rcp palm/resource.h
	rm -f $(OUTDIR)/*.bin
ifdef MERGE
	echo $(MERGE) > $(OUTDIR)/FILR0000.bin
endif
	pilrc $(CPP_INCS) -V palm/ht.rcp $(OUTDIR)

$(OUTDIR)/ht-sections.o: $(OUTDIR)/ht-sections.s
	$(CC) -c $(OUTDIR)/ht-sections.s -o $(OUTDIR)/ht-sections.o

$(OUTDIR)/ht-sections.s $(OUTDIR)/ht-sections.ld: palm/ht.def
	cd $(OUTDIR) && m68k-palmos-multigen ../palm/ht.def

$(OUTDIR)/ht: $(OBJS:%=$(OUTDIR)/%) $(OUTDIR)/ht-sections.ld
	$(CC) $(CFLAGS) -lnoexcept -o $(OUTDIR)/ht $(OBJS:%=$(OUTDIR)/%) libgcc.a $(OUTDIR)/ht-sections.ld $(MAPFLAGS)

$(OUTDIR)/%.o: palm/%.s
	$(CC) -c palm/$(*F).s -o $@

$(OUTDIR)/packfile.o: packfile.cpp packfile.h
	$(CC) $(CFLAGS) -c packfile.cpp -o $(OUTDIR)/packfile.o

$(OUTDIR)/misc.o: misc.cpp ht.h res.h palm/htplatform.h license.h
	$(CC) $(CFLAGS) -c misc.cpp -o $(OUTDIR)/misc.o

$(OUTDIR)/%.o: %.cpp ht.h res.h palm/htplatform.h
	$(CC) $(CFLAGS) -c $(*F).cpp -o $@

$(OUTDIR)/armcode: palm/armcode.cpp palm/arm.h res.h miscgraphics.cpp
	arm-palmos-gcc $(CFLAGSARM) -c palm/armcode.cpp -o $(OUTDIR)/armcode.o
	arm-palmos-gcc -nostartfiles -o $(OUTDIR)/armcode $(OUTDIR)/armcode.o

$(OUTDIR)/%.o: palm/%.cpp ht.h res.h palm/htplatform.h
	$(CC) $(CFLAGS) -c palm/$(*F).cpp -o $@

$(OUTDIR)/%.o: palm/%.c ht.h res.h palm/htplatform.h
	$(CC) $(CFLAGS) -c palm/$(*F).c -o $@

$(OUTDIR)/rip.o: ../inc/rip.cpp
	$(CC) $(CFLAGS) -c ../inc/rip.cpp -o $(OUTDIR)/rip.o


