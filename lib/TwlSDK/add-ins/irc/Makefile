#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - add-ins - irc
# File:     Makefile
#
# Copyright 2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-07-22#$
# $Rev: 10936 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

include		$(TWLSDK_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

SUBDIRS 	= build

#----------------------------------------------------------------------------

include		$(TWLSDK_ROOT)/build/buildtools/modulerules


TWL_ADDIN_IRC_ROOT     ?= $(TWLSDK_ROOT)/add-ins/irc

PACKING_DATE           ?= $(shell date +%y%m%d)
TWLSDK_ADDIN_NAME      ?= TwlSDK-IRC-$(PACKING_DATE)
TWLSDK_ADDIN_ROOT       = $(call cygpathu,$(TWL_ADDIN_IRC_ROOT))
ADDIN_PACKAGE           = $(TWLSDK_ADDIN_ROOT)/$(TWLSDK_ADDIN_NAME).zip

zip:
	make make-archive

make-archive:
	@echo --- $@ ---
	@-rm -f $(ADDIN_PACKAGE)
	@( cd ..; \
	  for i in `cat $(TWLSDK_ADDIN_ROOT)/Files.lst` ; do	\
		if [ ! -e $i ]	; then 				\
			echo 'Error: Missing file: '$$i;	\
			echo '';				\
			exit 1; 				\
		fi 						\
	  done; 						\
	  cat $(TWLSDK_ADDIN_ROOT)/Files.lst | zip -r@ $(ADDIN_PACKAGE) )

#----------------------------------------------------------------------------
#===== End of Makefile =====
