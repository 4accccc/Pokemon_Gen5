#! make -f
#----------------------------------------------------------------------------
# Project:  Nintendo DS  Development environment
# File:     Makefile
#
# Copyright 2006 GameFreak.inc  All rights reserved.
#----------------------------------------------------------------------------


#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#	環境が違うところでMakeできないようにするためのおまじない
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#現在のディレクトリ（cygwinのパスから変換する）
NOWDIR	=	$(shell pwd|tr [A-Z] [a-z] |sed -e 's/\/cygdrive\///')/
#環境変数が想定しているディレクトリから、：を取り除いたもの
TGTDIR	= $(shell echo $(PROJECT_ROOT)|tr [A-Z] [a-z]|sed -e 's/://')
ifneq ($(NOWDIR),$(TGTDIR))
$(warning コンパイル環境を確認してください)
$(error $(NOWDIR) != $(TGTDIR) )
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



MULTIBOOT_ARC_DIR = ./multiboot/arc/
MULTIBOOT_DIR = ./multiboot/
MULTIBOOT2_ARC_DIR = ./multiboot2/arc/
MULTIBOOT2_DIR = ./multiboot2/
RESOURCE_DIR = ./resource/
PROG_DIR = ./prog/
ARC_DIR = ./prog/arc
LIB_DIR = ./lib


.PHONY : do-build rom force clean	prog resource arc test landdata dmyfmmdl build_model area_data

#  makeした場合にすべて作られるように記述してください
#--------------------------------------- ----------------------------
do-build:
	$(MAKE) -C $(RESOURCE_DIR)
#	$(MAKE) -C $(MULTIBOOT_ARC_DIR)
#	$(MAKE) -C $(MULTIBOOT_DIR)
#	$(MAKE) -C $(MULTIBOOT_DIR) installsrl
	$(MAKE) -C $(ARC_DIR)
	$(MAKE) -C $(PROG_DIR)


rom:
	touch prog/src/test/testmode.c
	$(MAKE)

force:
	rm -rf prog/depend/*
	rm -rf multiboot/depend/*
	$(MAKE)

#-------------------------------------------------------------------
#  色違い・ROM版作成用
#-------------------------------------------------------------------
white:
	cp ./prog/spec/version_w ./prog/version
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) -C $(ARC_DIR) clean
	$(MAKE) -C $(RESOURCE_DIR)
	$(MAKE) -C $(ARC_DIR) 
	$(MAKE) -C $(PROG_DIR) 
#	cp ./prog/spec/version ./prog/version

black:
	cp ./prog/spec/version_b ./prog/version
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) -C $(ARC_DIR) clean
	$(MAKE) -C $(RESOURCE_DIR)
	$(MAKE) -C $(ARC_DIR) 
	$(MAKE) -C $(PROG_DIR) 
#	cp ./prog/spec/version ./prog/version

fes:
	cp ./prog/spec/version_fes ./prog/version
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) -C $(ARC_DIR) clean
	$(MAKE) -C $(RESOURCE_DIR)
	$(MAKE) -C $(ARC_DIR) 
	$(MAKE) -C $(PROG_DIR) 
#	cp ./prog/spec/version ./prog/version

white_rom:
	cp ./prog/spec/version_wr ./prog/version
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) -C $(ARC_DIR) clean
	$(MAKE) -C $(RESOURCE_DIR)
	$(MAKE) -C $(ARC_DIR) 
	$(MAKE) -C $(PROG_DIR) 
#	cp ./prog/spec/version ./prog/version

black_rom:
	cp ./prog/spec/version_br ./prog/version
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) -C $(ARC_DIR) clean
	$(MAKE) -C $(RESOURCE_DIR)
	$(MAKE) -C $(ARC_DIR) 
	$(MAKE) -C $(PROG_DIR) 
#	cp ./prog/spec/version ./prog/version

fes_rom:
	cp ./prog/spec/version_fesr ./prog/version
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) -C $(ARC_DIR) clean
	$(MAKE) -C $(RESOURCE_DIR)
	$(MAKE) -C $(ARC_DIR) 
	$(MAKE) -C $(PROG_DIR) 
	cp ./prog/bin/ARM9-TS.HYB/Rom/main.srl ./rom/VHYJ_r.srl
#	cp ./prog/spec/version ./prog/version

black_rom_protect:
	cp ./prog/spec/version_brp ./prog/version
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) -C $(ARC_DIR) clean
	$(MAKE) -C $(RESOURCE_DIR)
	$(MAKE) -C $(ARC_DIR) 
	$(MAKE) -C $(PROG_DIR) 
#	cp ./prog/spec/version ./prog/version

white_rom_protect:
	cp ./prog/spec/version_wrp ./prog/version
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) -C $(ARC_DIR) clean
	$(MAKE) -C $(RESOURCE_DIR)
	$(MAKE) -C $(ARC_DIR) 
	$(MAKE) -C $(PROG_DIR) 
#	cp ./prog/spec/version ./prog/version

fes_rom_protect:
	cp ./prog/spec/version_fesrp ./prog/version
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) -C $(ARC_DIR) clean
	$(MAKE) -C $(RESOURCE_DIR)
	$(MAKE) -C $(ARC_DIR) 
	$(MAKE) -C $(PROG_DIR) 
	cp ./prog/bin/ARM9-TS.HYB/Rom/main.srl ./rom/VHYJ_p.srl
#	cp ./prog/spec/version ./prog/version


master_rom:
	$(MAKE) white_rom
	cp ./prog/bin/ARM9-TS.HYB/Rom/main.srl ./rom/IRDJ.srl
	$(MAKE) black_rom
	cp ./prog/bin/ARM9-TS.HYB/Rom/main.srl ./rom/IREJ.srl

master:
	$(MAKE) white
	cp ./prog/bin/ARM9-TS.HYB/Release/main.srl ./rom/IRDJ_r.srl
	$(MAKE) black
	cp ./prog/bin/ARM9-TS.HYB/Release/main.srl ./rom/IREJ_r.srl

master_rom_protect:
	$(MAKE) white_rom_protect
	cp ./prog/bin/ARM9-TS.HYB/Rom/main.srl ./rom/IRDJ_p.srl
	$(MAKE) black_rom_protect
	cp ./prog/bin/ARM9-TS.HYB/Rom/main.srl ./rom/IREJ_p.srl


# cleanした場合にすべて消えるように記述してください
#--------------------------------------------------------------------
clean:
	$(MAKE) -C $(RESOURCE_DIR) -j3 clean
	$(MAKE) -C $(MULTIBOOT_ARC_DIR) clean
	$(MAKE) -C $(MULTIBOOT_DIR) clean
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) -C $(ARC_DIR) clean
	rm -rf ./prog/filetree/a

#--------------------------------------------------------------------
prog:
	$(MAKE) -C $(PROG_DIR)

resource:
	$(MAKE) -C $(RESOURCE_DIR)

arc:
	$(MAKE) -C $(ARC_DIR)

mb:
	$(MAKE) -C $(RESOURCE_DIR)
	$(MAKE) -C $(MULTIBOOT_ARC_DIR)
	$(MAKE) -C $(MULTIBOOT_DIR)
#	$(MAKE) -C $(MULTIBOOT_DIR) installsrl
	$(MAKE) -C $(MULTIBOOT2_ARC_DIR)
	$(MAKE) -C $(MULTIBOOT2_DIR)
#	$(MAKE) -C $(MULTIBOOT2_DIR) installsrl
	$(MAKE) -C $(ARC_DIR)
	$(MAKE) -C $(PROG_DIR)

test:
	$(MAKE) CONVERTUSER=true clean
	$(MAKE) CONVERTUSER=true
	$(MAKE) CONVERTUSER=false clean
	$(MAKE) CONVERTUSER=false


sddebug:
	$(MAKE) -C $(PROG_DIR) clean
	$(MAKE) PM_DEBUG_SD_PRINT=yes


# フィールドマップ　地形データのみclean make
#--------------------------------------------------------------------
FLD_LAND_DATA_DIR = $(RESOURCE_DIR)/fldmapdata/land_data/

landdata:
	$(MAKE) -C $(FLD_LAND_DATA_DIR) clean
	$(MAKE) -C $(FLD_LAND_DATA_DIR)
	$(MAKE)

# フィールド動作モデル　ダミーモデル作成
#--------------------------------------------------------------------
FLD_MMDL_DATA_DIR = $(RESOURCE_DIR)/fldmmdl/

fmmdldmy:
	$(MAKE) -C $(FLD_MMDL_DATA_DIR) dmyres


build_model:
	$(MAKE) -C $(RESOURCE_DIR)/fldmapdata/build_model clean
	$(MAKE)

area_data:
	$(MAKE) -C $(RESOURCE_DIR)/fldmapdata/area_data clean
	$(MAKE)

eventdata:
	$(MAKE) -C $(RESOURCE_DIR)/fldmapdata/eventdata clean
	$(MAKE)

# トレーナーグラフィック更新用 resource/trgra/convの削除
#--------------------------------------------------------------------
trgra_clean:
	$(MAKE) -C $(RESOURCE_DIR)/trgra/conv clean
	$(MAKE) -C $(RESOURCE_DIR)/trgra clean
	$(MAKE) -C $(RESOURCE_DIR)/trface clean

#----------------------------------------------------------------------------
#	make後、日時を表示
#----------------------------------------------------------------------------
date:
	$(MAKE)
	date

make up_fes_d:
	cp prog\bin\ARM9-TS.HYB\Release\main.srl p:\swan\rom\iwasawa\fes_rom_d.srl
# cp prog\bin\ARM9-TS.HYB\Release\main.tlf p:\swan\rom\iwasawa\debug
#	cp prog\bin\ARM9-TS.HYB\Release\main.tef p:\swan\rom\iwasawa\debug
#	cp prog\bin\ARM9-TS.HYB\Release\main.tef.xMAP p:\swan\rom\iwasawa\debug

