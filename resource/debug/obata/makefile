#------------------------------------------------------------------------------
#  narcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= obata_toshihiro

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = debug_obata

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

ICACONV = ..\..\..\tools\ica\ica_converter.rb


#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define


CONVERTDATA = ica_test_data2.bin \
							houou_test.nsbmd houou_test.nsbca houou_test.nsbta \
							trailer.nsbmd t1_house_01.nsbmd h01_ship.nsbmd \
							dice.nsbmd \
							dice_anime_srt.bin \
							dice_anime_sr.bin \
							dice_anime_st.bin \
							dice_anime_rt.bin \
							dice_anime_s.bin \
							dice_anime_r.bin \
							dice_anime_t.bin \
							elboard_test.nsbmd \
							elboard_test.nsbtx \
							elboard_test.nsbta \
							elboard_test.nsbca \
							gelboard01.nsbmd \
							gelboard01.nsbta

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: 
endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:	subdir $(TARGETDIR)/$(targetname).narc 

subdir:
	@$(MAKE_SUBDIR)

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# コンバート
	ruby $(ICACONV) -rt camera_null2.ica ica_test_data2.bin
	ruby $(ICACONV) -srt dice.ica dice_anime_srt.bin
	ruby $(ICACONV) -sr dice.ica dice_anime_sr.bin
	ruby $(ICACONV) -st dice.ica dice_anime_st.bin
	ruby $(ICACONV) -rt dice.ica dice_anime_rt.bin
	ruby $(ICACONV) -s dice.ica dice_anime_s.bin
	ruby $(ICACONV) -r dice.ica dice_anime_r.bin
	ruby $(ICACONV) -t dice.ica dice_anime_t.bin
	g3dcvtr houou_test.imd
	g3dcvtr houou_test.ica
	g3dcvtr houou_test.ita
	g3dcvtr trailer.imd
	g3dcvtr t1_house_01.imd
	g3dcvtr h01_ship.imd
	g3dcvtr dice.imd
	g3dcvtr elboard_test.imd -emdl
	g3dcvtr elboard_test.imd -etex
	g3dcvtr elboard_test.ica
	g3dcvtr elboard_test.ita
	g3dcvtr gelboard01.imd
	g3dcvtr gelboard01.ita
endif


# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、元ファイルからコンバートするルールを有効にする
# コピー先のnarcが古い場合実行する
$(targetname).narc: $(CONVERTDATA)
	nnsarc -r -l -n -i $(targetname) $^
	$(NAIXCUT) $(targetname).naix 
endif



#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f $(CONVERTDATA)
	rm -f $(targetname).narc
	rm -f $(targetname).naix
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix


