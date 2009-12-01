#------------------------------------------------------------------------------
#
# 配置モデルアーカイブ生成
#		
#
#------------------------------------------------------------------------------
KCODE = "SJIS"

require "fileutils"

class SymbolRedefineError < Exception; end
class AnimeIndexError < Exception; end

ANIME_INDEX_FILE  ="wkdir/buildmodel_anime.h"

COL_FILENAME = 1
COL_ANIME_IDX = 8
COL_PROG_IDX = 9
COL_SUBMODEL_IDX = 10
COL_SUBMODEL_X = 11
COL_SUBMODEL_Y = 12
COL_SUBMODEL_Z = 13

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
class SubModelPos
  attr_reader :x, :y, :z
  def initialize x, y, z
    @x = Integer(x)
    @y = Integer(y)
    @z = Integer(z)
  end
end

#------------------------------------------------------------------------------
# 各配置モデルごとのアニメ指定IDのテーブルを生成する
# IN: anm_dic   アニメデータの索引テーブル
#     arr         配置モデルごとのアニメ指定ID（配置モデルと同じ並び順であることを想定）
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
def makeAnimeDictionary(anime_index_file)
  anm_dic = Array.new
  File.open(anime_index_file, "r"){|file|
    file.each{|line|
      if line =~/^#/ then
        next
      end
      column = line.split
      anm_dic << column[0].upcase
    }
  }
  return anm_dic
end

def getAnimeID(anm_dic, anime_name)
  index = anm_dic.index(anime_name.upcase)
  if anime_name == "×" then
    index = 0xffff
  elsif index == nil then
    raise AnimeIndexError, "アニメ指定 #{anime_name} が見つかりませんでした"
  end
  return index
end

def getProgID(prog_name)
  if prog_name == "×" then
    return 0xffff
  elsif prog_name =~/\d+/ then
    return Integer(prog_name)
  end
end

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
def makeIndexes(anm_arr, prog_arr, submodel_arr, pos_arr)

  output = ""
  anm_arr.each_index{|idx|
    output += [anm_arr[idx], prog_arr[idx], submodel_arr[idx]].pack("SSS")
    output += [pos_arr[idx].x, pos_arr[idx].y, pos_arr[idx].z].pack("sss")
  }
  return output
end

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
def makeSubmodelIDList(symbols, submodel_list)
  submodel_ids = submodel_list.map{|name|
    idx = symbols.index(name)
    if name == "×" then
      0xffff 
    elsif idx == nil then
      puts "従属モデル名#{name}が見つかりません！！"
      0xffff
    else
      idx
    end
  }
  return submodel_ids
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

anm_dic = makeAnimeDictionary(ANIME_INDEX_FILE)

input_file = File.open(INPUT_FILE, "r")
dep_file = File.open(DEP_FILE, "w")
arc_file = File.open(ARC_FILE, "w")

dep_file.puts "#{DEP_SYMBOL}	=	\\\n"

check_list = Array.new

syms = Array.new
anime_ids = Array.new
progs = Array.new
submodels = Array.new
submodel_pos = Array.new

input_file.gets

while line = input_file.gets
	column = line.split
	if column.length < 2 then break end
  sym = column[COL_FILENAME]
  if syms.index(sym) == nil then
    syms << sym
  else
    raise SymbolRedefineError, "ファイル#{sym}が多重定義されています"
  end
	nsbmd_name = "#{sym}.nsbmd"
	dep_file.puts "\t$(RESDIR)/#{nsbmd_name} \\\n"
	arc_file.puts "\"nsbmdfiles/#{nsbmd_name}\""
	imd_name = "imdfiles/#{sym}.imd"
	check_list << imd_name unless FileTest.exists? imd_name
  anime_ids << getAnimeID(anm_dic, column[COL_ANIME_IDX])
  progs << getProgID(column[COL_PROG_IDX])
  submodels << column[COL_SUBMODEL_IDX]
  submodel_pos << SubModelPos.new( column[COL_SUBMODEL_X],
                                  column[COL_SUBMODEL_Y],
                                  column[COL_SUBMODEL_Z] ) 
end

if syms.length > 255 then
  puts "配置モデルの種類が制限値（255)を超えています！！"
  puts "プログラマに連絡してください。"
  exit 1
end

submodel_ids = makeSubmodelIDList(syms, submodels)

syms.each_index{|idx|
#  printf("%-16s %04x, %04x %04x\n", syms[idx], anime_ids[idx], progs[idx], submodel_ids[idx])
}
bin_file = File.open(BIN_FILE, "wb")
bin_file.write makeIndexes(anime_ids, progs, submodel_ids, submodel_pos)
bin_file.close

dep_file.printf("\n")
check_list.each{|item|
	puts "\t@echo #{item}が存在しないため、ダミーファイルをコピーします"
	FileUtils.cp "imdfiles/dummy_buildmodel.imd",item
}

input_file.close
dep_file.close
arc_file.close

