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
MKFILE_ANMLIST = mkfile_anmlist
MKFILE_MMDLLIST = mkfile_scr_mdllist

SUBDIRS	= mmdl_pokedata

.PHONY: do-build clean

do-build:
	$(MAKE) -f $(MKFILE_ANMLIST)
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	$(MAKE) -f $(MKFILE_RESLIST)
	$(MAKE) -f $(MKFILE_NCGIMD)
endif
	$(MAKE) -f $(MKFILE_RESARC)
	$(MAKE) -f $(MKFILE_MDLLIST)
	@$(MAKE_SUBDIR)

clean:
	$(MAKE) -f $(MKFILE_MDLLIST) clean
	$(MAKE) -f $(MKFILE_RESARC) clean
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	$(MAKE) -f $(MKFILE_NCGIMD) clean
	$(MAKE) -f $(MKFILE_RESLIST) clean
endif
	$(MAKE) -f $(MKFILE_ANMLIST) clean
	@$(MAKE_SUBDIR)

dmyres:
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	$(MAKE) -f $(MKFILE_ANMLIST)
	$(MAKE) -f $(MKFILE_RESLIST) dmyres
endif

scrlist:
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	$(MAKE) -f $(MKFILE_MMDLLIST)
endif

ncg:
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	$(MAKE) -f $(MKFILE_NCGIMD)
endif
	$(MAKE) -f $(MKFILE_RESARC)

list:
	$(MAKE) -f $(MKFILE_ANMLIST)
ifeq	($(CONVERTUSER),true) #CONVERTUSER
	$(MAKE) -f $(MKFILE_RESLIST)
	$(MAKE) -f $(MKFILE_NCGIMD)
endif
	$(MAKE) -f $(MKFILE_RESARC)
	$(MAKE) -f $(MKFILE_MDLLIST)
	@$(MAKE_SUBDIR)

#アニメ定義のみコンバート
anm:
	$(MAKE) -f $(MKFILE_ANMLIST)

clean_anm:
	$(MAKE) -f $(MKFILE_ANMLIST) clean
