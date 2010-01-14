#============================================================================
#
#		マップ差し替えデータコンバート
#
#	2009.12.07	tamada GAMEFREAK inc.
#
#============================================================================

#------------------------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#------------------------------------------------------------------------------
USERS	=	tamada nakatsui obata_toshihiro

#----------------------------------------------------------------------------
#※ここに作成するファイル名を書く
#----------------------------------------------------------------------------

TARGETNAME	= map_replace

TARGET_NARC	= $(TARGETNAME).narc
TARGET_NAIX	= $(TARGETNAME).naix

TARGET_HEADER = $(TARGETNAME).h
TARGET_BINARY = $(TARGETNAME).bin

SRC_XLS	= $(TARGETNAME).xls
SRC_TXT	= $(SRC_XLS:.xls=.txt)
#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)fieldmap

DEPENDFILES	= ../land_data/map_land_data.naix ../map_matrix/map_matrix.naix

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)macro_define


#----------------------------------------------------------------------------
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
#	ツールへのパス指定
#----------------------------------------------------------------------------

CSV_RUBY	= ruby $(PROJECT_ROOT)tools/exceltool/xls2xml/tab_out_sheet.rb

#============================================================================
#
#	ルール定義
#
#============================================================================
.PHONY:	do-build clean test

#----------------------------------------------------------------------------
#	生成ルール
#----------------------------------------------------------------------------
do-build: $(TARGETDIR)/$(TARGET_NARC) $(TARGETDIR)/$(TARGET_HEADER)


ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる

$(TARGET_NARC) $(TARGET_HEADER):	$(SRC_XLS) $(DEPENDFILES)
	$(CSV_RUBY) -comma -n 0 $(SRC_XLS) > $(SRC_TXT)
	ruby map_replace.rb $(SRC_TXT) $(TARGET_BINARY) $(TARGET_HEADER)
	nnsarc -i -c -n -l $(TARGET_NARC) *.bin
	@$(NAIXCUT) $(TARGET_NAIX)

endif	#CONVERTUSER

$(TARGETDIR)/$(TARGET_HEADER):	$(TARGET_HEADER)
	$(COPY) $(TARGET_HEADER) $(TARGETDIR)

$(TARGETDIR)/$(TARGET_NARC):	$(TARGET_NARC)
	$(COPY)	$(TARGET_NARC) $(TARGETDIR)
	$(COPY)	$(TARGET_NAIX) $(TARGETDIR)

#----------------------------------------------------------------------------
#	Cleanルール
#----------------------------------------------------------------------------
clean:
	rm -f $(TARGETDIR)/$(TARGET_NARC)
	rm -f $(TARGETDIR)/$(TARGET_NAIX)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ動作させる
	rm -f $(TARGET_NARC) $(TARGET_NAIX)
	rm -f $(SRC_TXT)
endif


#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
test:
	ruby map_replace.rb $(SRC_TXT) $(TARGET_BINARY) $(TARGET_HEADER)

