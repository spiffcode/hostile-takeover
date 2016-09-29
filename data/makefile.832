MONO=mono

UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
INICRUNCH=inicrunch_osx
CPP=cpp -P -Wno-invalid-pp-token -Wno-trigraphs
endif
ifeq ($(UNAME), Linux)
INICRUNCH=inicrunch_linux
CPP=cpp -P -Wno-trigraphs
endif

HELPFILE=help.txt

ifdef IPHONE
TILESIZE=32
ifdef MULTIPLAYER
DEFINI=-DIPHONE -DMULTIPLAYER
HELPFILE=helpiphone-multiplayer.txt
else
DEFINI=-DIPHONE
HELPFILE=helpiphone.txt
endif
endif

ifdef DEV_BUILD
DEFDEV=-DDEV_BUILD=1
endif

ifndef DEPTH
DEPTH=8
endif

ifeq ($(DEPTH),8)
PAL_SIZE=256
FIXED_PAL_SIZE=131
endif

ifeq ($(DEPTH),4)
PAL_SIZE=16
FIXED_PAL_SIZE=16
endif

ifdef TS
TILESIZE=$(TS)
endif

ifndef TILESIZE
TILESIZE=16
endif

OUTDIR:=$(DEPTH)$(TILESIZE)
ARTDIR:=art$(DEPTH)$(TILESIZE)
PALDIR:=art$(DEPTH)$(TILESIZE)

ifeq ($(DEPTH),4)
ARTDIR:=art8$(TILESIZE)
endif
DT:=$(DEPTH)$(TILESIZE)

# Hack to support 240x320 mode which blends hires and custom res elements
# NOTE: all assignments using TILESIZE above this point must be of the ':='
# variety.
# NOTE: must rm -rf art820 first since none of it is actually "checked in"

ifeq ($(TILESIZE),20)
override TILESIZE=24
ifdef QUICK
COPYART=xcopy art824 art820 /Q /Y /R /D /E /I /EXCLUDE:excludecvs.txt && xcopy art820.true art820 /Q /Y /R /E /I /EXCLUDE:excludecvs.txt
else
COPYART=@rm -rf art820 && mkdir art820 && xcopy art824 art820 /Q /Y /R /D /E /I /EXCLUDE:excludecvs.txt && xcopy art820.true art820 /Q /Y /R /E /I /EXCLUDE:excludecvs.txt
endif
endif

# Hack to support iPhone's 32 tile size. Use all the art from 24, but scale
# tiles and animations.

ifeq ($(TILESIZE),32)
ARTDIR:=art$(DEPTH)24
PALDIR:=art$(DEPTH)24
AMX_SCALEARG:=-scale 1.3333333333333 -noscaleicon
TILE_SCALEARG:=-scale 1.3333333333333
FORMBMP_SCALEARG:=-scale 1.0
MAPBMP_SCALEARG:=-scale 1.3333333333333
RAWBMP_SCALEARG:=-scale 1.0
endif


#
# Palette settings
#

PRELOAD_PAL=preload_$(DEPTH)bpp.pal
COMMON_PAL=$(ARTDIR)/common_$(DEPTH)bpp.pal
SHELL_PAL=$(ARTDIR)/shell_$(DEPTH)bpp.pal
FIXED_PAL=$(PALDIR)/fixed_$(DEPTH)bpp.pal
CUTSCENE_PAL=$(PALDIR)/cutscene_$(DEPTH)bpp.pal
DESERT_PAL=$(PALDIR)/desert_$(DEPTH)bpp.pal
GRASSY_PAL=$(PALDIR)/grassy_$(DEPTH)bpp.pal

#
# Palette space for background template
#

ifeq ($(DEPTH),4)
DESERT_BACKGROUND_PAL_SIZE=0
GRASSY_BACKGROUND_PAL_SIZE=0
endif

ifeq ($(DEPTH),8)
DESERT_BACKGROUND_PAL_SIZE=32
GRASSY_BACKGROUND_PAL_SIZE=32
endif

#
# Global defines
#

CPP_INCS=-I../game
TOOLDIR=../bin
PDB=htdata$(DT).pdb
CREATORID=WARI

#
# amx categorization
#

FIXED_AMXS=$(filter-out $(CUTSCENE_AMXS),$(wildcard $(ARTDIR)/*.amx))

#
# Non-amx image categorization
#

NOTSRC_BMPS=
DESERT_BMPS=
GRASSY_BMPS=
FONT_BMPS=$(ARTDIR)/standardfont.bmp $(ARTDIR)/shadowfont.bmp $(ARTDIR)/buttonfont.bmp $(ARTDIR)/titlefont.bmp $(ARTDIR)/hudfont.bmp
CUTSCENE_RAW_BMPS=$(wildcard $(ARTDIR)/cutscenebitmaps/*.bmp)
SHELL_RAW_BMPS=$(wildcard $(ARTDIR)/shellrawbitmaps/*.bmp)
FIXED_RAW_BMPS=$(wildcard $(ARTDIR)/rawbitmaps/*.bmp)
FIXED_BMPS=$(filter-out $(DESERT_BMPS) $(GRASSY_BMPS) $(FONT_BMPS) $(NOTSRC_BMPS),$(wildcard $(ARTDIR)/bitmaps/*.bmp))

# Would be better to put these in their own directory if this scaling
# experiment works

FIXED_MAP_BMPS=\
$(ARTDIR)/bitmaps/arrow0.bmp \
$(ARTDIR)/bitmaps/arrow1.bmp \
$(ARTDIR)/bitmaps/arrow2.bmp \
$(ARTDIR)/bitmaps/arrow3.bmp \
$(ARTDIR)/bitmaps/arrow4.bmp \
$(ARTDIR)/bitmaps/arrow5.bmp \
$(ARTDIR)/bitmaps/arrow6.bmp \
$(ARTDIR)/bitmaps/arrow7.bmp \
$(ARTDIR)/bitmaps/fog0001.bmp \
$(ARTDIR)/bitmaps/fog0010.bmp \
$(ARTDIR)/bitmaps/fog0011.bmp \
$(ARTDIR)/bitmaps/fog0100.bmp \
$(ARTDIR)/bitmaps/fog0101.bmp \
$(ARTDIR)/bitmaps/fog0111.bmp \
$(ARTDIR)/bitmaps/fog1000.bmp \
$(ARTDIR)/bitmaps/fog1010.bmp \
$(ARTDIR)/bitmaps/fog1011.bmp \
$(ARTDIR)/bitmaps/fog1100.bmp \
$(ARTDIR)/bitmaps/fog1101.bmp \
$(ARTDIR)/bitmaps/fog1110.bmp \
$(ARTDIR)/bitmaps/fog1111.bmp \
$(ARTDIR)/bitmaps/galax1a.bmp \
$(ARTDIR)/bitmaps/galax1b.bmp \
$(ARTDIR)/bitmaps/galax1c.bmp \
$(ARTDIR)/bitmaps/galax2a.bmp \
$(ARTDIR)/bitmaps/galax2b.bmp \
$(ARTDIR)/bitmaps/galax2c.bmp \
$(ARTDIR)/bitmaps/galax3a.bmp \
$(ARTDIR)/bitmaps/galax3b.bmp \
$(ARTDIR)/bitmaps/galax3c.bmp \
$(ARTDIR)/bitmaps/placementbad.bmp \
$(ARTDIR)/bitmaps/placementgood.bmp \
$(ARTDIR)/bitmaps/plant.bmp \
$(ARTDIR)/bitmaps/plant1.bmp \
$(ARTDIR)/bitmaps/plant2.bmp \
$(ARTDIR)/bitmaps/plant3.bmp \
$(ARTDIR)/bitmaps/plant4.bmp \
$(ARTDIR)/bitmaps/plant5.bmp \
$(ARTDIR)/bitmaps/rocketartifact.bmp \
$(ARTDIR)/bitmaps/rocks.bmp \
$(ARTDIR)/bitmaps/tree.bmp \
$(ARTDIR)/bitmaps/tree1.bmp \
$(ARTDIR)/bitmaps/tree2.bmp \
$(ARTDIR)/bitmaps/tree3.bmp \
$(ARTDIR)/bitmaps/tree4.bmp \
$(ARTDIR)/bitmaps/tree5.bmp \
$(ARTDIR)/bitmaps/tree6.bmp \
$(ARTDIR)/bitmaps/tree7.bmp \
$(ARTDIR)/bitmaps/scorch_8x8.bmp \
$(ARTDIR)/bitmaps/scorch_16x16.bmp \
$(ARTDIR)/bitmaps/scorch_32x16.bmp \
$(ARTDIR)/bitmaps/scorch_48x48.bmp \
$(ARTDIR)/bitmaps/arrowheadleft.bmp \
$(ARTDIR)/bitmaps/arrowheadup.bmp \
$(ARTDIR)/bitmaps/arrowheadright.bmp \
$(ARTDIR)/bitmaps/arrowheaddown.bmp \
$(ARTDIR)/bitmaps/build_btn_up.bmp \
$(ARTDIR)/bitmaps/build_btn_down.bmp \
$(ARTDIR)/bitmaps/build_btn_disabled.bmp \
$(ARTDIR)/bitmaps/abort_build_btn_up.bmp \
$(ARTDIR)/bitmaps/abort_build_btn_down.bmp \
$(ARTDIR)/bitmaps/repair_btn_up.bmp \
$(ARTDIR)/bitmaps/repair_btn_down.bmp \
$(ARTDIR)/bitmaps/repair_btn_disabled.bmp \
$(ARTDIR)/bitmaps/abort_repair_btn_up.bmp \
$(ARTDIR)/bitmaps/abort_repair_btn_down.bmp \
$(ARTDIR)/bitmaps/sell_btn_up.bmp \
$(ARTDIR)/bitmaps/sell_btn_down.bmp \
$(ARTDIR)/bitmaps/sell_btn_disabled.bmp

FIXED_FORM_BMPS=$(filter-out $(FIXED_MAP_BMPS),$(FIXED_BMPS))


#
# Level categorization
#

GRASSY_LEVELS=S_00.ld S_01.ld S_02.ld S_03.ld S_04.ld S_05.ld S_06.ld C_02.ld C_03.ld M_05.ld M_11.ld M_13.ld M_14.ld M_16.ld M_17.ld M_19.ld M_22.ld
DESERT_LEVELS=S_07.ld S_08.ld S_09.ld S_10.ld S_11.ld S_12.ld S_13.ld S_14.ld C_01.ld C_04.ld C_05.ld D_03.ld M_02.ld M_03.ld M_04.ld M_06.ld M_07.ld M_08.ld M_09.ld M_10.ld M_12.ld M_15.ld M_18.ld M_20.ld M_21.ld
ifdef DEV_BUILD
GRASSY_LEVELS:=$(GRASSY_LEVELS) testlevel.ld M_01.ld
endif
ALL_LEVELS=$(DESERT_LEVELS) $(GRASSY_LEVELS)

#
# Targets
#
# $(subst sounds/,,$(subst .wav,.snd,$(wildcard sounds/*.wav))) \
#

BINS=\
$(patsubst %.ini.pp,%.ini,$(wildcard *.ini.pp)) \
$(patsubst %.amx,%.anir,$(patsubst $(ARTDIR)/%,%,$(wildcard $(ARTDIR)/*.amx))) \
$(patsubst %.ld,%.lvl,$(ALL_LEVELS))

TRGS=\
trg/desert_bmps_$(DT).trg trg/grassy_bmps_$(DT).trg \
trg/desert_levels_$(DT).trg trg/grassy_levels_$(DT).trg \
trg/fixed_map_bmps_$(DT).trg trg/fixed_form_bmps_$(DT).trg \
trg/fixed_amxs_$(DT).trg trg/fixed_raw_bmps_$(DT).trg \
trg/shell_raw_bmps_$(DT).trg trg/cutscene_raw_bmps_$(DT).trg \
trg/help_$(DT).trg trg/strings_$(DT).trg trg/font_bmps_$(DT).trg

all:
	@. ./makedirs.sh
	$(COPYART)
	@make --no-print-directory $(OUTDIR)/$(PDB)

clean:
	@. ./makedirs.sh
	$(COPYART)
	rm -f $(OUTDIR)/*.*
	rm -f trg/*$(DT).trg
	rm -f $(FIXED_PAL) $(DESERT_PAL) $(GRASSY_PAL)

special:
	make
	cp proc_base_cropped.png tbmtest.png
	$(MONO) $(TOOLDIR)/bcr2.exe -code $(FIXED_PAL) $(OUTDIR) tbmtest.png
	$(MONO) $(TOOLDIR)/packpdb2.exe -p $(CREATORID) $(OUTDIR)/$(PDB) $(OUTDIR)/*.*
	cp -f $(OUTDIR)/$(PDB) ../game/$(PDB)

#
# Make .pdb
#

$(OUTDIR)/$(PDB): $(TRGS) $(BINS:%=$(OUTDIR)/%)
	@echo +++VERSION+++ > $(OUTDIR)/version.txt
	$(MONO) $(TOOLDIR)/packpdb2.exe -p $(CREATORID) $(OUTDIR)/$(PDB) "$(OUTDIR)/*.*" -nocompress "*.snd" "*.tset*" "*.tbm" version.txt forms.ini
	cp -f $(OUTDIR)/$(PDB) ../game/$(PDB)

#
# Crunch .amx's
#

trg/fixed_amxs_$(DT).trg: $(FIXED_AMXS) $(FIXED_PAL) $(patsubst %.amx,%/*.png,$(FIXED_AMXS)) $(TOOLDIR)/acrunch.exe
	for i in $(FIXED_AMXS); do echo $(TOOLDIR)/acrunch $$i && $(MONO) $(TOOLDIR)/acrunch.exe $(AMX_SCALEARG) $(FIXED_PAL) $$i $(OUTDIR); done
	@echo x >trg/fixed_amxs_$(DT).trg

#
# Crunch .ini's
#

$(OUTDIR)/%.ini: %.ini.pp ../game/res.h
	@echo Preprocessing $(@F).pp...
	@$(CPP) -DTILESIZE=$(TILESIZE) $(DEFDEMO) $(DEFDEV) $(DEFINI) $(CPP_INCS) $(@F).pp ini.tmp
	@if [ -d "prepre" ]; then cp ini.tmp prepre/$(@F); fi
	@$(TOOLDIR)/$(INICRUNCH) ini.tmp $@

#	@if EXIST prepre. @cp ini.tmp prepre/$(@F)

#
# Help
#

trg/help_$(DT).trg: $(HELPFILE)
	@cp $(HELPFILE) $(OUTDIR)/help.txt
	@echo x > trg/help_$(DT).trg

#
# StringTable
#

trg/strings_$(DT).trg: ../game/wistrings.h
	$(MONO) $(TOOLDIR)/stringtable.exe ../game/wistrings.h $(OUTDIR)/strings.bin
	@echo x > trg/strings_$(DT).trg

#
# Crunch .bmps
#

trg/desert_bmps_$(DT).trg : $(DESERT_PAL) $(DESERT_BMPS)
	$(MONO) $(TOOLDIR)/bcr2.exe $(BMP_SCALEARG) $(DESERT_PAL) $(OUTDIR) $(DESERT_BMPS)
	@echo x > trg/desert_bmps_$(DT).trg

trg/grassy_bmps_$(DT).trg : $(GRASSY_PAL) $(GRASSY_BMPS)
	$(MONO) $(TOOLDIR)/bcr2.exe $(BMP_SCALEARG) $(GRASSY_PAL) $(OUTDIR) $(GRASSY_BMPS)
	@echo x > trg/grassy_bmps_$(DT).trg

trg/fixed_form_bmps_$(DT).trg : $(FIXED_PAL) $(FIXED_FORM_BMPS)
	$(MONO) $(TOOLDIR)/bcr2.exe $(FORMBMP_SCALEARG) $(FIXED_PAL) $(OUTDIR) $(FIXED_FORM_BMPS)
	@echo x > trg/fixed_form_bmps_$(DT).trg

trg/fixed_map_bmps_$(DT).trg : $(FIXED_PAL) $(FIXED_MAP_BMPS)
	$(MONO) $(TOOLDIR)/bcr2.exe $(MAPBMP_SCALEARG) $(FIXED_PAL) $(OUTDIR) $(FIXED_MAP_BMPS)
	@echo x > trg/fixed_map_bmps_$(DT).trg

trg/fixed_raw_bmps_$(DT).trg : $(FIXED_PAL) $(FIXED_RAW_BMPS)
	$(MONO) $(TOOLDIR)/bcr2.exe -raw $(RAWBMP_SCALEARG) $(FIXED_PAL) $(OUTDIR) $(FIXED_RAW_BMPS)
	@echo x > trg/fixed_raw_bmps_$(DT).trg

trg/shell_raw_bmps_$(DT).trg : $(SHELL_PAL) $(SHELL_RAW_BMPS)
	$(MONO) $(TOOLDIR)/bcr2.exe -raw $(RAWBMP_SCALEARG) $(SHELL_PAL) $(OUTDIR) $(SHELL_RAW_BMPS)
	@echo x > trg/shell_raw_bmps_$(DT).trg

trg/font_bmps_$(DT).trg : $(FIXED_PAL) $(FONT_BMPS)
	$(MONO) $(TOOLDIR)/bcr2.exe -font $(FIXED_PAL) $(OUTDIR) $(FONT_BMPS)
	@echo x > trg/font_bmps_$(DT).trg

#
# Cutscene bitmaps are special because each bitmap has its own palette
# which also must be crunched to the output dir
#

trg/cutscene_raw_bmps_$(DT).trg : $(patsubst %.bmp,%.act,$(CUTSCENE_RAW_BMPS)) $(CUTSCENE_RAW_BMPS) $(TOOLDIR)/palbin.exe $(TOOLDIR)/paltool.exe
	for i in $(CUTSCENE_RAW_BMPS); do echo Crunching cutscene $$i && $(MONO) ../bin/paltool.exe -6 -c $(PAL_SIZE) -p $(PAL_SIZE) -s -t -o _tmp.pal $(COMMON_PAL) $$i && $(MONO) ../bin/bcr2.exe -raw $(RAWBMP_SCALEARG) _tmp.pal $(OUTDIR) $$i && $(MONO) ../bin/palbin.exe _tmp.pal $(OUTDIR)/`basename $$i .bmp`.palbin && rm _tmp.pal; done
	@echo x > trg/cutscene_raw_bmps_$(DT).trg

#
# Crunch levels
#
# Parameter order:
# tile size (16 / 24)
# depth (4 / 8)
# nThresholdBackground (% of tile consumed by background before being considered background - for minimap)
# nLuminanceBackgroundMult (luminance multiplier for background minimap tiles)
# nSaturationBackgroundMult (saturation multiplier for background minimap tiles)
# nLuminanceForegroundMult (luminance multiplier for foreground minimap tiles)
# nSaturationForegroundMult (saturation multiplier for foreground minimap tiles)
# palette directory
# out directory
# levels (*.ld's)

trg/desert_levels_$(DT).trg : $(DESERT_LEVELS) $(DESERT_PAL) $(TOOLDIR)/mcl.exe
	$(MONO) $(TOOLDIR)/mcl.exe -levels $(TILESIZE) $(DEPTH) -1.0 1.0 1.0 1.0 1.0 $(PALDIR) $(OUTDIR) $(DESERT_LEVELS)
	@if [ -d "prepre" ]; then for i in $(subst .ld,.lvl,$(DESERT_LEVELS)); do cp $(OUTDIR)/$$i prepre; done; fi
	@for i in $(subst .ld,.lvl,$(DESERT_LEVELS)); do echo $$i; $(CPP) -imacros ../game/res.h $(OUTDIR)/$$i > ini.tmp; ../bin/$(INICRUNCH) ini.tmp $(OUTDIR)/$$i; done
	@echo x > trg/desert_levels_$(DT).trg

trg/grassy_levels_$(DT).trg : $(GRASSY_LEVELS) $(GRASSY_PAL) $(TOOLDIR)/mcl.exe
	$(MONO) $(TOOLDIR)/mcl.exe -levels $(TILESIZE) $(DEPTH) 0.8 0.9 0.9 1.1 1.3 $(PALDIR) $(OUTDIR) $(GRASSY_LEVELS)
	@if [ -d "prepre" ]; then for i in $(subst .ld,.lvl,$(GRASSY_LEVELS)); do cp $(OUTDIR)/$$i prepre; done; fi
	@for i in $(subst .ld,.lvl,$(GRASSY_LEVELS)); do echo $$i; $(CPP) -imacros ../game/res.h $(OUTDIR)/$$i > ini.tmp;  ../bin/$(INICRUNCH) ini.tmp $(OUTDIR)/$$i; done
	@echo x > trg/grassy_levels_$(DT).trg

#
# Crunch palettes
# Don't apply dependencies that will make these rebuild all the time, because it is
# not a fast process. They will get made if they don't exist, otherwise they can
# be forced to rebuild with "make clean".
#

$(FIXED_PAL) : $(COMMON_PAL) $(TOOLDIR)/paltool.exe
	@echo $(patsubst %.amx,%/*.png,$(FIXED_AMXS)) > files.tmp
	$(MONO) $(TOOLDIR)/paltool.exe -6 -c $(FIXED_PAL_SIZE) -p $(PAL_SIZE) -s -t -o $(FIXED_PAL) -f files.tmp $(COMMON_PAL)

# Shell palette contains the special colors needed for the title (main) screen

$(SHELL_PAL) : $(FIXED_PAL) $(TOOLDIR)/paltool.exe $(TOOLDIR)/palbin.exe $(SHELL_RAW_BMPS)
	$(MONO) $(TOOLDIR)/paltool.exe -6 -c $(PAL_SIZE) -p $(PAL_SIZE) -s -t -o $(SHELL_PAL) $(FIXED_PAL) $(SHELL_RAW_BMPS)
	$(MONO) $(TOOLDIR)/palbin.exe $(SHELL_PAL) $(OUTDIR)/shell.palbin
	$(MONO) $(TOOLDIR)/shadowmap.exe $(SHELL_PAL) $(OUTDIR)/shell.palbin.shadowmap 0.6

$(DESERT_PAL) : $(FIXED_PAL) $(TOOLDIR)/palbin.exe
	$(MONO) $(TOOLDIR)/mcl.exe -makepal $(TILESIZE) desert.tc $(PAL_SIZE) $(FIXED_PAL_SIZE) $(DESERT_BACKGROUND_PAL_SIZE) $(FIXED_PAL) $(DESERT_PAL)
	$(MONO) $(TOOLDIR)/palbin.exe $(DESERT_PAL) $(OUTDIR)/desert.palbin
	$(MONO) $(TOOLDIR)/shadowmap.exe $(DESERT_PAL) $(OUTDIR)/desert.palbin.shadowmap 0.6

$(GRASSY_PAL) : $(FIXED_PAL) $(TOOLDIR)/palbin.exe
	$(MONO) $(TOOLDIR)/mcl.exe -makepal $(TILESIZE) grassy.tc $(PAL_SIZE) $(FIXED_PAL_SIZE) $(GRASSY_BACKGROUND_PAL_SIZE) $(FIXED_PAL) $(GRASSY_PAL)
	$(MONO) $(TOOLDIR)/palbin.exe $(GRASSY_PAL) $(OUTDIR)/grassy.palbin
	$(MONO) $(TOOLDIR)/shadowmap.exe $(GRASSY_PAL) $(OUTDIR)/grassy.palbin.shadowmap 0.6
