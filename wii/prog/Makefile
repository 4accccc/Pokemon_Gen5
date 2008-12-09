#! make -f
#----------------------------------------------------------------------------
# Project:  NW4R - SystemLib - build - demos - db - directPrint
# File:     Makefile
#
# Copyright (C)2005-2008 Nintendo  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Revision: 1.10 $
#----------------------------------------------------------------------------

NW4R_USELIBS	:= SystemLib LayoutLib

SUBDIRS		=


#----------------------------------------------------------------------------

#SRCDIR		=	./src \
#				./src/lyt_demolib \
#				./src/network \
#				./src/network/tool \
#				./src/network/device \
#				./src/recdemo


REVOEX_ROOT	=	C:/RVL/RevoEX
LINCLUDES	:=	$(SDK_ROOT)/include \
				$(REVOEX_ROOT)/include
LLIBRARY_DIRS	:=	$(REVOEX_ROOT)//RVL/lib
LLIBRARIES		:=	mpD.a \
					mpdlD.a \
					mpdsD.a \
					mpfsD.a \
					ncdD.a \
					netD.a \
					nhttpD.a \
					ntrD.a \
					nwc24D.a \
					soD.a \
					sslD.a \
					vfD.a \
					wdD.a 
					


INCDIR		:=	./include \
				./lyt_demolib \
				./rexdemo 


SRCS		=	draw.cpp \
				mpdsmodel.c \
				ds_net.c \
				crc.c \
				lyt_demolib/lyt_heap.cpp \
				lyt_demolib/lyt_pad.cpp \
				recdemo/config.c \
				recdemo/demokpad.c \
				recdemo/graphic.c \
				recdemo/heap.c \
				recdemo/netconfig.c \
				recdemo/netfunc.c \
				network/net.c \
				network/net_command.c \
				network/net_handle.c \
				network/net_system.c \
				network/tool/net_queue.c \
				network/tool/net_ring_buff.c


TARGET_BIN	=	main.elf

#SRCDIR		=	# using default
#LCFILE		=	# using default

include	$(NW4R_ROOT)/Library/build/buildtools/commondefs


#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(NW4R_ROOT)/Library/build/buildtools/modulerules


#===== End of Makefile =====
