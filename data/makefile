MONO=mono
PYTHON=python

UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
INICRUNCH=inicrunch_osx
CPP=cpp -P -Wno-invalid-pp-token -Wno-trigraphs
endif
ifeq ($(UNAME), Linux)
INICRUNCH=inicrunch_linux
CPP=cpp -P -Wno-trigraphs
endif

HELPFILE=helpiphone-multiplayer.txt
DEPTH=24
TILESIZE=32

ARTDIR:=../../art$(DEPTH)$(TILESIZE)
DT:=$(DEPTH)$(TILESIZE)
OUTDIR:=htdata$(DT)
TEXPACKDIR:=../texpack
FONTMAP:=../font/fontmap.py

#
# Global defines
#

CPP_INCS=-I../game
TOOLDIR=../bin
PDB=htdata$(DT).pdb
CREATORID=WARI
VERSION=+++VERSION+++

#
# Level categorization
#

GRASSY_LEVELS=s_00.ld s_01.ld s_02.ld s_03.ld s_04.ld s_05.ld s_06.ld c_02.ld c_03.ld m_05.ld m_11.ld m_13.ld m_14.ld m_16.ld m_17.ld m_19.ld m_22.ld
DESERT_LEVELS=s_07.ld s_08.ld s_09.ld s_10.ld s_11.ld s_12.ld s_13.ld s_14.ld c_01.ld c_04.ld c_05.ld d_03.ld m_02.ld m_03.ld m_04.ld m_06.ld m_07.ld m_08.ld m_09.ld m_10.ld m_12.ld m_15.ld m_18.ld m_20.ld m_21.ld
ALL_LEVELS=$(DESERT_LEVELS) $(GRASSY_LEVELS)

#
# amx categorization
#

AMXS := $(wildcard $(ARTDIR)/*.amx)

# Missing 2432 art has been substituted with art from art824.
# This 824 art needs to be scaled. Keep the following list
# in sync with the animation section of ART_8BIT within
# texpack/texjson.py 

AMXS_SCALE := \
$(ARTDIR)/movetarget.amx \
$(ARTDIR)/vacuum.amx \
$(ARTDIR)/upgrades.amx \
$(ARTDIR)/tankshot.amx \
$(ARTDIR)/rocket.amx \
$(ARTDIR)/ricochet.amx \
$(ARTDIR)/bullet.amx \
$(ARTDIR)/andyshot.amx \
$(ARTDIR)/activator.amx \
$(ARTDIR)/vts.amx \
$(ARTDIR)/replicator.amx
AMX_SCALEARG:=-scale 1.3333333333333 -noscaleicon


#
# Bins
#

BINS=\
$(patsubst %.ini.pp,%.ini,$(wildcard *.ini.pp)) \
$(patsubst %.ld,%.lvl,$(ALL_LEVELS))

#
# TRGS
#

TRGS=\
trg/help_$(DT).trg \
trg/strings_$(DT).trg \
trg/all_levels_$(DT).trg \
trg/amxs_$(DT).trg \
trg/textures_$(DT).trg


all:
	@. ./makedirs.sh
	@make --no-print-directory $(OUTDIR)/$(PDB)

clean:
	@. ./makedirs.sh
	@rm -rf $(OUTDIR)/*
	@rm -f trg/*$(DT).trg

bins:
	@make $(BINS:%=$(OUTDIR)/%)

amxs:
	@make trg/amxs_$(DT).trg

levels:
	@make trg/all_levels_$(DT).trg

help:
	@make trg/help_$(DT).trg

textures:
	@make trg/textures_$(DT).trg

mark:
	@echo $(VERSION) > $(OUTDIR)/version.txt

#
# Make data
#

$(OUTDIR)/$(PDB): $(PNG_TARGET) $(TXT_TARGET) $(TRGS) $(BINS:%=$(OUTDIR)/%)
	@echo $(VERSION) > $(OUTDIR)/version.txt

#
# Crunch .ini's
#

$(OUTDIR)/%.ini: %.ini.pp ../game/res.h
	@echo Preprocessing $(@F).pp...
	@$(CPP) -DTILESIZE=$(TILESIZE) $(DEFDEMO) $(DEFDEV) $(DEFINI) $(CPP_INCS) $(@F).pp ini.tmp
	@if [ -d "prepre" ]; then cp ini.tmp prepre/$(@F); fi
	@$(TOOLDIR)/$(INICRUNCH) ini.tmp $@

#
# Copy txt
#

$(OUTDIR)/%.txt : %.txt
	@echo Processing $(@F) 
	@cp $< $@
vpath %.txt $(TXT_DIRS)

#
# StringTable
#

trg/strings_$(DT).trg: ../game/wistrings.h
	$(MONO) $(TOOLDIR)/stringtable.exe ../game/wistrings.h $(OUTDIR)/strings.bin
	@echo x > trg/strings_$(DT).trg

#
# Help
#

trg/help_$(DT).trg: $(HELPFILE)
	@cp $(HELPFILE) $(OUTDIR)/help.txt
	@echo x > trg/help_$(DT).trg

#
# Crunch levels
#

trg/all_levels_$(DT).trg : $(ALL_LEVELS) $(TOOLDIR)/mcl.exe
	$(MONO) $(TOOLDIR)/mcl.exe -levels $(TILESIZE) $(DEPTH) -1.0 1.0 1.0 1.0 1.0 $(OUTDIR) $(ALL_LEVELS)
	@if [ -d "prepre" ]; then for i in $(subst .ld,.lvl,$(ALL_LEVELS)); do cp $(OUTDIR)/$$i prepre; done; fi
	@for i in $(subst .ld,.lvl,$(ALL_LEVELS)); do echo $$i; $(CPP) -imacros ../game/res.h $(OUTDIR)/$$i > ini.tmp;  ../bin/$(INICRUNCH) ini.tmp $(OUTDIR)/$$i; done
	@echo x > trg/all_levels_$(DT).trg

#
# Crunch .amx's
#

trg/amxs_$(DT).trg: $(AMXS) $(TOOLDIR)/acrunch.exe
	@for i in $(AMXS); do \
		if [[ "$(AMXS_SCALE)" =~ "$$i" ]]; then \
			echo "acrunch.exe $(AMX_SCALEARG) $$i $(OUTDIR)"; \
			$(MONO) $(TOOLDIR)/acrunch.exe $(AMX_SCALEARG) $$i $(OUTDIR); \
            	else \
			echo "acrunch.exe $$i $(OUTDIR)"; \
                	$(MONO) $(TOOLDIR)/acrunch.exe $$i $(OUTDIR); \
            	fi; \
	done; \

#
# Pack texture atlases
#

trg/textures_$(DT).trg:
	@$(PYTHON) $(FONTMAP) $(ARTDIR) $(OUTDIR)
	@$(PYTHON) $(TEXPACKDIR)/texjson.py $(ARTDIR) trg/artjson_$(DT).trg
	@$(MONO) $(TOOLDIR)/texpack.exe trg/artjson_$(DT).trg $(OUTDIR)
	@rm trg/artjson_$(DT).trg
