#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tanaka_ryota	shimoyamada tamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = wbt_dldata

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)
#TARGETDIR = ./out

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
#lsources:=$(wildcard *.ncl)
#gsources:=$(wildcard *.ncg)
#csources:=$(wildcard *.nsc)
#esources:=$(wildcard *.nce)
xlsources:=$(wildcard *.xls)

# リソースが吐き出すファイルを自動でつくる
#lobjects = $(subst .ncl,.NCLR,$(lsources))
#gobjects = $(subst .ncg,.NCGR,$(gsources))
#cobjects = $(subst .nsc,.NSCR,$(csources))
#eobjects = $(subst .nce,.NCER,$(esources))
#aobjects = $(subst .nce,.NANR,$(esources))
xlsobjs = $(subst .xls,.bin,$(xlsources))

# testファイル
input_file  = test_all.xls
output_file = test_all.bin
output_file_tmp = test_all_tmp.bin
output_file_crc = test_all_crc.bin

XLS2TAB		= ruby $(PROJECT_ROOT)/tools/exceltool/xls2xml/tab_out_sheet.rb -c

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .NCER .nce .xls .bin

#.ncl.NCLR:
#	g2dcvtr $< -pcm
#
#.ncg.NCGR:
#	g2dcvtr $< -bg
#
#.nsc.NSCR:
#	g2dcvtr $<
#
#.nce.NCER:
#	g2dcvtr $< -br

.xls.bin:
	ruby wbt_dldata_conv.rb $(input_file)
	ruby my_crc_ccitt.rb $(output_file) $(output_file_crc)
#	ruby my_crc_ccitt.rb $(output_file) $(output_file_tmp)
#	ruby my_crc_ccitt.rb $(output_file_tmp) $(output_file_crc) -t
#	rm $(output_file_tmp)

#test_all.bin: test_all.xls
#	ruby wbt_dldata_conv.rb $<

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
#$(targetname).narc: $(lobjects) $(gobjects) $(eobjects) $(cobjects) $(aobjects) $(xlsobjs) $(output_file)
$(targetname).narc: $(lobjects) $(gobjects) $(eobjects) $(cobjects) $(aobjects) $(output_file) $(output_file_crc)
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
#	rm -f *.NANR
#	rm -f *.NCER
#	rm -f *.NSCR
#	rm -f *.NCGR
#	rm -f *.NCLR
	rm -f *.narc
	rm -f *.naix
	rm -f *.bin
#	rm -f *.csv
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


