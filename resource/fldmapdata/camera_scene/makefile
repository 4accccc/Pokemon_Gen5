#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tomoya_takahashi iwao_kazumasa masafumi_saitou

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = grid_camera_scene

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)fieldmap/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

##########################################
#コンバートファイル
##########################################
FILE_XLS = $(wildcard *.xls)
FILE_BIN = $(FILE_XLS:.xls=.dat)


##########################################
#インクルードヘッダー
##########################################
FLD_SCENE_LOADER_HEADER = $(PROJECT_PROGDIR)src/field/fld_scenearea_loader_func.h
FLD_SCENE = $(PROJECT_PROGDIR)src/field/fld_scenearea.h 


##########################################
#ターゲットファイル
##########################################
LIST_FILE 	= list.txt

#------------------------------------------------------------------------------
#	#サフィックス
#	#コンバート対象者のみ、コンバートのルールを有効にする
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	

.SUFFIXES: .xls .dat

.xls.dat:
	#エクセルデータをタブ区切りにする。
	$(PROJECT_ROOT)tools/exceltool/ExcelSeetConv.exe -o $*.tab -p 0 -s tab $<
	#テキストデータ生成
	perl conv.pl $*.tab $*.sa
	#binary化
	perl $(PROJECT_ROOT)tools/rail_editor/scenearea_conv.pl $*.sa $(FLD_SCENE_LOADER_HEADER) $(FLD_SCENE)



endif



.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)


#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(FILE_BIN)
	
	-ls *.dat > $(LIST_FILE)
	$(ARCLIST_MAKE) $(LIST_FILE)

	nnsarc -c -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix
	-rm	$(LIST_FILE)
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
	rm -f *.dat
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif





