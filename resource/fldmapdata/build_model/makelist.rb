#------------------------------------------------------------------------------
#
#
# 配置モデルアーカイブ生成
#		
#
# 2009.12 内部構造変更（動作は変わっていない）
#
#------------------------------------------------------------------------------
KCODE = "SJIS"

require "fileutils"

#C形式ヘッダ読み込みのため
require "#{ENV["PROJECT_ROOT"]}tools/headerdata.rb"


class TooMuchSymbolError < Exception; end
class SymbolRedefineError < Exception; end
class AnimeIndexError < Exception; end


BUILDMODEL_HEADER = "../../../prog/src/field/field_buildmodel.h"
ANIME_INDEX_FILE  ="wkdir/buildmodel_anime.h"

COL_FILENAME = 1
COL_ANIME_IDX = 8
COL_PROG_IDX = 9
COL_SUBMODEL_IDX = 10
COL_SUBMODEL_X = 11
COL_SUBMODEL_Y = 12
COL_SUBMODEL_Z = 13

#------------------------------------------------------------------------------
# 配置モデルデータを保持するクラス
#------------------------------------------------------------------------------
class BMData
  attr_reader :sym, :anime_id, :prog, :submodel_id, :submodel_pos

  #------------------------------------------------------------------------------
  # アニメIDの辞書生成
  #------------------------------------------------------------------------------
  def BMData.makeAnimeDictionary(anime_index_file)
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
  def BMData.readBuildModelHeader( filename )
    bmdic = HeaderDataArray.new
    bmdic.load( filename )
    return bmdic
  end

  @@syms = Array.new
  @@anm_dic = self.makeAnimeDictionary(ANIME_INDEX_FILE)
  @@prog_dic = self.readBuildModelHeader( BUILDMODEL_HEADER )

  #------------------------------------------------------------------------------
  #   初期化処理
  #------------------------------------------------------------------------------
  def initialize( sym, anime_id, prog, submodel, x, y, z )
    if @@syms.index(sym) == nil then
      @@syms << sym
    else
      raise SymbolRedefineError, "ファイル#{sym}が多重定義されています"
    end
    if @@syms.length > 300 then
      raise TooMuchSymbolError, "配置モデルの種類が制限値（300)を超えています！！"
    end
    @sym = sym
    @anime_id = getAnimeID( anime_id )
    @prog = getProgID( prog )
    @submodel = submodel
    @submodel_id = nil
    @smx = Integer(x)
    @smy = Integer(y)
    @smz = Integer(z)
  end

  def dump()
    #p self
    output = ""
    output += [ @anime_id, @prog, @submodel_id ].pack("SSS")
    output += [ @smx, @smy, @smz ].pack("sss")
  end
  
  #------------------------------------------------------------------------------
  # 各配置モデルごとのアニメ指定IDのテーブルを生成する
  # IN: anm_dic   アニメデータの索引テーブル
  #     arr         配置モデルごとのアニメ指定ID（配置モデルと同じ並び順であることを想定）
  #------------------------------------------------------------------------------
  def getAnimeID(anime_name)
    index = @@anm_dic.index(anime_name.upcase)
    if anime_name == "×" then
      index = 0xffff
    elsif index == nil then
      raise AnimeIndexError, "アニメ指定 #{anime_name} が見つかりませんでした"
    end
    return index
  end

  #------------------------------------------------------------------------------
  # 従属モデル名→IDに変換
  #------------------------------------------------------------------------------
  def createSubModelID()
    idx = @@syms.index(@submodel)
    if @submodel == "×" then
      @submodel_id = 0xffff 
    elsif idx == nil then
      p @@syms
      puts "従属モデル名#{@submodel}が見つかりません！！"
      @submodel_id = 0xffff
    else
      @submodel_id = idx
    end
  end

  #------------------------------------------------------------------------------
  # プログラム指定IDの取得
  #------------------------------------------------------------------------------
  def getProgID(prog_name)
    if prog_name == "×" then
      return 0
    elsif prog_name =~/\d+/ then
     return Integer(prog_name)
    else
      return Integer( @@prog_dic.search( "BM_PROG_ID_" + prog_name, @sym ) )
    end
  end

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



bmdata = Array.new

#入力ファイル読み込み、BMDataの配列を生成する
File.open(INPUT_FILE, "r"){|input_file|
  input_file.gets

  input_file.each{|line|
    column = line.split
    if column.length < 2 then break end
    bmdata << BMData.new( column[COL_FILENAME],
                     column[COL_ANIME_IDX], column[COL_PROG_IDX], column[COL_SUBMODEL_IDX],
                     column[COL_SUBMODEL_X], column[COL_SUBMODEL_Y], column[COL_SUBMODEL_Z] ) 

  }
}

#バイナリファイル生成
output = ""
bmdata.each{|bm|
  bm.createSubModelID()
  output += bm.dump
}
bin_file = File.open(BIN_FILE, "wb")
bin_file.write output
bin_file.close



#アーカイブ対象ファイルを記述するファイルの名前
File.open(ARC_FILE, "w"){|arc_file|
  bmdata.each{|bm|
    nsbmd_name = "#{bm.sym}.nsbmd"
    arc_file.puts "\"nsbmdfiles/#{nsbmd_name}\""
  }
  arc_file.close
}

#依存ファイル定義を記述するファイルの名前
File.open(DEP_FILE, "w"){|dep_file|
  dep_file.puts "#{DEP_SYMBOL}	=	\\\n"
  bmdata.each{|bm|
    nsbmd_name = "#{bm.sym}.nsbmd"
    dep_file.puts "\t$(RESDIR)/#{nsbmd_name} \\\n"
  }
  dep_file.printf("\n")
}

#imdが存在しない場合、ダミーを生成する
bmdata.each{|bm|
	imd_name = "imdfiles/#{bm.sym}.imd"
  unless FileTest.exists?( imd_name ) then
    puts "\t@echo #{imd_name}が存在しないため、ダミーファイルをコピーします"
    FileUtils.cp "imdfiles/dummy_buildmodel.imd",imd_name
  end
}

