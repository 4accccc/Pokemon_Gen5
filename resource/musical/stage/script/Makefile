#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= ariizumi_nobuhiko tanoue_reiko iwao_kazumasa

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= 

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
esfres:=$(wildcard *.esf)

# リソースが吐き出すファイルを自動でつくる
scriptobj = $(subst .esf,.bin,$(esfres))
baseobj = $(addprefix ../,$(scriptobj))


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .esf .s .bin

.esf.s:
	ruby ../../eescmk.rb c:/home/pokemon_wb/prog/src/musical/stage/script/musical_script_command.h $*.esf c:/home/pokemon_wb/

.s.bin:
	/cygdrive/c/Program\ Files/Freescale/CW\ for\ NINTENDO\ DSi\ V1.1/ARM_Tools/Command_Line_Tools/mwasmarm.exe $< -o $*.o
	/cygdrive/c/Program\ Files/Freescale/CW\ for\ NINTENDO\ DSi\ V1.1/ARM_Tools/Command_Line_Tools/mwldarm.exe -dis -o $*.elf $*.o
	../../../../tools/elf2bin $*.elf
	$(COPY) $*.bin ../
endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
do-build:	subdir esf
else
do-build:	subdir
endif
subdir:
	@$(MAKE_SUBDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする

esf: $(notdir $(esfres:.esf=.s)) $(notdir $(esfres:.esf=.bin))

endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f *.bin
	rm -f *.elf
	rm -f *.o
	rm -f *.s
	rm -f ../*.bin
endif


