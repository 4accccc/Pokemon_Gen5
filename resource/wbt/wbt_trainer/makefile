#------------------------------------------------------------------------------
#
#	WBTトレーナーデータコンバート環境
#
#	@author		tamada GAMEFREAK inc.
#	@date			2011.03.09
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tamada suginaka_katsunori

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
arctarget = wbt_trlist
trdata_bin		= wbt_trainer.bin
trname_gmm = wbt_trname.gmm
trtitle_gmm = wbt_trtitle.gmm
#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)wbt
GMMDIR	= $(PROJECT_RSCDIR)message/src

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

ELF2BIN		= $(PROJECT_ROOT)/tools/celf2bin.exe
XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb
CONV			= wbt_trainer.rb
#------------------------------------------------------------------------------
#データ
#------------------------------------------------------------------------------
DAT_FILE = $(wildcard *.rail)
OBJ_FILE = $(RAILFILE:.rail=.dat)

LIST_FILE 	= list.txt

DATAFILE	= wbt_trainer_list.xls
DATATXT = $(DATAFILE:.xls=.txt)

BINCDAT	= $(trdata_bin:.bin=.cdat)

MSGCDAT	= wbt_trmsg_id.cdat
#------------------------------------------------------------------------------
#ヘッダ参照先
#------------------------------------------------------------------------------
WBT_DEPENDS	= $(PROJECT_PROGDIR)include/pm_version.h \
							$(PROJECT_PROGDIR)include/poke_tool/poketype_def.h \
							$(PROJECT_PROGDIR)src/field/wbt_trlist_struct.h \
							$(PROJECT_PROGDIR)include/field/wbt.h \
							../../sound/swan_sound_data.sadl \
							../../trtype/trtype_def.h \
							../../fldmmdl/fldmmdl_objcode.h

#------------------------------------------------------------------------------
#	#サフィックス
#	#コンバート対象者のみ、コンバートのルールを有効にする
#------------------------------------------------------------------------------


.SUFFIXES: .c .bin


LDIRT_CLEAN = $(TARGETDIR)/$(arctarget).narc $(TARGETDIR)/$(arctarget).naix

#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
LDIRT_CLEAN += *.dat *.bin $(arctarget).narc $(arctarget).naix arc_result.txt
LDIRT_CLEAN += $(GMMDIR)/$(trname_gmm) $(GMMDIR)/$(trtitle_gmm)
LDIRT_CLEAN += $(trname_gmm) $(trtitle_gmm)
LDIRT_CLEAN += $(MSGCDAT)
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
#wbt_trainer.bin: wbt_trainer.dat
$(trdata_bin): $(DATATXT) $(WBT_DEPENDS) $(CONV)
	@ruby $(CONV) list $(DATATXT) > $(BINCDAT)
	@echo "WBT trainer parameter convert"
	@echo "//" > temp.c
	@cat dummy_header.h $(BINCDAT) |unix2dos > temp.c
	@$(CC) $(INCLUDES) $(INCPATH)  -c temp.c -MD -o temp.o
	@$(MWLD) -dis -o wbt_trainer.elf temp.o
	@$(ELF2BIN) wbt_trainer.elf
	@-rm temp.o temp.c temp.d
	@-rm wbt_trainer.elf





.PHONY:	do-build subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(arctarget).narc $(GMMDIR)/$(trname_gmm) $(GMMDIR)/$(trtitle_gmm) $(MSGCDAT)

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(arctarget).narc:  $(arctarget).narc 
	@echo copying $(arctarget).narc
	@$(COPY) $(arctarget).narc $(TARGETDIR)/
	@$(COPY) $(arctarget).naix $(TARGETDIR)/

#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
# コピー先のnarcが古い場合実行する
$(arctarget).narc: $(trdata_bin)
	@nnsarc -i -cl -l -n $(arctarget).narc *.bin > arc_result.txt
	@$(NAIXCUT) $(arctarget).naix

$(DATATXT):	$(DATAFILE)
	$(XLS2TAB) -c $(DATAFILE) > $(DATATXT)

$(GMMDIR)/$(trname_gmm):	$(trname_gmm)
	$(COPY) $(trname_gmm) $(GMMDIR)

$(GMMDIR)/$(trtitle_gmm):	$(trtitle_gmm)
	$(COPY)	$(trtitle_gmm) $(GMMDIR)

$(trname_gmm) $(trtitle_gmm) :	$(DATATXT) $(CONV)
	ruby $(CONV) gmm $(DATATXT) $(GMMDIR)/../template.gmm

$(MSGCDAT):	$(DATATXT) $(CONV)
	ruby $(CONV) msgid $(DATATXT) > $(MSGCDAT)

#wbt_trainer.dat: $(DATATXT)
#	@ruby $(CONV) $(DATATXT) 

endif

make-verinfo:
	@echo field WBT trainer parameter converter 1.1.0
#------------------------------------------------------------------------------
#	make clean ルール
#------------------------------------------------------------------------------
#警告抑止のため、LDIRT_CLEANを定義してNitroのMakefileに任せるようにした


