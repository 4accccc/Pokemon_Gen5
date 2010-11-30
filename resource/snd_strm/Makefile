#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#
#		debug用サウンドストリームコンバートmakefile	
#		2010.11.08 Toru=Nagihashi
#
#		行っていること
#		・エクセルから、コンバートするwavのバッチファイル、アーカイブするファイルのリスト、ソースで使用するcdatを作成
#		・バッチファイルからコンバート
#		・アーカイブのリストからnarc作成
#		・アーカイブ、cdatをコピー
#
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= toru_nagihashi ichinose

#------------------------------------------------------------------------------
#※定義名
#------------------------------------------------------------------------------
targetname	= snd_strm
dataname 		= snd_strm_data
LIST_FILE 	= list.txt
CONV_BAT		= conv.bat
debugname		= snd_strm_debug

#------------------------------------------------------------------------------
#※コピー先へのパスを書く コピー先のフォルダは先に作成する必要があります。
#	グラフィックなら基本はgraphicに入れてください。
#	ただし、これは分類わけしたい、など理由がある場合は、
#	新たにarc/にフォルダを作成してください。
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define





##########################################
#ターゲットファイル
##########################################

WAVFILE = $(wildcard *.wav)
STRMFILE = $(WAVFILE:.wav=.strm)


.PHONY:	do-build clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build: $(TARGETDIR)/$(targetname).narc $(TARGETDIR)/$(targetname).naix $(TARGETDIR)/$(dataname).cdat	$(TARGETDIR)/$(debugname).cdat

# コピーを実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)

$(TARGETDIR)/$(targetname).naix: $(targetname).naix
	$(COPY) $(targetname).naix $(TARGETDIR)

$(TARGETDIR)/$(dataname).cdat: $(dataname).cdat
	$(COPY) $(dataname).cdat $(TARGETDIR)

$(TARGETDIR)/$(debugname).cdat: $(debugname).cdat
	$(COPY) $(debugname).cdat $(TARGETDIR)


# コピー先のnarcが古い場合実行する
ifeq	($(CONVERTUSER),true)
$(targetname).narc $(targetname).naix: $(STRMFILE) $(LIST_FILE)
	nnsarc -c -l -n -i $(targetname) -S $(LIST_FILE)
	$(NAIXCUT) $(targetname).naix

$(STRMFILE): $(WAVFILE)	$(CONV_BAT)
	$(CONV_BAT)

$(LIST_FILE) $(datname).cdat $(CONV_BAT) $(debugname).cdat: $(dataname).csv
	ruby snd_strm_data_conv.rb $(dataname).csv $(dataname).cdat $(LIST_FILE) $(CONV_BAT) $(debugname).cdat
	chmod 777 $(CONV_BAT)

$(dataname).csv: $(dataname).xls
	ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out.rb	-c $^ > $(dataname).csv
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
	@$(MAKE_SUBDIR)
	rm -f *.strm
	rm -f *.narc
	rm -f *.naix
	rm -f $(datname).cdat
	rm -f $(LIST_FILE)
	rm -f $(CONV_BAT)
	rm -f $(dataname).csv
endif
	rm -f $(TARGETDIR)/$(datname).cdat
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix

