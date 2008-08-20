#------------------------------------------------------------------------------
#
#	サンプルマップコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

include $(PROJECT_RSCDIR)\macro_define

#	コンバート作業が必要なユーザーの名前を記述する
USERS	=	watanabe tamada

targetname	= sample_map
NARCNAME	= $(targetname).narc
NAIXNAME	= $(targetname).naix

TARGETDIR	= $(PROJECT_ARCDIR)/test_graphic


SUBDIRS	=	build


#とりあえず直下の*.imdを依存対象とする
MAPIMDS	:= $(wildcard *.imd)
MAPMDLS = $(subst .imd,.nsbmd,$(MAPIMDS))

.SUFFIXES:	.nsbmd .imd

ifneq	(,$(findstring $(USERNAME),$(USERS)))
#imdデータからの生成ルール
.imd.nsbmd:
	g3dcvtr $< -emdl
	g3dcvtr $< -etex
endif


.PHONY:	do-build clean subdir clean-local

do-build:	subdir $(TARGETDIR)/$(NARCNAME)

subdir:
	@$(MAKE_SUBDIR)

$(TARGETDIR)/$(NARCNAME):	$(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

$(NARCNAME):	$(MAPMDLS) $(wildcard *.bin) build/dummy_file
	nnsarc -c -l -n -i $(NARCNAME) -S datalist.scr > error
	$(NAIXCUT) $(NAIXNAME)

clean-local:
ifneq	(,$(findstring $(USERNAME),$(USERS)))
	-rm -f *.nsbtx *.nsbmd
	-rm -f $(NARCNAME)
	-rm -f $(NAIXNAME)
endif

clean:	clean-local
	@$(MAKE_SUBDIR)
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)

