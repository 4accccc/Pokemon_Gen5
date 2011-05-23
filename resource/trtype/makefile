#------------------------------------------------------------------------------
#
#	トレーナータイプ用Makefile
#
#	2011.05.23	tamada@gamefreak.co.jp
#------------------------------------------------------------------------------

RESOURCE_MACRO	= $(PROJECT_RSCDIR)\macro_define
#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
include ../trdata/user.def

USERS += tamada

XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out_sheet.rb -t
#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------

#トレーナータイプID定義ヘッダ
TYPE_HEADER = trtype_def.h

#トレーナータイプ別性別指定テーブル
GENDER_CDAT = trtype_sex.cdat

#トレーナータイプ別CG指定テーブル
GRATBL_CDAT = trtype_gra.cdat

#トレーナータイプ別名称gmmファイル
TYPE_GMMFILE = trtype.gmm

#トレーナータイプ別ハッシュ
HASH = trtype_hash.rb
#トレーナータイプ別グラフィック指定ファイル
SCRFILE = trfgra_wb.scr
#トレーナーグラフィックリストファイル（trdata_exist.rbで使用する）
LSTFILE = trfgra_wb.lst

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= ../../prog/include/tr_tool/
GMMDIR	= ../../resource/message/src/
GRADIR	= ../../resource/trgra/
HASHDIR	= ../../tools/hash/
GRATBLDIR	= ../../prog/src/system/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)/macro_define
#MWAS,MWLD
include	$(NITROSYSTEM_ROOT)/build/buildtools/commondefs
include	$(COMMONDEFS_CCTYPE_CW)
#Moduleルール
include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

#------------------------------------------------------------------------------
#	ファイル削除ルール
#------------------------------------------------------------------------------
LDIRT_CLEAN =  $(TARGETDIR)$(TYPE_HEADER) $(TARGETDIR)$(GENDER_CDAT) $(GMMDIR)$(TYPE_GMMFILE) $(HASHDIR)$(HASH) $(GRATBLDIR)$(GRATBL_CDAT)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += *.s  $(TYPE_HEADER) $(GENDER_CDAT) $(TYPE_GMMFILE) $(HASH) $(GRATBL_CDAT) *.scr out_end
endif

.PHONY:	do-build

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

#パーソナルデータ生成

do-build: out_end

out_end: trtype_list.tab
	ruby trtype_conv.rb $< ../message/template.gmm
	ruby ../../tools/trdata_conv/file_exist.rb $(LSTFILE) ../../../swan_doc/trgra/

trtype_list.tab: trtype_list.xls
	$(XLS2TAB) trtype_list.xls > trtype_list.tab
endif

do-build: $(TARGETDIR)$(TYPE_HEADER) $(TARGETDIR)$(GENDER_CDAT) $(GMMDIR)$(TYPE_GMMFILE) $(GRADIR)$(SCRFILE) $(HASHDIR)$(HASH) $(GRATBLDIR)$(GRATBL_CDAT)

$(TARGETDIR)$(TYPE_HEADER):	$(TYPE_HEADER)
	$(COPY)	$(TYPE_HEADER) $(TARGETDIR)

$(TARGETDIR)$(GENDER_CDAT):	$(GENDER_CDAT)
	$(COPY)	$(GENDER_CDAT) $(TARGETDIR)

$(GMMDIR)$(TYPE_GMMFILE):	$(TYPE_GMMFILE)
	$(COPY)	$(TYPE_GMMFILE) $(GMMDIR)

$(GRADIR)$(SCRFILE):	$(SCRFILE)
	$(COPY)	$(SCRFILE) $(GRADIR)

$(HASHDIR)$(HASH):	$(HASH)
	$(COPY)	$(HASH) $(HASHDIR)

$(GRATBLDIR)$(GRATBL_CDAT):	$(GRATBL_CDAT)
	$(COPY)	$(GRATBL_CDAT) $(GRATBLDIR)

