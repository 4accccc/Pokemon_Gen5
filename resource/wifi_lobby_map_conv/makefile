##-----------------------------------------------------------------------------
#**
#*			WIFIロビーマップコンバータ
#**
##-----------------------------------------------------------------------------


#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= matsuda

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = wflby_map

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

SUBDIRS		=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

##############################################################################
#			マップデータ
##############################################################################
OUTPUT = wflby_map.dat
ARC_FILE 	= wflby_map.narc
ARCH_FILE 	= wflby_map.naix
EXCELL_CONV = $(PROJECT_ROOT)/tools/exceltool/ExcelSeetConv.exe

##############################################################################
#			グラフィックデータ
##############################################################################
GRA_NAIX_DIR	= .\\..\\wifi_lobby\\
GRA_NAIX_FILE	= wifi_lobby.naix
GRA_DATA_FILE	= wflby_3dmapobj_data.c
GRA_TARGET_FILE	= 	wflby_3dmapobj_data_0.dat
CP_DATA_C = .\\..\\wifi_lobby\\wflby_3dmapobj_data.c


#----------------------------------------------------------------------------


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .xls

endif

.PHONY:	do-build clean subdir

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
do-build:cp_data wflby_map.narc
	$(COPY) wflby_mapdata.h $(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/
	$(COPY) wflby_maparc.h $(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/
	$(COPY) wflby_3dmapobj_data.h $(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)

cp_data:
	xcopy /d /y $(CP_DATA_C) .
	xcopy /d /y $(GRA_NAIX_DIR)$(GRA_NAIX_FILE) .

wflby_map.narc: wflby_map.dat $(GRA_TARGET_FILE)
	nnsarc -c -l -n -i wflby_map.narc -S arc_list.txt

wflby_map.dat:	wflby_mapdata.xls
	$(EXCELL_CONV) -o wflby_map.txt -p 1 -s csv wflby_mapdata.xls
	$(EXCELL_CONV) -o wflby_gdata.txt -p 2 -s csv wflby_mapdata.xls
	perl conv_gdata.pl

wflby_3dmapobj_data_0.dat: $(GRA_NAIX_FILE) $(GRA_DATA_FILE)
	perl conv_3dmapobj_data.pl


else
do-build:$(TARGETDIR)/$(targetname).narc \
		 $(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/wflby_mapdata.h \
		 $(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/wflby_maparc.h \
		 $(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/wflby_3dmapobj_data.h

$(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/wflby_mapdata.h:wflby_mapdata.h
	$(COPY) wflby_mapdata.h $(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/

$(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/wflby_maparc.h:wflby_maparc.h
	$(COPY) wflby_maparc.h $(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/

$(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/wflby_3dmapobj_data.h:wflby_3dmapobj_data.h
	$(COPY) wflby_3dmapobj_data.h $(PROJECT_ROOT)/prog/src/net_app/wifi_lobby/

$(TARGETDIR)/$(targetname).narc:$(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *_data_*.dat
	rm -f wflby_gdata.txt
	rm -f wflby_map.dat
	rm -f wflby_map.txt
	rm -f wifi_lobby.naix
	rm -f wflby_3dmapobj_data*.dat
	rm -f $(GRA_DATA_FILE)
	rm -f wflby_map.naix
	rm -f wflby_map.narc
endif
