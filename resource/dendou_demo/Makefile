#------------------------------------------------------------------------------
#  zukan_graのnarcを作るmakefile  
#  直下のサブディレクトリ中のファイルに対してアーカイブを行います
#  2009.12.03
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= toyama hiro_nakamura

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = dendou_demo_gra

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR_ARC = $(PROJECT_ARCDIR)
TARGETDIR_H = $(PROJECT_ROOT)prog/src/demo/dendou_demo

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#------------------------------------------------------------------------------
#※ここでアーカイブしたい直下のサブディレクトリの名前を書く
#------------------------------------------------------------------------------
SUBDIRS_PTCL = ptcl
SUBDIRS_PTCL_BUG = ptcl/bug
SUBDIRS_PTCL_DARK = ptcl/dark
SUBDIRS_PTCL_DRAGON = ptcl/dragon
SUBDIRS_PTCL_ELEC = ptcl/elec
SUBDIRS_PTCL_FIGHT = ptcl/fight
SUBDIRS_PTCL_FIRE = ptcl/fire
SUBDIRS_PTCL_FLY = ptcl/fly
SUBDIRS_PTCL_GHOST = ptcl/ghost
SUBDIRS_PTCL_GRASS = ptcl/grass
SUBDIRS_PTCL_GROUND = ptcl/ground
SUBDIRS_PTCL_ICE = ptcl/ice
SUBDIRS_PTCL_NORMAL = ptcl/normal
SUBDIRS_PTCL_POISON = ptcl/poison
SUBDIRS_PTCL_PSYC = ptcl/psyc
SUBDIRS_PTCL_ROCK = ptcl/rock
SUBDIRS_PTCL_STEEL = ptcl/steel
SUBDIRS_PTCL_WARTER = ptcl/warter
SUBDIRS_PTCL_NAME = ptcl/name

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define

# 自動でリソースを集める
lsources:=$(wildcard *.ncl)
gsources:=$(wildcard *.ncg)
ssources:=$(wildcard *.nsc)
esources:=$(wildcard *.nce)
parsources1:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_BUG)))
parsources2:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_DARK)))
parsources3:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_DRAGON)))
parsources4:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_ELEC)))
parsources5:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_FIGHT)))
parsources6:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_FIRE)))
parsources7:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_FLY)))
parsources8:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_GHOST)))
parsources9:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_GRASS)))
parsources10:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_GROUND)))
parsources11:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_ICE)))
parsources12:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_NORMAL)))
parsources13:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_POISON)))
parsources14:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_PSYC)))
parsources15:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_ROCK)))
parsources16:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_STEEL)))
parsources17:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_WARTER)))
parsources99:=$(wildcard $(addsuffix /*.spr,$(SUBDIRS_PTCL_NAME)))

# リソースが吐き出すファイルを自動でつくる
lobjects = $(subst .ncl,.NCLR,$(lsources))
gobjects = $(subst .ncg,.NCGR,$(gsources))
cobjects = $(subst .nsc,.NSCR,$(ssources))
eobjects = $(subst .nce,.NCER,$(esources))
aobjects = $(subst .nce,.NANR,$(esources))


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .NCER .nce .spa .spt .spr

.ncl.NCLR:
	g2dcvtr $<

.ncg.NCGR:
	g2dcvtr $< -bg

.nsc.NSCR:
	g2dcvtr $<

.nce.NCER:
	g2dcvtr $< -br

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR_ARC)/$(targetname).narc $(TARGETDIR_H)/name_spa.h Makefile

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR_ARC)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR_ARC)
	$(COPY) $(targetname).naix $(TARGETDIR_ARC)

$(TARGETDIR_H)/name_spa.h: name_spa.h
	$(COPY) name_spa.h $(TARGETDIR_H)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(lobjects) $(gobjects) $(eobjects) $(cobjects) $(aobjects) bug.spa dark.spa dragon.spa elec.spa fight.spa fire.spa fly.spa ghost.spa grass.spa ground.spa ice.spa normal.spa poison.spa psyc.spa rock.spa steel.spa warter.spa name.spa
	nnsarc -r -l -n -i -f $(targetname) $^
	$(NAIXCUT) $(targetname).naix

bug.spa : $(parsources1)
	../../tools/SPRArchiver.exe -o bug_spa.h $(SUBDIRS_PTCL_BUG) $(SUBDIRS_PTCL_BUG) bug.spa

dark.spa : $(parsources2)
	../../tools/SPRArchiver.exe -o dark_spa.h $(SUBDIRS_PTCL_DARK) $(SUBDIRS_PTCL_DARK) dark.spa

dragon.spa : $(parsources3)
	../../tools/SPRArchiver.exe -o dragon_spa.h $(SUBDIRS_PTCL_DRAGON) $(SUBDIRS_PTCL_DRAGON) dragon.spa

elec.spa : $(parsources4)
	../../tools/SPRArchiver.exe -o elec_spa.h $(SUBDIRS_PTCL_ELEC) $(SUBDIRS_PTCL_ELEC) elec.spa

fight.spa : $(parsources5)
	../../tools/SPRArchiver.exe -o fight_spa.h $(SUBDIRS_PTCL_FIGHT) $(SUBDIRS_PTCL_FIGHT) fight.spa

fire.spa : $(parsources6)
	../../tools/SPRArchiver.exe -o file_spa.h $(SUBDIRS_PTCL_FIRE) $(SUBDIRS_PTCL_FIRE) fire.spa

fly.spa : $(parsources7)
	../../tools/SPRArchiver.exe -o fly_spa.h $(SUBDIRS_PTCL_FLY) $(SUBDIRS_PTCL_FLY) fly.spa

ghost.spa : $(parsources8)
	../../tools/SPRArchiver.exe -o ghost_spa.h $(SUBDIRS_PTCL_GHOST) $(SUBDIRS_PTCL_GHOST) ghost.spa

grass.spa : $(parsources9)
	../../tools/SPRArchiver.exe -o grass_spa.h $(SUBDIRS_PTCL_GRASS) $(SUBDIRS_PTCL_GRASS) grass.spa

ground.spa : $(parsources10)
	../../tools/SPRArchiver.exe -o ground_spa.h $(SUBDIRS_PTCL_GROUND) $(SUBDIRS_PTCL_GROUND) ground.spa

ice.spa : $(parsources11)
	../../tools/SPRArchiver.exe -o ice_spa.h $(SUBDIRS_PTCL_ICE) $(SUBDIRS_PTCL_ICE) ice.spa

normal.spa : $(parsources12)
	../../tools/SPRArchiver.exe -o normal_spa.h $(SUBDIRS_PTCL_NORMAL) $(SUBDIRS_PTCL_NORMAL) normal.spa

poison.spa : $(parsources13)
	../../tools/SPRArchiver.exe -o poison_spa.h $(SUBDIRS_PTCL_POISON) $(SUBDIRS_PTCL_POISON) poison.spa

psyc.spa : $(parsources14)
	../../tools/SPRArchiver.exe -o psyc_spa.h $(SUBDIRS_PTCL_PSYC) $(SUBDIRS_PTCL_PSYC) psyc.spa

rock.spa : $(parsources15)
	../../tools/SPRArchiver.exe -o rock_spa.h $(SUBDIRS_PTCL_ROCK) $(SUBDIRS_PTCL_ROCK) rock.spa

steel.spa : $(parsources16)
	../../tools/SPRArchiver.exe -o steel_spa.h $(SUBDIRS_PTCL_STEEL) $(SUBDIRS_PTCL_STEEL) steel.spa

warter.spa : $(parsources17)
	../../tools/SPRArchiver.exe -o warter_spa.h $(SUBDIRS_PTCL_WARTER) $(SUBDIRS_PTCL_WARTER) warter.spa

name.spa : $(parsources99)
	../../tools/SPRArchiver.exe -o name_spa.h $(SUBDIRS_PTCL_NAME) $(SUBDIRS_PTCL_NAME) name.spa

# -fオプションを付けて、パス名から作成したIDにする
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除

ifeq ($(aobjects),)
	rm -f $(aobjects)
endif
ifeq ($(eobjects),)
	rm -f $(eobjects)
endif
ifeq ($(cobjects),)
	rm -f $(cobjects)
endif
ifeq ($(gobjects),)
	rm -f $(gobjects)
endif
ifeq ($(lobjects),)
	rm -f $(lobjects)
endif

	rm -f *.N*R
	rm -f *.spa
	rm -f *.narc
	rm -f *.naix
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR_ARC)/$(targetname).narc
	rm -f $(TARGETDIR_ARC)/$(targetname).naix


