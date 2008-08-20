#------------------------------------------------------------------------------
#
#	サンプルマップコンバート用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

include $(PROJECT_RSCDIR)\macro_define

targetname	= sample_map
TARGETDIR	= $(PROJECT_RSCDIR)\test_graphic


SUBDIRS	=	build


#とりあえず直下の*.imdを依存対象とする
MAPIMDS	:= $(wildcard *.imd)
MAPMDLS = $(subst .imd,.nsbmd,$(MAPIMDS))

.SUFFIXES:	.nsbmd .imd

#imdデータからの生成ルール
.imd.nsbmd:
	g3dcvtr $< -emdl
	g3dcvtr $< -etex


.PHONY:	do-build clean subdir

do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

clean:
	@$(MAKE_SUBDIR)
	-rm -f *.nsbtx *.nsbmd
	-rm -f $(targetname).narc
	-rm -f $(targetname).naix

$(TARGETDIR)/$(targetname).narc:	$(targetname).narc
	$(COPY)	$(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR)

$(targetname).narc:	$(MAPMDLS) $(wildcard *.bin) build/dummy_file
	nnsarc -c -l -n -i $(targetname).narc -S datalist.scr > error
	$(NAIXCUT) $(targetname).naix

