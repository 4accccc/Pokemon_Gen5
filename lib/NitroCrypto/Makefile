#! make -f
#----------------------------------------------------------------------------
# Project:  NitroCrypto
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
# Revision 1.4  2006/03/08 10:44:36  okubata_ryoma
# attach, detachコマンド追加
#
# Revision 1.3  2006/02/24 07:46:06  okubata_ryoma
# check_codeの追加
#
# Revision 1.2  2006/02/24 02:47:57  okubata_ryoma
# zipファイル作成コードをrelease以下に移動
#
# Revision 1.1  2006/02/24 01:27:24  okubata_ryoma
# NitroCrypto の新設
#
#
# $NoKeywords: $
#----------------------------------------------------------------------------

include	$(NITROSDK_ROOT)/build/buildtools/commondefs

SUBDIRS = build

include	$(NITROSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------
.PHONY:	indent-all do-indent do-rename do-diff

SDKINDENT		 = $(NITRO_TOOLSDIR)/bin/sdkindent
SDKDIFF			 = $(NITRO_TOOLSDIR)/bin/sdkindent.libdiff
SDKDIR_BUILD		?= build
SDKDIR_INCLUDE		 = include
#SDKDIR_BUILDOPT	 = LD=true MAKEROM=true MAKELCF=true
SDKDIR_BUILDOPT		 = 
SDKCOPYRIGHT		 = $(NITRO_TOOLSDIR)/bin/sdkindent.copyright

indent-all:
	( export LMACRO_FLAGS=-DSDK_NO_MESSAGE=1;		\
	  export NITRO_NO_OPT_G=1;				\
	  $(MAKE) -kB $(SDKDIR_BUILDOPT) -C $(SDKDIR_BUILD) $(SDKINDENT_BUILD);	\
	  $(REMAKE) do-rename;					\
	  $(REMAKE) do-indent;					\
	  $(REMAKE) do-copyright;				\
	  $(MAKE) -skB $(SDKDIR_BUILDOPT) -C $(SDKDIR_BUILD) $(SDKINDENT_BUILD); \
	  $(REMAKE) do-diff )

indent-sub:
	( export LMACRO_FLAGS=-DSDK_NO_MESSAGE=1;		\
	  export NITRO_NO_OPT_G=1;				\
	  $(REMAKE) do-indent;					\
	  $(MAKE) -skB -C $(SDKDIR_BUILD) $(SDKINDENT_BUILD);	\
	  $(REMAKE) do-diff )

indent-diff:
	  $(REMAKE) do-diff

do-indent:
	@echo ===== INDENT STAGE =====
	/bin/find $(SDKDIR_BUILD)   -type f \
		-name '*.[ch]' ! -name '*.yy.c' ! -name '*.yy.patched.c' \
		-size -128k -print -exec $(SDKINDENT) {} \;
	/bin/find $(SDKDIR_INCLUDE) -type f \
		-name '*.[ch]' ! -name '*.yy.c' ! -name '*.yy.patched.c' \
		-size -128k -print -exec $(SDKINDENT) {} \;

do-copyright:
	@echo ===== COPYRIGHT STAGE =====
	/bin/find $(SDKDIR_BUILD)   -type f \
		-name '*.[ch]' ! -name '*.yy.c' ! -name '*.yy.patched.c' \
		-size -128k -print \
		-mtime -`/bin/date +%j` \
		-exec $(SDKCOPYRIGHT) {} \;
	/bin/find $(SDKDIR_INCLUDE) -type f \
		-name '*.[ch]' ! -name '*.yy.c' ! -name '*.yy.patched.c' \
		-size -128k -print \
		-mtime -`/bin/date +%j` \
		-exec $(SDKCOPYRIGHT) {} \;

do-rename:
	@echo ===== RENAME STAGE =====
	/bin/find $(SDKDIR_BUILD) -type f -name '*.o' -exec $(MV) {} {}.backup \;

do-diff:
	@echo ===== COMPARE STAGE =====
	/bin/find $(SDKDIR_BUILD) -type f -name '*.o' -exec $(SDKDIFF) {} {}.backup \;

#----------------------------------------------------------------------------
.PHONY:	check_code check_lang_en check_copyright_date_2006

CHECKEES	= build include

check_code: check_lang_en check_copyright_date_2006

#
#  Check backslash code for language EN
#	Inverse result code of grep (e.g. grep returns 1 if not found)
#	 -> Error if grep finds 0x5c(\) on 2nd byte of 16bit-wide 
#           Japanese kanji code at the end of line
#
check_lang_en:
	@echo --- $@ ---
	-@! grep -I -n -r --exclude='*.txt' "[^[:space:] -~][\]$$" $(CHECKEES)

#
#  Check Copyright date for 2006
#
check_copyright_date_2006:
	@echo --- $@ ---
	-@for dir in $(CHECKEES); do \
	    /bin/find $$dir \
		\! \( -type d \( -name '_*' -o \
				 -name CVS  -o \
				 -exec /bin/test -e {}/_ \; \) -prune \) \
		-type f \
		\! -name '*.exe' \! -name '*.[dao]' \! -name '*.sbin' \
		\! -name '*.nlf' \! -name '*.nef'   \! -name '*.xMAP' \
		-mtime -`/bin/date +%j` \
		-exec grep -q 'Copyright [0-9 ,-]* Nintendo' {} \; \
		\! -exec grep -q 'Copyright [0-9 ,-]*2006' {} \; \
		-printf "Error: Copyright Date: %p\n"; \
	done


EMPTY	 		:=
SPACE 			:= $(EMPTY) $(EMPTY)
NITROSDK_ROOT_		?= c:/NitroSDK
NITROSDK_ROOT		?= $(subst $(SPACE),\ ,$(subst \,/,$(NITROSDK_ROOT_)))
NITROCRYPTO_ROOT	?= $(NITROSDK_ROOT)/../NitroCrypto

.PHONY:	attach detach

attach: 
	make -C ${NITROCRYPTO_ROOT}/build/buildtools full

detach: 
	make -C ${NITROCRYPTO_ROOT}/build/buildtools clobber
#===== End of Makefile =====
