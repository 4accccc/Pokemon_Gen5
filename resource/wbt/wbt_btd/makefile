
include $(PROJECT_RSCDIR)\macro_define


SUBDIRS	= work1a work1n work2a work2n work3a work3n rental

PROJECT_TOOLSDIR	= $(PROJECT_ROOT)/tools
TRAINER_HASH	= $(PROJECT_TOOLSDIR)/btl_subway/bts_tr_arr.rb
TRAINER_TXT		= $(PROJECT_TOOLSDIR)/btl_subway/trainer.txt 

.PHONY:	do-build clean

do-build:	$(TRAINER_HASH)
	@$(MAKE_SUBDIR)

clean:
	@$(MAKE_SUBDIR)


$(TRAINER_HASH):	$(TRAINER_TXT)
	@echo "Rebuild Trainer Name Hash Table"
	ruby $(PROJECT_TOOLSDIR)/btl_subway/bts_trainer_arr.rb $(TRAINER_TXT) > $(TRAINER_HASH)

