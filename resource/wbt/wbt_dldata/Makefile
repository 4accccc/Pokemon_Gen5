#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
#USERS	= tanaka_ryota	shimoyamada tamada
USERS	= shimoyamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = wbt_dldata

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
#TARGETDIR = ./out

WBT_EXE = wbt_dldata_conv.exe
WBT_CRC_EXE = my_crc_ccitt.exe

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
xlsources:=$(wildcard *.xls)

# リソースが吐き出すファイルを自動でつくる
xlsobjs = $(subst .xls,.bin,$(xlsources))

# testファイル
input_file  = test_all.xls
output_file = test_all.bin
output_file_tmp = test_all_tmp.bin
output_file_crc = test_all_crc.bin

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .xls .bin

#.xls.bin:
#	$(WBT_EXE) $(input_file)
#	$(WBT_CRC_EXE) $(output_file) $(output_file_crc)

# ruby版(しばらく残し)
#.xls.bin:
#	ruby wbt_dldata_conv.rb $(input_file)
#	ruby my_crc_ccitt.rb $(output_file) $(output_file_crc)
##	ruby my_crc_ccitt.rb $(output_file) $(output_file_tmp)
##	ruby my_crc_ccitt.rb $(output_file_tmp) $(output_file_crc) -t
##	rm $(output_file_tmp)

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

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする

# コピー先のnarcが古い場合実行する
#$(targetname).narc: $(output_file) $(output_file_crc)
$(targetname).narc: $(output_file_crc)
	nnsarc -r -l -n -i $(targetname) $^
	$(NAIXCUT) $(targetname).naix

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.narc
	rm -f *.naix
#	rm -f *.bin
	rm -f $(output_file)
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix

