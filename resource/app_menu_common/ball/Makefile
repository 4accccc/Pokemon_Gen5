#------------------------------------------------------------------------------
#	リソースのコンバートを行い、中間ファイルを1つ前のディレクトリにコピーする。
#	- 一つ前のディレクトリのmakefileで以下の処理をいれる必要があります。
#	-- このmakefileをコンバート対象者しか読み込まないように記述。
#	-- .narc生成の依存対象にこのフォルダ内のリソースを追加。（wildcardで指定)
#  2009.10.07 genya hosaka
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= 

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
ssources:=$(wildcard *.nsc)
esources:=$(wildcard *.nce)

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
sobjects = $(subst .nsc,.NSCR,$(ssources))
cobjects = $(subst .nce,.NCGR,$(esources))
eobjects = $(subst .nce,.NCER,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))

RES_OBJ =  $(lobjects) $(gobjects) $(sobjects) $(eobjects) $(aobjects) $(cobjects)

# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .NCER .nce

.ncl.NCLR:
	g2dcvtr $<
	CP $*.NCLR ./../

.ncg.NCGR:
	g2dcvtr $< -bg
	CP $*.NCGR ./../

.nsc.NSCR:
	g2dcvtr $<
	CP $*.NSCR ./../

.nce.NCER:
	g2dcvtr $< -ncn
	CP $*.NCER ./../
	CP $*.NANR ./../

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(RES_OBJ)

subdir:
	@$(MAKE_SUBDIR)


#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
	rm -f *.NCGR
	rm -f *.NCLR
	rm -f *.NCER
	rm -f *.NANR



