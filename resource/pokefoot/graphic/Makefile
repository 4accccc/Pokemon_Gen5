##-----------------------------------------------------------------------------
#**
#*			足跡グラフィックデータアーカイブメイク
#**
##-----------------------------------------------------------------------------

USERS	= matsuda kawada_koji

SUBDIRS		=

POKEGRA:=$(wildcard *.nce)
POKECOL:=$(wildcard *.ncl)

RES_DIR	= .

include	$(PROJECT_RSCDIR)\macro_define
include	$(COMMONDEFS_CCTYPE_CW)


#Makeで生成される*.bin/*.narcはmake cleanの削除対象にする
LDIRT_CLEAN	= $(POKEGRA:.nce=.NCGR) \
			$(POKEGRA:.nce=.NCER) \
			$(POKEGRA:.nce=.NANR) \
			$(POKECOL:.ncl=.NCLR) \
			pokefoot.narc pokefoot.naix

#----------------------------------------------------------------------------

include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

.suffixes:.nce.ncl.NCER.NCLR

do-build:pokefoot.narc

poke_foot_000.NCER: poke_foot_000.nce
	g2dcvtr poke_foot_000.nce
	ntrcomp -l 2 -o poke_foot_000_lzh.NCGR poke_foot_000.NCGR
	mv poke_foot_000_lzh.NCGR poke_foot_000.NCGR
#	ntrcomp -l 2 -o poke_foot_000_lzh.NCER poke_foot_000.NCER
#	mv poke_foot_000_lzh.NCER poke_foot_000.NCER
#	ntrcomp -l 2 -o poke_foot_000_lzh.NANR poke_foot_000.NANR
#	mv poke_foot_000_lzh.NANR poke_foot_000.NANR

#do-build: $(POKEGRA:.nce=.NCER)
%.NCER: %.nce
	g2dcvtr $<
	ntrcomp -l 2 -o $*_lzh.NCGR $*.NCGR
	mv $*_lzh.NCGR $*.NCGR

#do-build: $(POKECOL:.ncl=.NCLR)
%.NCLR: %.ncl
	g2dcvtr $<

#do-build: pokefoot.narc
pokefoot.narc: $(POKEGRA:.nce=.NCER) $(POKECOL:.ncl=.NCLR) pokefoot.scr
	nnsarc -c -l -n -i pokefoot.narc -S pokefoot.scr
	xcopy /d /y pokefoot.narc .\\..\\
	xcopy /d /y pokefoot.naix .\\..\\


#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	@$(MAKE_SUBDIR)
endif
	rm -f *.N??R
