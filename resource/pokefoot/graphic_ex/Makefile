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
			pokefoot_ex.narc pokefoot_ex.naix

# 読み込むファイル
input_personal            = $(PROJECT_RSCDIR)/personal/personal_wb.csv

#----------------------------------------------------------------------------

include	$(NITROSYSTEM_ROOT)/build/buildtools/modulerules

.suffixes:.nce.ncl.NCER.NCLR

do-build:pokefoot_ex.narc

#do-build: $(POKEGRA:.nce=.NCER)
%.NCER: %.nce
	g2dcvtr $<
	ntrcomp -l 2 -o $*_lzh.NCGR $*.NCGR
	mv $*_lzh.NCGR $*.NCGR

#do-build: pokefoot_ex.narc

pokefoot_ex.narc: $(POKEGRA:.nce=.NCER) $(POKECOL:.ncl=.NCLR) pokefoot_ex.scr
	nnsarc -c -l -n -i pokefoot_ex.narc -S pokefoot_ex.scr
	xcopy /d /y pokefoot_ex.narc .\\..\\
	xcopy /d /y pokefoot_ex.naix .\\..\\


#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	@$(MAKE_SUBDIR)
endif
	rm -f *.N??R
