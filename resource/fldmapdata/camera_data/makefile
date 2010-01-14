#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2009.07.09
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tamada fuchino unno tomoya_takahashi obata_toshihiro

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = field_camera_data

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)fieldmap

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

ELF2BIN		= $(PROJECT_ROOT)/tools/celf2bin.exe
XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb
#------------------------------------------------------------------------------
#データ
#------------------------------------------------------------------------------
DAT_FILE = $(wildcard *.rail)
OBJ_FILE = $(RAILFILE:.rail=.dat)

LIST_FILE 	= list.txt

DATAFILE	= field_camera.xls
DATATXT = $(DATAFILE:.xls=.txt)

CDAT	= field_camera.dat
#------------------------------------------------------------------------------
#ヘッダ参照先
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
#	#サフィックス
#	#コンバート対象者のみ、コンバートのルールを有効にする
#------------------------------------------------------------------------------


.SUFFIXES: .c .bin


LDIRT_CLEAN = $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname).naix
#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
LDIRT_CLEAN += *.dat *.bin $(targetname).narc $(targetname).naix arc_result.txt
endif


LINCLUDES	= $(NITROSDK_ROOT)/include
LINCLUDES	+= $(dir $<)

INCPATH	= -I$(NITROSDK_ROOT)/include
#----------------------------------------------------------------------------
#	Ｃソース→バイナリ変換を可能にするための設定
#----------------------------------------------------------------------------
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)

include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#%.bin: %.c
#field_camera.bin: field_camera.dat
field_camera.bin: $(DATATXT)
	@ruby field_camera.rb $(DATATXT) 
	@echo "camera parameter convert"
	@echo "//" > temp.c
	@cat dummy_header.h field_camera.dat |unix2dos > temp.c
	@$(CC) $(INCLUDES) $(INCPATH)  -c temp.c -MD -o temp.o
	@$(MWLD) -dis -o field_camera.elf temp.o
	@$(ELF2BIN) field_camera.elf
	@-rm temp.o temp.c temp.d
	@-rm field_camera.elf





.PHONY:	do-build subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc:  $(targetname).narc 
	@echo copying $(targetname).narc
	@$(COPY) $(targetname).narc $(TARGETDIR)
	@$(COPY) $(targetname).naix $(TARGETDIR)


#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
# コピー先のnarcが古い場合実行する
$(targetname).narc: field_camera.bin
	@nnsarc -i -cl -l -n $(targetname).narc *.bin > arc_result.txt
	@$(NAIXCUT) $(targetname).naix

$(DATATXT):	$(DATAFILE)
	$(XLS2TAB) -c $(DATAFILE) > $(DATATXT)

#field_camera.dat: $(DATATXT)
#	@ruby field_camera.rb $(DATATXT) 


endif

make-verinfo:
	@echo field camera parameter converter 1.1.0
#------------------------------------------------------------------------------
#	make clean ルール
#------------------------------------------------------------------------------
#警告抑止のため、LDIRT_CLEANを定義してNitroのMakefileに任せるようにした


