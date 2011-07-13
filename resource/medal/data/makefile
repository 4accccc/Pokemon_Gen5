#------------------------------------------------------------------------------
#
#	メダルデータ用Makefile
#
#	2011.01.7	onoue_masayuki
#------------------------------------------------------------------------------

EXCELCONV = ../../../tools/exceltool/ExcelSeetConv.exe
MEDALLIST_CSV 		= medal_data.csv
MEDALDATA_DEF_CSV = data_def.csv
MEDALLIST_XLS = medal_data.xls
MEDALLIST_H		= medal_data.h
MEDAL_ID_H  	= mdlid_def.h
MEDALRECID_H  = mdlrec_id.h
MEDAL_SORT_CDAT  = mdlsort_def.cdat
DATADIR				= ./convert_tmp/
TMPGMM				= ../../message/template.gmm
MEDALNAME_GMM	= medalname.gmm
MEDALEX_GMM		= medalexplanation.gmm
MEDALHELP_GMM = medalhelp.gmm
RECORD_H			= record.h
RECORD_H_PATH = ../../../prog/include/savedata/
RECORDRBDEF		= recorddef.rb

GRATYPERBDEF			= gratypeddef.rb

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	onoue_masayuki ito_hiroto

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
MEDALDATANARC	= medal_data
NARCNAME	= $(MEDALDATANARC).narc
NAIXNAME	= $(MEDALDATANARC).naix

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)
GMMDIR	= ../../message/src/
INCLUDE_DIR	= ../../../prog/include/app/
INCLUDE_TOOL_DIR	= ../../../prog/include/medal_tool/
SRC_DIR	= ../../../prog/src/app/medal_box/
CONVERTERDIR	= ../converter/

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
LDIRT_CLEAN = $(TARGETDIR)/$(NARCNAME) $(TARGETDIR)/$(NAIXNAME) $(GMMDIR)/$(MEDALNAME_GMM) $(GMMDIR)/$(MEDALEX_GMM) $(GMMDIR)/$(MEDALHELP_GMM) $(INCLUDE_DIR)/$(MEDAL_ID_H) $(SRC_DIR)/$(MEDAL_SORT_CDAT)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
LDIRT_CLEAN += $(NARCNAME) $(NAIXNAME) $(DATADIR)* $(MEDAL_SORT_CDAT) $(MEDAL_ID_H) $(MEDALRECID_H) $(MEDALNAME_GMM) $(MEDALEX_GMM) $(MEDALHELP_GMM) $(RECORD_H) $(CONVERTERDIR)$(RECORDRBDEF)
endif

.PHONY:	do-build

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

do-build: $(CONVERTERDIR)$(RECORDRBDEF) $(DATADIR)$(MEDALLIST_CSV) $(NARCNAME) 

$(CONVERTERDIR)$(RECORDRBDEF) : $(RECORD_H_PATH)$(RECORD_H)
	cp $(RECORD_H_PATH)$(RECORD_H) $(DATADIR)$(RECORD_H)
	ruby $(CONVERTERDIR)record_hash_maker.rb $(DATADIR)$(RECORD_H) $(CONVERTERDIR)$(RECORDRBDEF) 

$(DATADIR)$(MEDALLIST_CSV): $(MEDALLIST_XLS) $(RECORD_H_PATH)$(RECORD_H)
	@$(EXCELCONV) -o $(DATADIR)$(MEDALLIST_CSV) -p 1 -s csv $(MEDALLIST_XLS)
	@$(EXCELCONV) -o $(DATADIR)$(MEDALDATA_DEF_CSV) -p 2 -s csv $(MEDALLIST_XLS)
	ruby $(CONVERTERDIR)gratype_data_maker.rb $(CONVERTERDIR)$(GRATYPERBDEF) $(DATADIR)$(MEDALDATA_DEF_CSV)
	ruby $(CONVERTERDIR)medal_converter.rb $(DATADIR)$(MEDALLIST_CSV) $(MEDAL_ID_H) $(MEDALRECID_H) $(MEDAL_SORT_CDAT) $(TMPGMM) $(MEDALNAME_GMM) $(MEDALEX_GMM) $(MEDALHELP_GMM) $(DATADIR)

$(NARCNAME) : $(MEDALLIST_XLS)
	nnsarc -c -l -n -i $(NARCNAME) $(DATADIR)medal_*bin
	$(NAIXCUT) $(NAIXNAME)

endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR)/$(NARCNAME) $(TARGETDIR)/$(NAIXNAME) $(GMMDIR)/$(MEDALNAME_GMM) $(GMMDIR)/$(MEDALEX_GMM) $(GMMDIR)/$(MEDALHELP_GMM) $(INCLUDE_DIR)/$(MEDAL_ID_H) $(INCLUDE_DIR)/$(MEDALRECID_H) $(SRC_DIR)/$(MEDAL_SORT_CDAT)
$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)

$(TARGETDIR)/$(NAIXNAME):	$(NAIXNAME)
	$(COPY)	$(NAIXNAME) $(TARGETDIR)

$(GMMDIR)/$(MEDALNAME_GMM):	$(MEDALNAME_GMM)
	$(COPY)	$(MEDALNAME_GMM) $(GMMDIR)

$(GMMDIR)/$(MEDALEX_GMM):	$(MEDALEX_GMM)
	$(COPY)	$(MEDALEX_GMM) $(GMMDIR)

$(GMMDIR)/$(MEDALHELP_GMM):	$(MEDALHELP_GMM)
	$(COPY)	$(MEDALHELP_GMM) $(GMMDIR)

$(INCLUDE_DIR)/$(MEDAL_ID_H):	$(MEDAL_ID_H)
	$(COPY)	$(MEDAL_ID_H) $(INCLUDE_DIR)

$(INCLUDE_DIR)/$(MEDALRECID_H):	$(MEDALRECID_H)
	$(COPY)	$(MEDALRECID_H) $(INCLUDE_DIR)

$(SRC_DIR)/$(MEDAL_SORT_CDAT):	$(MEDAL_SORT_CDAT)
	$(COPY)	$(MEDAL_SORT_CDAT) $(SRC_DIR)

