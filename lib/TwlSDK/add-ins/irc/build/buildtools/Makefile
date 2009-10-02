#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - IRC - buildtools
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
# $Date:: 2009-04-16#$
# $Rev: 10419 $
# $Author: yosizaki $
#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------
#  install したライブラリが clobber されるまえに commondefs.add-ins.* を
#  削除すると不都合が起こるので、該当ファイルがあるなら clobber 動作を中止
#
ifeq	($(MAKECMDGOALS),clobber)
FILE_TESTEE	= $(TWL_ADDIN_IRC_LIBDIR)/$(firstword $(TWL_ADDIN_IRC_LIBDIR))
MUST_WAIT	= $(shell test -e $(FILE_TESTEE) && echo TRUE)
endif

ifeq	($(MUST_WAIT),TRUE)
INSTALL_TARGETS	=
INSTALL_DIR	=
else
INSTALL_TARGETS	= commondefs.add-ins.irc
INSTALL_DIR	= $(ROOT)/build/buildtools
endif

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
