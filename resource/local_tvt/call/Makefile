#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= toru_nagihashi nishino

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= local_tvt_call

#------------------------------------------------------------------------------
#依存するファイル名
#------------------------------------------------------------------------------
xlsname			= local_tvt_call_data
msgname 		= ..\..\..\prog\include\msg\msg_tvt_call.h
typename		= ..\..\..\prog\src\app\local_tvt\local_tvt_type_def.h
zonename		= ..\..\fldmapdata\zonetable\zone_id.h

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
TARGETDIR_H = ..\..\..\prog\src\app\local_tvt

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
do-build:	$(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname).naix $(TARGETDIR_H)/$(targetname).h

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)

$(TARGETDIR)/$(targetname).naix: $(targetname).naix
	$(COPY) $(targetname).naix $(TARGETDIR)

$(TARGETDIR_H)/$(targetname).h:	$(targetname).h
	$(COPY) $(targetname).h $(TARGETDIR_H)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc $(targetname).naix: $(targetname).dat
	nnsarc -r -l -n -i $(targetname) $^
	$(NAIXCUT) $(targetname).naix

$(targetname).dat $(targetname).h: $(xlsname).xls $(msgname) $(zonename)
	ruby ../../../tools/exceltool/xls2xml/tab_out_sheet.rb -n 0 -c $(xlsname).xls > $(xlsname).csv
	ruby ltvt_call_data_converter.rb $(xlsname).csv $(typename) $(msgname) $(zonename) $(targetname)
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
	rm -f *.csv
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


