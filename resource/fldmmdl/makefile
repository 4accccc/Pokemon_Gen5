#======================================================================
#	makefile
#	動作モデル　リストコンバート
#======================================================================
include mkdefine
include $(PROJECT_RSCDIR)/macro_define

MKFILE_RESLIST = mkfile_reslist
MKFILE_NCGIMD = mkfile_ncgimd
MKFILE_RESARC = mkfile_resarc
MKFILE_MDLLIST = mkfile_mdllist

.PHONY: do-build clean

do-build:
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	$(MAKE) -f $(MKFILE_RESLIST)
	$(MAKE) -f $(MKFILE_NCGIMD)
endif
	$(MAKE) -f $(MKFILE_RESARC)
	$(MAKE) -f $(MKFILE_MDLLIST)

clean:
	$(MAKE) -f $(MKFILE_MDLLIST) clean
	$(MAKE) -f $(MKFILE_RESARC) clean
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	$(MAKE) -f $(MKFILE_NCGIMD) clean
	$(MAKE) -f $(MKFILE_RESLIST) clean
endif

dmyres:
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	$(MAKE) -f $(MKFILE_RESLIST) dmyres
endif
