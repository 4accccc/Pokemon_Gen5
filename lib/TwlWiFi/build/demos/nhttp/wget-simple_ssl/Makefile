#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - demos.TWL
# File:     Makefile
#
# Copyright 2007-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-08#$
# $Rev: 1183 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------
TARGET_PLATFORM = TWL NITRO
TWL_ARCHGEN     = ALL
TARGET_BIN	    = main.srl

SRCS		    = main.c test_cainfo.c

LLIBRARIES	    = libnhttp$(TWIFI_LIBSUFFIX_EX).a
#----------------------------------------------------------------------------

include	$(TWLWIFI_ROOT)/build/buildtools/commondefs.demos

TWLWIFI_LIBNSSL =

do-build:	$(TARGETS)

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules

%.der: %.crt
	openssl x509 -in $< -outform der -out $(basename $@).der

src/%_cainfo.c: etc/%.der
	$(TWIFI_ROOT)/tools/bin/certconv -n ca -o $@ $<

#===== End of Makefile =====
