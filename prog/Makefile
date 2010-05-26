#! make -f
#----------------------------------------------------------------------------
# Project:  Nintendo DS  Development environment 'pokemon_wb'
# File:     Makefile
#
# Copyright 2006 GameFreak.inc  All rights reserved.
#
#----------------------------------------------------------------------------

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#	環境が違うところでMakeできないようにするためのおまじない
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#現在のディレクトリ（cygwinのパスから変換する）
NOWDIR	=	$(shell pwd|tr [A-Z] [a-z] |sed -e 's/\/cygdrive\///')/
#環境変数が想定しているディレクトリから、：を取り除いたもの
TGTDIR	= $(shell echo $(PROJECT_PROGDIR)|tr [A-Z] [a-z]|sed -e 's/://')
ifneq ($(NOWDIR),$(TGTDIR))
$(warning コンパイル環境を確認してください)
$(error $(NOWDIR) != $(TGTDIR) )
endif

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



MAKE_OVERLAY = makefile_overlay
MAKE_MAIN = makefile_main


do-build:
	$(MAKE) -f $(MAKE_OVERLAY) lcf_def.txt
	$(MAKE) -f $(MAKE_OVERLAY) overlay/main.lsf
ifeq	($(USERNAME),HUDSON)	#HUDSONのみ、makeのスレッドを増やす
	@echo "====== hudson make ! - 6 ====="
	$(MAKE) -j 6 -f $(MAKE_MAIN)
else
ifeq	($(USERNAME),none)
	$(MAKE) -j 6 -f $(MAKE_MAIN)
else
	$(MAKE) -j 3 -f $(MAKE_MAIN)
endif
endif




clean:
	$(MAKE) -f $(MAKE_MAIN) clean


#----------------------------------------------------------------------------
#	ソースのみのリコンパイル
#----------------------------------------------------------------------------
source:
	$(MAKE) -f $(MAKE_OVERLAY)
	$(MAKE) -f $(MAKE_MAIN) source


#----------------------------------------------------------------------------
#	ソースあたりオブジェクトサイズ算出
#----------------------------------------------------------------------------
analyze:
	$(MAKE) -f $(MAKE_MAIN) analyze

#----------------------------------------------------------------------------
#	オーバーレイマップ可視化
#----------------------------------------------------------------------------
overlaymap:
	$(MAKE) -f $(MAKE_MAIN) overlaymap

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
filesize:
		$(MAKE) -f $(MAKE_MAIN) filesize

#----------------------------------------------------------------------------
#	警告フィルタの適用
#----------------------------------------------------------------------------
warning:
	$(MAKE) -f $(MAKE_MAIN) warning

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
echo_buildtype:
	$(MAKE) -f $(MAKE_MAIN) echo_buildtype


#----------------------------------------------------------------------------
#	削除するとプログラムメモリに余裕が出来るOS_Printをリストアップ
#----------------------------------------------------------------------------
include make_prog_files
check_printf:
	@perl $(PROJECT_ROOT)/tools/check_printf.pl SEARCH

	@echo ■常駐SRCS > check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_EVENT_WIFICLUB>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_EVENT_WIFICLUB) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_EVENT_BATTLECALL>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_EVENT_BATTLECALL) >> check_printf_result.txt

	@echo ★FIELD_ROOT>>check_printf_result.txt
	@echo ■SRCS_OVERLAY_NOTWIFI>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_NOTWIFI) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_INTRUDE_SYSTEM>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_INTRUDE_SYSTEM) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_DEV_WIRELESS>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_DEV_WIRELESS) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_DEV_IRC>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_DEV_IRC) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_FIELD_INTRUDE>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_FIELD_INTRUDE) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_FIELD_INIT>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_FIELD_INIT) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_FIELDMAP>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_FIELDMAP) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_FIELD_WEATHER_RAIN>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_FIELD_WEATHER_RAIN) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_BEACON_VIEW>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_BEACON_VIEW) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_CGEAR>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_CGEAR) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_MUSICAL_SHOT>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_MUSICAL_SHOT) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_BATTLE_RECORDER_MUSICAL_LOOK>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_BATTLE_RECORDER_MUSICAL_LOOK) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_BATTLE_RECORDER_CORE>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_BATTLE_RECORDER_CORE) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_BATTLE_RECORDER>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_BATTLE_RECORDER) >> check_printf_result.txt

	@echo ★BATTLE_ROOT>>check_printf_result.txt
	@echo ■SRCS_OVERLAY_DEV_WIFILIB>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_DEV_WIFILIB) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_DEV_WIFI>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_DEV_WIFI) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_BATTLE>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_BATTLE) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_BATTLE_VIEW>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_BATTLE_VIEW) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_BATTLE_B_APP>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_BATTLE_B_APP) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_BATTLE_PLIST>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_BATTLE_PLIST) >> check_printf_result.txt
	@echo ■SRCS_OVERLAY_SHINKA_DEMO>>check_printf_result.txt
	@perl $(PROJECT_ROOT)/tools/check_printf.pl CHECK $(SRCS_OVERLAY_SHINKA_DEMO) >> check_printf_result.txt
