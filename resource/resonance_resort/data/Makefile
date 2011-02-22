#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= nagihashi_toru shimoyamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetall			= resonance_resort_data
targetpeople	= resort_people
targetpeopleh	= resort_people_def

#------------------------------------------------------------------------------
#依存するファイル名
#------------------------------------------------------------------------------
xlsname			= resonance_resort_data
msgfile 		= ..\..\..\prog\include\msg\script/msg_c16r0101.h #TODO
scriptfile	= ..\..\fldmapdata\script\c16r0101_def.h #TODO

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR_ARC = $(PROJECT_ARCDIR)
TARGETDIR_PROG = ..\..\..\prog\src/field/resonance_resort/

#------------------------------------------------------------------------------
#TOOLS
#------------------------------------------------------------------------------
XLSCONV 	= $(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe
PEOPLECONV = resort_people_converter.rb

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= 

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

.PHONY:	do-build clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	$(TARGETDIR_ARC)/$(targetall).narc $(TARGETDIR_ARC)/$(targetall).naix $(TARGETDIR_PROG)/$(targetpeopleh).h

# narc のコピーだけ実行する
$(TARGETDIR_ARC)/$(targetall).narc: $(targetall).narc
	$(COPY) $(targetall).narc $(TARGETDIR_ARC)

$(TARGETDIR_ARC)/$(targetall).naix: $(targetall).naix
	$(COPY) $(targetall).naix $(TARGETDIR_ARC)

$(TARGETDIR_PROG)/$(targetpeopleh).h:	$(targetpeopleh).h
	$(COPY) $(targetpeopleh).h $(TARGETDIR_PROG)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetall).narc $(targetall).naix: $(targetpeople).dat
	nnsarc -r -l -n -i $(targetall) $^
	$(NAIXCUT) $(targetall).naix

#人物データコンバート
$(targetpeople).dat $(targetpeopleh).h: $(xlsname).xls $(msgfile) $(scriptfile) $(PEOPLECONV)
	$(XLSCONV) -o $(targetpeople)_data.csv -p 1 -s csv $(xlsname).xls
	$(XLSCONV) -o $(targetpeople)_define.csv -p 2 -s csv $(xlsname).xls
	rm -f *.dat
	ruby $(PEOPLECONV) $(targetpeople)_data.csv $(targetpeople)_define.csv $(msgfile) $(scriptfile) $(targetpeople) $(targetpeopleh)
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
	rm -f $(targetall).narc
	rm -f $(targetall).naix
	rm -f $(targetpeople)_data.csv
	rm -f $(targetpeople)_define.csv
endif
	rm -f $(TARGETDIR_ARC)/$(targetall).narc
	rm -f $(TARGETDIR_ARC)/$(targetall).naix


