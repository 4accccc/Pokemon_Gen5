#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= toru_nagihashi tetsuji_ohta kawada_koji shimoyamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = townmap_data
targetname_b = $(targetname)_version_black
targetname_w = $(targetname)_version_white
targetname2	= townmap_replace.cdat
#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR_ARC	= $(PROJECT_ARCDIR)#アーカイブ用
TARGETDIR_H		= $(PROJECT_ROOT)/prog/include/app#ヘッダ用

#依存するファイル
MESSAGE_GMM = ..\..\message\dst\msg_townmap.h
ZONETABLE_XLS = ..\..\fldmapdata\zonetable\zonetable.xls

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR_ARC)/$(targetname_b).narc $(TARGETDIR_ARC)/$(targetname_w).narc $(TARGETDIR_H)/$(targetname).h $(TARGETDIR_ARC)/$(targetname2)

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR_ARC)/$(targetname_b).narc: $(targetname_b).narc
	$(COPY) $(targetname_b).narc $(TARGETDIR_ARC)

$(TARGETDIR_ARC)/$(targetname_w).narc: $(targetname_w).narc
	$(COPY) $(targetname_w).narc $(TARGETDIR_ARC)

$(TARGETDIR_H)/$(targetname).h: $(targetname).h
	$(COPY) $(targetname).h		 $(TARGETDIR_H) 

$(TARGETDIR_ARC)/$(targetname2): $(targetname2)
	$(COPY) $(targetname2)		 $(TARGETDIR_ARC) 

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname_b).narc: $(targetname_b).dat
	nnsarc -r -l -n -i $(targetname_b) $^

$(targetname_w).narc: $(targetname_w).dat
	nnsarc -r -l -n -i $(targetname_w) $^

$(targetname_w).dat $(targetname_b).dat $(targetname2): townmap_data.xls $(MESSAGE_GMM) $(ZONETABLE_XLS)
	perl townmap_data_conv.pl townmap_data.xls

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.dat
	rm -f *.h
	rm -f $(targetname_b).narc
	rm -f $(targetname_w).narc
	rm -f townmap_error.txt
endif
	rm -f $(TARGETDIR_ARC)/$(targetname_b).narc
	rm -f $(TARGETDIR_ARC)/$(targetname_w).narc


