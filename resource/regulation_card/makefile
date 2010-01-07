#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= ohno toru_nagihashi

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define


MAKE_REG = makefile_reg
MAKE_CRC = makefile_crc


do-build:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	$(MAKE) -f $(MAKE_REG)
	$(MAKE) -j 3 -f $(MAKE_CRC)
endif

clean:
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
	$(MAKE) -f $(MAKE_REG) clean
	$(MAKE) -f $(MAKE_CRC) clean
endif
