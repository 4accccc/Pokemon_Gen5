#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#※コンバートを行う人のユーザーIDをここに書く
#------------------------------------------------------------------------------
USERS	= tanaka_ryota tamada

#------------------------------------------------------------------------------
#※ここに作成するnarc名を書く
#------------------------------------------------------------------------------
TARGETNAME = wbt_dldata_conv

SUBTARGETNAME = my_crc_ccitt

#------------------------------------------------------------------------------
#※コピー先へのパスを書く（通常はPROJECT_ARCDIRでよい）
#------------------------------------------------------------------------------
#TARGETDIR = out
TARGETDIR = ../wbt_dldata

#
HASH_DIR   = $(PROJECT_ROOT)/tools/hash
#
MONSNO_RB  = monsno_hash.rb
ITEM_RB    = item_hash.rb
WAZA_RB    = wazano_hash.rb
SEIKAKU_RB = seikaku_hash.rb
TOKUSEI_RB = tokusei_hash.rb
#TR_RB      = trtype_hash.rb
TR_RB      = 
#
HASH_RB  = $(MONSNO_RB) $(ITEM_RB) $(WAZA_RB) $(SEIKAKU_RB) $(TOKUSEI_RB) $(TR_RB)
#
#
#WBT_DIR  = ../wbt_dldata
WBT_DIR  = .
#
# うまい方法があるはずだ..
WBT_RB0  = version.rb
WBT_RB1  = wbt_dldata_character.rb
WBT_RB2  = wbt_dldata_config.rb
WBT_RB3  = wbt_dldata_conv.rb
WBT_RB4  = wbt_dldata_data.rb
WBT_RB5  = wbt_dldata_match.rb
WBT_RB6  = wbt_dldata_pokemon.rb
WBT_RB7  = wbt_dldata_rule.rb
WBT_RB8  = wbt_dldata_util.rb
WBT_RB9  = wbt_tokusei_data.rb
WBT_RB10 = my_tab_out_direct_sheet.rb
WBT_RB11 = wbt_dldata_error.rb
#
WBT_TOKUSEI = wbt_tokusei_conv.rb
#
WBT_PERSONAL = $(PROJECT_RSCDIR)\personal\personal_sw.csv
#
WBT_RB  = $(WBT_RB0) $(WBT_RB1) $(WBT_RB2) $(WBT_RB3) $(WBT_RB4) \
	  $(WBT_RB5) $(WBT_RB6) $(WBT_RB7) $(WBT_RB8) $(WBT_RB9) $(WBT_RB10) $(WBT_RB11) wbt_dldata_dictionary.rb

WBT_RB20 = $(SUBTARGETNAME).rb

#------------------------------------------------------------------------------
#※サブディレクトリでもMakeしたい場合、ここにディレクトリ名を書く
#------------------------------------------------------------------------------
SUBDIRS	=

#共通ルールファイルのインクルード
include	$(PROJECT_RSCDIR)\macro_define
#
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
#ifeq	($(CONVERTUSER),false)	#コンバート対象者のみ、コンバートのルールを有効にする

# サフィックスルールを定義することで 依存関係を自動で判断してくれる
.SUFFIXES: .rb .exe
#
#$(TARGETNAME).exe: $(HASH_RB) $(WBT_RB)
#	ruby rubyscript2exe.rb $(TARGETNAME).rb
#
#.xls.bin:
#	ruby wbt_dldata_conv.rb $(input_file)
#	ruby my_crc_ccitt.rb $(output_file) $(output_file_crc)
#	ruby my_crc_ccitt.rb $(output_file) $(output_file_tmp)
#	ruby my_crc_ccitt.rb $(output_file_tmp) $(output_file_crc) -t
#	rm $(output_file_tmp)
#
endif

.PHONY:	do-build clean subdir

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
#do-build:	subdir $(TARGETDIR)/$(TARGETNAME).exe $(TARGETDIR)/$(SUBTARGETNAME).exe
do-build:	subdir $(TARGETNAME).exe $(SUBTARGETNAME).exe

subdir:
	@$(MAKE_SUBDIR)

# コンバート対象者外はダミー(何もしない)
ifeq	($(CONVERTUSER),false)
#ifeq	($(CONVERTUSER),true)
$(TARGETNAME).exe: version.rb
$(SUBTARGETNAME).exe: version.rb
endif

# コンバート対象者はコンバート
ifeq	($(CONVERTUSER),true)
#ifeq	($(CONVERTUSER),false)

# コピーだけ実行する
#$(TARGETDIR)/$(TARGETNAME).exe: $(TARGETNAME).exe
#	$(COPY) $(TARGETNAME).exe $(TARGETDIR)/$(TARGETNAME).exe
#
#$(TARGETDIR)/$(SUBTARGETNAME).exe: $(SUBTARGETNAME).exe
#	$(COPY) $(SUBTARGETNAME).exe $(TARGETDIR)/$(SUBTARGETNAME).exe

$(TARGETNAME).exe: $(HASH_RB) $(WBT_RB)
	ruby rubyscript2exe.rb $(TARGETNAME).rb

$(SUBTARGETNAME).exe: $(WBT_RB20)
	ruby rubyscript2exe.rb $(SUBTARGETNAME).rb
#
#
$(MONSNO_RB) : $(HASH_DIR)/$(MONSNO_RB)
	$(COPY) $(HASH_DIR)/$(MONSNO_RB) .
#
$(ITEM_RB) : $(HASH_DIR)/$(ITEM_RB)
	$(COPY) $(HASH_DIR)/$(ITEM_RB) .
#
$(WAZA_RB) : $(HASH_DIR)/$(WAZA_RB)
	$(COPY) $(HASH_DIR)/$(WAZA_RB) .
#
$(SEIKAKU_RB) : $(HASH_DIR)/$(SEIKAKU_RB)
	$(COPY) $(HASH_DIR)/$(SEIKAKU_RB) .
#
$(TOKUSEI_RB) : $(HASH_DIR)/$(TOKUSEI_RB)
	$(COPY) $(HASH_DIR)/$(TOKUSEI_RB) .
#
#$(TR_RB) : $(HASH_DIR)/$(TR_RB)
#	$(COPY) $(HASH_DIR)/$(TR_RB) .
#
#
#$(WBT_RB0) : $(WBT_DIR)/$(WBT_RB0)
#	$(COPY) $(WBT_DIR)/$(WBT_RB0) .
#
#$(WBT_RB1) : $(WBT_DIR)/$(WBT_RB1)
#	$(COPY) $(WBT_DIR)/$(WBT_RB1) .
#
#$(WBT_RB2) : $(WBT_DIR)/$(WBT_RB2)
#	$(COPY) $(WBT_DIR)/$(WBT_RB2) .
#
#$(WBT_RB3) : $(WBT_DIR)/$(WBT_RB3)
#	$(COPY) $(WBT_DIR)/$(WBT_RB3) .
#
#$(WBT_RB4) : $(WBT_DIR)/$(WBT_RB4)
#	$(COPY) $(WBT_DIR)/$(WBT_RB4) .
#
#$(WBT_RB5) : $(WBT_DIR)/$(WBT_RB5)
#	$(COPY) $(WBT_DIR)/$(WBT_RB5) .
#
#$(WBT_RB6) : $(WBT_DIR)/$(WBT_RB6)
#	$(COPY) $(WBT_DIR)/$(WBT_RB6) .
#
#$(WBT_RB7) : $(WBT_DIR)/$(WBT_RB7)
#	$(COPY) $(WBT_DIR)/$(WBT_RB7) .
#
#$(WBT_RB8) : $(WBT_DIR)/$(WBT_RB8)
#	$(COPY) $(WBT_DIR)/$(WBT_RB8) .
#
#$(WBT_RB9) : $(WBT_DIR)/$(WBT_RB9)
#	$(COPY) $(WBT_DIR)/$(WBT_RB9) .
#
#$(WBT_RB10) : $(WBT_DIR)/$(WBT_RB10)
#	$(COPY) $(WBT_DIR)/$(WBT_RB10) .
#
#$(WBT_RB20) : $(WBT_DIR)/$(WBT_RB20)
#	$(COPY) $(WBT_DIR)/$(WBT_RB20) .
#
$(WBT_RB9) : $(WBT_TOKUSEI) $(WBT_PERSONAL)
	ruby $(WBT_TOKUSEI)
#
endif

#------------------------------------------------------------------------------
#	make clean ルール
#※生成ファイルを削除するように記述を変更してください
#------------------------------------------------------------------------------
clean:
	@$(MAKE_SUBDIR)
ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、中間ファイル削除
#ifeq	($(CONVERTUSER),false)	#コンバート対象者のみ、中間ファイル削除
	rm -f $(TARGETNAME).exe
	rm -f $(SUBTARGETNAME).exe
	rm -f $(HASH_RB)
	rm -f $(WBT_RB9)
endif
#	rm -f $(TARGETDIR)/$(TARGETNAME).exe
#	rm -f $(TARGETDIR)/$(SUBTARGETNAME).exe

