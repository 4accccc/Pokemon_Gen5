#------------------------------------------------------------------------------
#  通信アイコンnarcを作るmakefile  
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= abc

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
targetname = mission

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
TARGETDIR = $(PROJECT_ARCDIR)

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	= 


#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define








ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .NCLR .ncl .NCGR .ncg .NSCR .nsc .imd

endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
#do-build:	$(ZONETABLE_DIR)$(ZONEID_H) $(MESSAGE_INC_DIR)$(MSG_MISSION_H) $(MESSAGE_INC_DIR)$(MSG_MISSION_MONO_H) $(FLDMMDL_DIR)$(FLDMMDL_CODE_H) $(ITEM_DIR)$(ITEM_H) $(FIELDCOMM_DIR)$(MISSION_TYPES_H) subdir $(TARGETDIR)/$(targetname).narc
#do-build:	subdir $(TARGETDIR)/$(targetname).narc $(ZONEID_H) $(MSG_MISSION_H) $(MSG_MISSION_MONO_H) $(FLDMMDL_CODE_H) $(ITEM_H) $(MISSION_TYPES_H)
do-build:	subdir $(TARGETDIR)/$(targetname).narc

subdir:
	@$(MAKE_SUBDIR)

# narc のコピーだけ実行する
$(TARGETDIR)/$(targetname).narc: $(targetname).narc
	$(COPY) $(targetname).narc $(TARGETDIR)
	$(COPY) $(targetname).naix $(TARGETDIR) 



#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
	rm -f $(targetname).narc
	rm -f $(targetname).naix
	rm -f $(ZONEID_H)
	rm -f $(MSG_MISSION_H)
	rm -f $(MSG_MISSION_MONO_H)
	rm -f $(FLDMMDL_CODE_H)
	rm -f $(ITEM_H)
	rm -f $(MISSION_TYPES_H)
	rm -f $(MISSION_TYPES_H)
	rm -f *.csv
	rm -f mission_data.elf
	rm -f mission_data.bin
endif
	rm -f $(TARGETDIR)/$(targetname).narc
	rm -f $(TARGETDIR)/$(targetname).naix

