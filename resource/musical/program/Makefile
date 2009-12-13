#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= ariizumi_nobuhiko

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#コピー対象ディレクトリ
include copy_narc.txt

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname	= mus_prog_data

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
do-build:	subdir $(TARGETDIR)/mus_prog_data_01.narc $(TARGETDIR)/mus_prog_data_02.narc  $(TARGETDIR)/mus_prog_data_03.narc  $(TARGETDIR)/mus_prog_data_04.narc  $(TARGETDIR)/mus_prog_data_05.narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/mus_prog_data_01.narc: $(COPY_TARGET_PATH)/mus_prog_data_01.narc
	$(COPY) $(COPY_TARGET_PATH)/mus_prog_data_01.narc $(TARGETDIR)

$(TARGETDIR)/mus_prog_data_02.narc: $(COPY_TARGET_PATH)/mus_prog_data_02.narc
	$(COPY) $(COPY_TARGET_PATH)/mus_prog_data_02.narc $(TARGETDIR)

$(TARGETDIR)/mus_prog_data_03.narc: $(COPY_TARGET_PATH)/mus_prog_data_03.narc
	$(COPY) $(COPY_TARGET_PATH)/mus_prog_data_03.narc $(TARGETDIR)

$(TARGETDIR)/mus_prog_data_04.narc: $(COPY_TARGET_PATH)/mus_prog_data_04.narc
	$(COPY) $(COPY_TARGET_PATH)/mus_prog_data_04.narc $(TARGETDIR)

$(TARGETDIR)/mus_prog_data_05.narc: $(COPY_TARGET_PATH)/mus_prog_data_05.narc
	$(COPY) $(COPY_TARGET_PATH)/mus_prog_data_05.narc $(TARGETDIR)


#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	rm -f $(TARGETDIR)/mus_prog_data_01.narc
	rm -f $(TARGETDIR)/mus_prog_data_02.narc
	rm -f $(TARGETDIR)/mus_prog_data_03.narc
	rm -f $(TARGETDIR)/mus_prog_data_04.narc
	rm -f $(TARGETDIR)/mus_prog_data_05.narc


