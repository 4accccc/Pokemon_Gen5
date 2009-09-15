#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - MB
# File:     Makefile
#
# Copyright 2004-2009 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2009-06-04#$
# $Rev: 10698 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS			 =

#----------------------------------------------------------------------------
# 子機として配信可能なのは NITRO もしくは TWL-HYBRID ビルドした srl
# MB ライブラリが配信するのは NITRO アプリですので、
# 配信先の本体が TWL であっても HYBRID ロムは NTR モードでブートします。
#
# 詳しくは、関数リファレンスの MB ライブラリの概要をご覧下さい。
TARGET_PLATFORM	:=	ALL
TWL_ARCHGEN		:=	HYBRID

INCDIR			 =	../common/include ./include
SRCDIR			 =	./src ../common/src 

SRCS			 =	main.c		\
					font.c		\
					disp.c		\
					common.c	\
					gmain.c

SHARED_WH		:= TRUE

ifdef	SHARED_WH
# WH モジュール は wireless_shared へ移動しました.

WH_DIR			 =	$(ROOT)/build/demos/wireless_shared/wh
SRCDIR			+=	$(WH_DIR)
INCDIR			+=	$(WH_DIR)
SRCS			+=	$(WH_DIR)/wh.c
MACRO_FLAGS		+= -DSHARED_WH

else
# 従来のソースコードもまだ残してあります.

SRCS			+=	wmhigh.c

endif	#SHARED_WH


TARGET_BIN		 =	child.srl

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#-------------------------------#
#   install target		#
#-------------------------------#
INSTALL_TARGETS	 =	$(TARGETS)
INSTALL_DIR		 = ../data

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
