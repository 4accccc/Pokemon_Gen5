#------------------------------------------------------------------------------
#  コピーしてtargetnameを書き換えれば汎用的に使えます
#  2008.08.19
#------------------------------------------------------------------------------

# メモ
#
# WBT演出でのビジョンでトレーナーの顔グラフィックを表示する。
# その顔グラフィックのテクスチャデータのmakefile
# resource/trtypeをmakeすると、trgra_cut_wb.scrがコピーされてくる
# このファイルは,[ファイル名，トレーナータイプ]の一覧
#
# このmakefileが大本。
#	makefile_writeとmakefile_texの2つのmakeファイルを呼び出している
#
#	makefile_write･･･wbt_cut_gra.rbのrubyを実行しているだけ。
#										このrubyではreplace_dependのmakeファイルを作成している。
#										replace_dependはncgをコンバートするmakeファイル。
#
#	makefile_tex･･･↑で作成された、replace_dependのmakeファイルを実際にmakeする。
#									出来上がったバイナリ集をアーカイブにまとめる。
#


# 共通定義を利用
include mkdefine

# sub makefile
MKFILE_WRITE		= makefile_write		# ncgファイル名一覧を参照し、makefile生成
MKFILE_TEX			= makefile_tex			# narcにまとめる makefile

#共通ルールファイルのインクルード
include $(PROJECT_RSCDIR)\macro_define

.PHONY:	do-build clean

ifeq	($(CONVERTUSER),true)	#コンバート対象者のみ、コンバートのルールを有効にする
.SUFFIXES:	
endif

#------------------------------------------------------------------------------
#	make do-build ルール
#------------------------------------------------------------------------------
do-build:
	$(MAKE) -f $(MKFILE_WRITE)
	$(MAKE) -f $(MKFILE_TEX)

#------------------------------------------------------------------------------
#	make clean ルール
#------------------------------------------------------------------------------
clean:
	$(MAKE) -f $(MKFILE_TEX) clean
	$(MAKE) -f $(MKFILE_WRITE) clean

