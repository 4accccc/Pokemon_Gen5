#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tomoya_takahashi nakatsui takahashi hozumi_yukiko iwao_kazumasa murakami_naoto nakamura_akira suginaka_katsunori #tamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = field_rail_data

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
RAILFILE = $(wildcard *.rail)
ATTRFILE = $(wildcard *.attr)
OBJ_FILE = $(RAILFILE:.rail=.dat) $(ATTRFILE:.attr=.atdat)

LIST_FILE 	= list.txt

script = $(PROJECT_ROOT)\tools\rail_editor\rail_conv.pl
attrscript = $(PROJECT_ROOT)\tools\rail_editor\rail_attr_conv.pl

#------------------------------------------------------------------------------
#ヘッダ参照先
#------------------------------------------------------------------------------
HEADERDIR = $(PROJECT_PROGDIR)src/field/

HEADERDUMMY_TARGET = headerdummy.txt

#------------------------------------------------------------------------------
#ヘッダコピー先
#------------------------------------------------------------------------------
RAILHEADER_COPY = $(PROJECT_PROGDIR)src/field/rail_line/

#------------------------------------------------------------------------------
#	#サフィックス
#	#コンバート対象者のみ、コンバートのルールを有効にする
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	


.SUFFIXES: .rail .dat .attr .atdat

#コンバート環境
.rail.dat:
	@perl $(script) $< $(HEADERDIR)field_rail_loader_func.h $(HEADERDIR)field_rail.h
	@ruby $(PROJECT_ROOT)\tools\binlinker.rb $*.ofs $*.point $*.line $*.camera $*.linepos $@ RL

.attr.atdat:
	@perl $(attrscript) $<
	

endif



.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)


#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
ifeq	($(CONVERTUSER),true)	


# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(HEADERDUMMY_TARGET) $(targetname).narc 
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)
	$(COPY) *.h $(RAILHEADER_COPY)


# コピー先のnarcが古い場合実行する
$(targetname).narc: $(OBJ_FILE)
	-ls *.dat > $(LIST_FILE)
	-ls *.atdat >> $(LIST_FILE)
	$(ARCLIST_MAKE) $(LIST_FILE)
	nnsarc -c -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix
	-rm	$(LIST_FILE)
	rm -f *.ofs
	rm -f *.point
	rm -f *.line
	rm -f *.linepos
	rm -f *.camera

$(HEADERDUMMY_TARGET):$(HEADERDIR)field_rail_loader_func.h $(HEADERDIR)field_rail.h
	rm -f *.dat
	echo このファイルは、headerが更新されたときにdatを作り直すために用意されています。 > $(HEADERDUMMY_TARGET)

else


# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc 
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)
	$(COPY) *.h $(RAILHEADER_COPY)

#アーカイブのみ生成
$(targetname).narc: $(OBJ_FILE)
	-ls *.dat > $(LIST_FILE)
	-ls *.atdat >> $(LIST_FILE)
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
	rm -f $(targetname).narc
	rm -f $(targetname).naix

#コンバート対象者のみ、中間ファイル削除
ifeq	($(CONVERTUSER),true)	
	rm -f *.dat
	rm -f *.atdat
endif




