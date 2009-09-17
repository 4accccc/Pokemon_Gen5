#------------------------------------------------------------------------------
#
#	ポケモンパーソナル用Makefile
#
#	2008.08.19	tamada@gamefreak
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------

USERS	=	sogabe nishino

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------

HEADER = monsno_def.h
HASHFILE = monsno_hash.rb
GMMFILE = monsname.gmm
SCRFILE = pokegra_wb.scr
OTHERFILE = otherpoke_wb.scr

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= ../../prog/include/poke_tool/
GMMDIR	= ../../resource/message/src/
HASHDIR	= ../../tools/hash/

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

include	$(NITROSDK_ROOT)/build/buildtools/commondefs.cctype.CW

.PHONY:	do-build clean

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする

#パーソナルデータ生成

do-build: out_end

out_end: personal_wb.csv
	ruby ../../tools/personal_conv/personal_conv.rb $< ../message/template.gmm
endif

do-build: $(TARGETDIR)$(HEADER) $(GMMDIR)$(GMMFILE) $(HASHDIR)$(HASHFILE)

$(TARGETDIR)$(HEADER):	$(HEADER)
	$(COPY)	$(HEADER) $(TARGETDIR)

$(GMMDIR)$(GMMFILE):	$(GMMFILE)
	$(COPY)	$(GMMFILE) $(GMMDIR)

$(HASHDIR)$(HASHFILE):	$(HASHFILE)
	$(COPY)	$(HASHFILE) $(HASHDIR)

#------------------------------------------------------------------------------
#	make cleanルール
#------------------------------------------------------------------------------
clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	-rm -f *.s
	-rm -f $(HEADER)
	-rm -f $(GMMFILE)
	-rm -f $(HASHFILE)
	-rm -f *.scr
	-rm -f out_end
endif
	-rm -f $(TARGETDIR)$(HEADER)
	-rm -f $(GMMDIR)$(GMMFILE)
	-rm -f $(HASHDIR)$(HASHFILE)

