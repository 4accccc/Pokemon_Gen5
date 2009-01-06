#! make -f
#----------------------------------------------------------------------------
# Project:  NitroCrypto - build - buildtools
# File:     Makefile
#
# Copyright 2006 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: Makefile,v $
# Revision 1.5  2006/02/27 03:57:16  okubata_ryoma
# undo
#
# Revision 1.3  2006/02/27 00:47:28  okubata_ryoma
# Copyrightの修正
#
# Revision 1.2  2006/02/24 07:08:39  okubata_ryoma
# small fix
#
# Revision 1.1  2006/02/24 01:16:05  okubata_ryoma
# NitroCrypto の新設
#
#
# $NoKeywords: $
#----------------------------------------------------------------------------

include	$(NITROSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------
#  install したライブラリが clobber されるまえに commondefs.add-ins.* を
#  削除すると不都合が起こるので、該当ファイルがあるなら clobber 動作を中止
#
ifeq	($(MAKECMDGOALS),clobber)
FILE_TESTEE	= $(NITROCRYPTO_LIBDIR)/$(firstword $(NITROCRYPTO_LIBS))
MUST_WAIT	= $(shell test -e $(FILE_TESTEE) && echo TRUE)
endif

ifeq	($(MUST_WAIT),TRUE)
INSTALL_TARGETS	=
INSTALL_DIR	=
else
INSTALL_TARGETS	= commondefs.add-ins.crypto
INSTALL_DIR	= $(NITRO_INSTALL_ROOT)/build/buildtools
endif

#----------------------------------------------------------------------------

include	$(NITROSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
