#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= saitou obata_toshihiro nakatsui kagaya iwasawa

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = gimmick_assign

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

#------------------------------------------------------------------------------
#データ
#------------------------------------------------------------------------------
DAT_FILE = $(wildcard *.rail)
OBJ_FILE = $(RAILFILE:.rail=.dat)

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
include	$(COMMONDEFS_CCTYPE_CW)
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs

include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

.PHONY:	do-build clean

#------------------------------------------------------------------------------
#	make clean ルール
#------------------------------------------------------------------------------
#警告抑止のため、LDIRT_CLEANを定義してNitroのMakefileに任せるようにした
LDIRT_CLEAN = $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname).naix
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
LDIRT_CLEAN += $(targetname).narc $(targetname).naix
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(targetname).narc

#clean:
#	@rm -f $(TARGETDIR)/$(targetname).narc
#	@rm -f $(TARGETDIR)/$(targetname).naix
#ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
#	@rm -f $(targetname).narc
#	@rm -f $(targetname).naix
#endif

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc:  $(targetname).narc 
	@echo copying $(targetname).narc
	@$(COPY) $(targetname).narc $(TARGETDIR)
	@$(COPY) $(targetname).naix $(TARGETDIR)


#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
# コピー先のnarcが古い場合実行する
$(targetname).narc: fld_gimmick_assign.xls ../zonetable/zone_id.h
	ruby assign.rb ../zonetable/zone_id.h fld_gimmick_assign.xls dst.bin
	@nnsarc -i -cl -l -n $(targetname).narc dst.bin > arc_result.txt
	$(NAIXCUT) $(targetname).naix
endif

make-verinfo:
	@echo field camera parameter converter 1.1.0


