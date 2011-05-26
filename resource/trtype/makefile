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

XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out_sheet.rb -c
#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------

#トレーナータイプID定義ヘッダ
TYPE_HEADER = trtype_def.h

#トレーナータイプ別データテーブル
TYPE_CDAT = trtype_data.cdat
#トレーナータイプ別性別指定テーブル
#GENDER_CDAT = trtype_sex.cdat
#トレーナータイプ別CG指定テーブル
#GRATBL_CDAT = trtype_gra.cdat
#視線BGM指定テーブル
#TYPE_BGM_CDAT	= trtype_bgm.cdat

#トレーナータイプ別名称gmmファイル
TYPE_GMMFILE = trtype.gmm

#トレーナータイプ別ハッシュ
TYPE_HASH = trtype_hash.rb

#トレーナータイプ別グラフィック指定ファイル（正面）
SCRFILE_FRONT = trfgra_wb.scr
#トレーナータイプ別グラフィック指定ファイル（背面）
SCRFILE_BACK = trbgra_wb.scr
#トレーナーグラフィックリストファイル（trdata_exist.rbで使用する）
LSTFILE = trfgra_wb.lst

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= ../../prog/include/tr_tool/
CDATDIR = ../../prog/src/tr_tool/
GMMDIR	= ../../resource/message/src/
GRADIR	= ../../resource/trgra/
HASHDIR	= ../../tools/hash/
GRATBLDIR	= ../../prog/src/system/

TRTOOLSDIR = $(PROJECT_ROOT)tools/trdata_conv/

#生成する対象の全ファイル
ALL_TARGETS =  $(TARGETDIR)$(TYPE_HEADER) \
							 $(GMMDIR)$(TYPE_GMMFILE) \
							 $(HASHDIR)$(TYPE_HASH) \
							 $(GRADIR)$(SCRFILE_FRONT) \
							 $(GRADIR)$(SCRFILE_BACK) \
							 $(CDATDIR)$(TYPE_CDAT)
							 #$(CDATDIR)$(GENDER_CDAT) \
							 #$(GRATBLDIR)$(GRATBL_CDAT) \
							 #$(CDATDIR)$(TYPE_BGM_CDAT)

#ALL_TARGETSからディレクトリ指定を除いたもの
#	つまり、ローカルでコンバート対象ユーザーが生成するもの
LOCAL_TARGETS = $(notdir $(ALL_TARGETS))

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
LDIRT_CLEAN =  $(ALL_TARGETS)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += *.s *.scr out_end
LDIRT_CLEAN += $(LOCAL_TARGETS)
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
	ruby file_exist.rb $(LSTFILE) ../../../swan_doc/trgra/
	#ruby $(TRTOOLSDIR)file_exist.rb $(LSTFILE) ../../../swan_doc/trgra/

trtype_list.tab: trtype_list.xls
	$(XLS2TAB) trtype_list.xls > trtype_list.tab
endif

do-build: $(ALL_TARGETS)

$(TARGETDIR)$(TYPE_HEADER):	$(TYPE_HEADER)
	$(COPY)	$(TYPE_HEADER) $(TARGETDIR)

$(GMMDIR)$(TYPE_GMMFILE):	$(TYPE_GMMFILE)
	$(COPY)	$(TYPE_GMMFILE) $(GMMDIR)

$(GRADIR)$(SCRFILE_FRONT):	$(SCRFILE_FRONT)
	$(COPY)	$(SCRFILE_FRONT) $(GRADIR)

$(GRADIR)$(SCRFILE_BACK):	$(SCRFILE_BACK)
	$(COPY)	$(SCRFILE_BACK) $(GRADIR)

$(HASHDIR)$(TYPE_HASH):	$(TYPE_HASH)
	$(COPY)	$(TYPE_HASH) $(HASHDIR)

$(CDATDIR)$(TYPE_CDAT): $(TYPE_CDAT)
	$(COPY) $(TYPE_CDAT) $(CDATDIR)

#$(CDATDIR)$(GENDER_CDAT):	$(GENDER_CDAT)
#	$(COPY)	$(GENDER_CDAT) $(CDATDIR)

#$(GRATBLDIR)$(GRATBL_CDAT):	$(GRATBL_CDAT)
#	$(COPY)	$(GRATBL_CDAT) $(GRATBLDIR)

#$(CDATDIR)$(TYPE_BGM_CDAT): $(TYPE_BGM_CDAT)
#	$(COPY) $(TYPE_BGM_CDAT) $(CDATDIR)


