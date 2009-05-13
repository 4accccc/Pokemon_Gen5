#------------------------------------------------------------------------------
#
# 配置モデルアーカイブ生成
#		
#
#------------------------------------------------------------------------------
KCODE = "SJIS"

require "fileutils"

class AnimeIndexError < Exception; end

ANIME_INDEX_FILE  ="wkdir/buildmodel_anime.h"
#------------------------------------------------------------------------------
# 各配置モデルごとのアニメ指定IDのテーブルを生成する
# IN: outputfile  出力先オブジェクト
#     arr         配置モデルごとのアニメ指定ID（配置モデルと同じ並び順であることを想定）
#------------------------------------------------------------------------------
def makeAnimeIndex(outputfile, arr)
  anm_dic = Array.new
  File.open(ANIME_INDEX_FILE, "r"){|file|
    file.each{|line|
      if line =~/^#/ then
        next
      end
      column = line.split
      anm_dic << column[0].upcase
    }
  }
  #p anm_dic
  arr.each{|id|
    index = anm_dic.index(id.upcase)
    if id == "×" then
      index = 0xffff
    elsif index == nil then
      raise AnimeIndexError, "アニメ指定 #{id} が見つかりませんでした"
    end
    outputfile.write [index].pack("S")
  }
end

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
#入力ファイル（Excelファイルをタブ区切りテキストに変換したもの）
INPUT_FILE	=	ARGV[0]
#依存ファイルを定義するMakefile変数のシンボル名
DEP_SYMBOL	= ARGV[1]
#依存ファイル定義を記述するファイルの名前
DEP_FILE	=	ARGV[2]
#アーカイブ対象ファイルを記述するファイルの名前
ARC_FILE	=	ARGV[3]

BIN_FILE  = ARGV[4]

input_file = File.open(INPUT_FILE, "r")
dep_file = File.open(DEP_FILE, "w")
arc_file = File.open(ARC_FILE, "w")
bin_file = File.open(BIN_FILE, "wb")

dep_file.puts "#{DEP_SYMBOL}	=	\\\n"

check_list = Array.new
animes = Array.new

input_file.gets

while line = input_file.gets
	column = line.split
	if column.length < 2 then break end
	nsbmd_name = "#{column[1]}.nsbmd"
	dep_file.puts "\t$(RESDIR)/#{nsbmd_name} \\\n"
	arc_file.puts "\"nsbmdfiles/#{nsbmd_name}\""
	imd_name = "imdfiles/#{column[1]}.imd"
	check_list << imd_name unless FileTest.exists? imd_name
  animes << column[8]
end

makeAnimeIndex(bin_file, animes)

dep_file.printf("\n")
check_list.each{|item|
	puts "\t@echo #{item}が存在しないため、ダミーファイルをコピーします"
	FileUtils.cp "imdfiles/dummy_buildmodel.imd",item
}
input_file.close
dep_file.close
arc_file.close
bin_file.close

