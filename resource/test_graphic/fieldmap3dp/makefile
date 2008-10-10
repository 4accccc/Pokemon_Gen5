#---------------------------------------------------------------
#	マップコンバート用Makefile
#---------------------------------------------------------------

#---------------------------------------------------------------
#※コンバート作業が必要なユーザーの名前を記述する
#---------------------------------------------------------------
USERS	=	kagaya nakatsui

#リスト読み込み
include datalist.txt

#---------------------------------------------------------------
#	G3Dコンバータへのパス
#---------------------------------------------------------------
G3DCVTR	= $(NITROSYSTEM_ROOT)/tools/win/bin/g3dcvtr.exe

#---------------------------------------------------------------
#※ここに作成するnarc名を書く
#---------------------------------------------------------------
TARGETNAME	= fldmap3dp
NARCNAME	= $(TARGETNAME).narc
NAIXNAME	= $(TARGETNAME).naix

#---------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#---------------------------------------------------------------
TARGETDIR	= $(PROJECT_ARCDIR)/test_graphic

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

MAP_3DMDS		= $(notdir $(wildcard *.3dmd))
MAP_BINS		= $(notdir $(MAP_3DPPACKLIST:.3dppack=.bin))
MAP_IMDS		= $(notdir $(MAP_3DPPACKLIST:.3dppack=.imd))
MAP_NSBMDS		= $(notdir $(MAP_IMDS:.imd=.nsbmd))
MAP_NSBTXS		= $(notdir $(MAP_IMDS:.imd=.nsbtx))

#===============================================================
#コンバート対象者のみ、コンバートのルールを有効にする
ifeq	($(CONVERTUSER),true)
#===============================================================
.SUFFIXES: .3dppack .imd .nsbmd .nsbtx .3dmd .bin

#データからの生成ルール

#nsbmd
%.nsbmd: %.imd
	@echo ModelData Convert: $@
	@$(G3DCVTR) $(@:.nsbmd=.imd) -o $@

#nsbtx
%.nsbtx: %.imd
	@echo TextureDataConvert: $@
	@$(G3DCVTR) $(@:.nsbtx=.imd) -o $@ -etex

#3dppack
.nsbmd.3dppack:
.nsbtx.3dppack:
.nsbtp.3dppack:
.3dmd.3dppack:
.bin.3dppack:
	@echo Create3Dppack: $@
	$(BINLINKER) $*.nsbmd $*.nsbtx $*.nsbtp $*.bin $*.3dmd $*.3dppack 3D

#===============================================================
endif
#===============================================================
.PHONY:	do-build clean

#---------------------------------------------------------------
#	make do-build ルール
#---------------------------------------------------------------
do-build: $(TARGETDIR)/$(NARCNAME)

$(TARGETDIR)/$(NARCNAME): $(NARCNAME)
	$(COPY)	$(NARCNAME) $(TARGETDIR)
	$(COPY) $(NAIXNAME) $(TARGETDIR)

#コンバート対象者の場合は更新対象を変更
ifeq	($(CONVERTUSER),true)
$(NARCNAME): $(MAP_3DMDS) $(MAP_IMDS) $(MAP_NSBMDS) $(MAP_NSBTXS) $(MAP_3DPPACKLIST)
	nnsarc -c -l -n -i $(NARCNAME) $(MAP_3DPPACKLIST)
else
$(NARCNAME): $(MAP_3DPPACKLIST)
	nnsarc -c -l -n -i $(NARCNAME) $(MAP_3DPPACKLIST)
endif

#---------------------------------------------------------------
#	make cleanルール
#---------------------------------------------------------------
clean:
	-rm -f $(TARGETDIR)/$(NARCNAME)
	-rm -f $(TARGETDIR)/$(NAIXNAME)
	-rm -f $(NARCNAME)
	-rm -f $(NAIXNAME)
	
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	-rm -f *.3dppack
	-rm -f *.nsbmd
	-rm -f *.nsbtx
endif
