#============================================================================
#
#		マップ差し替えデータコンバート
#
#	2009.12.07	tamada GAMEFREAK inc.
#
#============================================================================
$KCODE = "SJIS"

#C形式ヘッダ読み込みのため
require "#{ENV["PROJECT_ROOT"]}tools/headerdata.rb"

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
COL_MATRIX_ID = 0
COL_LAYER = 1
COL_TYPE  = 2
COL_BASE_KEY  = 3
COL_VAR_A = 4
COL_VAR_B = 5
COL_VAR_C = 6
COL_VAR_D = 7
COL_COMMENT = 8

LAYER_MAPBLOCK = 0
LAYER_MATRIX = 1

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
class FileReadError < Exception; end

#------------------------------------------------------------------------------
#
#   必要なパラメータの変換用クラス
#
#------------------------------------------------------------------------------
class Dict
  @@matrix_syms = HeaderData.new("../map_matrix/map_matrix.naix")
  @@landdata_syms = HeaderData.new("../land_data/map_land_data.naix")
  @@prog_syms = HeaderData.new("../../../prog/src/field/map_replace_def.h")

  @@types = {
    "季節" => "MAPREPLACE_ID_SEASON",
    "バージョン" => "MAPREPLACE_ID_VERSION",
    "バージョン＋季節" => "MAPREPLACE_ID_SEASON_VERSION",
    "イベント１" => "MAPREPLACE_ID_EVENT01",
    "イベント２" => "MAPREPLACE_ID_EVENT02",
    "イベント３" => "MAPREPLACE_ID_EVENT03",
    "イベント４" => "MAPREPLACE_ID_EVENT04",
    "イベント５" => "MAPREPLACE_ID_EVENT05",
    "イベント６" => "MAPREPLACE_ID_EVENT06",
    "イベント７" => "MAPREPLACE_ID_EVENT07",
    "イベント８" => "MAPREPLACE_ID_EVENT08",
    "イベント９" => "MAPREPLACE_ID_EVENT09",
  }

  #マップマトリックスID検索
  def Dict.searchMatrix( str )
    name = "NARC_map_matrix_" + str.downcase + "_mat_bin"
    return @@matrix_syms.search( name )
  end

  #地形ブロックID検索
  def Dict.searchLandData( str )
    name = "NARC_map_land_data_" + str + "_3dppack"
    return @@landdata_syms.search( name )
  end

  #タイプ指定検索
  def Dict.searchTypeStr( str )
    if @@types.has_key?(str) then
      return @@types[str]
    else
      raise FileReadError, "type\"#{str}\"は未定義です"
    end
  end
  def Dict.getTypeValue( str )
    return @@prog_syms.search( str )
  end
  def Dict.searchType( str )
    if @@types.has_key?(str) then
      return @@prog_syms.search( @@types[str] )
    else
      raise FileReadError, "type\"#{str}\"は未定義です"
    end
  end

  #レイヤー指定検索
  def Dict.searchLayer( str )
    return @@prog_syms.search( "MAPREPLACE_LAYER_" + str )
  end

end

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------

def debug_puts( str)
end

def check_line( line )
  debug_puts "check line ..#{line}"
  column = line.split(",")
  debug_puts "matrix_id"
  return false if column[COL_MATRIX_ID] != "matrix_id"
  debug_puts "layer"
  return false if column[COL_LAYER] != "layer"
  debug_puts "type"
  return false if column[COL_TYPE]  != "type"
  debug_puts "基本キーＩＤ"
  return false if column[COL_BASE_KEY] != "基本キーID"
  debug_puts "変化パターンＡ"
  return false if column[COL_VAR_A] != "変化パターンA"
  debug_puts "変化パターンＢ"
  return false if column[COL_VAR_B] != "変化パターンB"
  debug_puts "変化パターンＣ"
  return false if column[COL_VAR_C] != "変化パターンC"
  debug_puts "変化パターンＤ"
  return false if column[COL_VAR_D] != "変化パターンD"
  debug_puts "コメント"
  return false if column[COL_COMMENT] != "コメント"

  debug_puts "check line ..OK"
  return true
end

#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
class MapReplaceArray

  class ReplaceData
    attr_reader :matrix_id, :layer, :type, :base_key, :vars, :comment

    def set_var( count, column )
      in_vars = column[COL_VAR_A, count]
      column[COL_VAR_A + count, 4 - count].each{|item|
        if item != "" then
          raise FileReadError
        end
      }
=begin
      [COL_VAR_D, COL_VAR_C, COL_VAR_B, COL_VAR_A].each{|item|
        if item != "" then
          raise FileReadError
        end
      }
=end
      return in_vars
    end

    def initialize( column )
      @column = column

      @matrix_id = Dict.searchMatrix(column[COL_MATRIX_ID])
      @layer = Dict.searchLayer(column[COL_LAYER])
      @type = Dict.searchTypeStr(column[COL_TYPE])

      in_var = column[COL_VAR_A .. COL_VAR_D]
      case @type
      when "MAPREPLACE_ID_SEASON"
        in_vars = set_var( 3, column )
      when "MAPREPLACE_ID_VERSION"
        in_vars = set_var( 1, column )
      when "MAPREPLACE_ID_SEASON_VERSION"
        in_vars = set_var( 4, column )
      when /^MAPREPLACE_ID_EVENT/
        in_vars = set_var( 1, column )
      else
        raise FileReadError, "コンバータが未対応のタイプ指定(#{@type})です"
      end

      case @layer
      when LAYER_MAPBLOCK
        @base_key = Dict.searchLandData( column[COL_BASE_KEY] )
        @vars = in_vars.map{|item| Dict.searchLandData( item ) }
      when LAYER_MATRIX
        @base_key = Dict.searchMatrix( column[COL_BASE_KEY] )
        @vars = in_vars.map{|item| Dict.searchMatrix( item ) }
      else
        raise FileReadError, "コンバータが未対応のレイヤー指定です"
      end
      1.upto(4 - @vars.length){|i| @vars << 0 }
    end

    def dumpDebug()
      p self
    end

    def dumpBinary()
      output = ""
      output += [@matrix_id, @layer, Dict.getTypeValue(@type), @base_key].pack("SCCS")
      output += @vars.pack("SSSS")
      output += [0xffff].pack("S")
      return output
    end
  end


  def initialize( file )
    @items = Array.new
    file.each{|line|
      column = line.chomp.split(",")
      if column.length < 2 then break end
      @items << ReplaceData.new( column )
    }
  end

  def dumpDebug()
    @items.each{|item| item.dumpDebug() }
  end

  def dumpBinary()
    output = ""
    @items.each{|item| output += item.dumpBinary() }
    return output
  end

  def dumpHeader()
    output = <<OUTPUT_HEADER
//======================================================================
/*
 * 自動生成ファイル：地形書き換えデータ
 */
//======================================================================
#pragma once

enum {
  MAPREPLACE_OFS_MATRIX_ID  = 0,
  MAPREPLACE_OFS_LAYER = 2,
  MAPREPLACE_OFS_TYPE = 3,
  MAPREPLACE_OFS_BASE_KEY = 4,
  MAPREPLACE_OFS_VAR = 6,
  MAPREPLACE_OFS_DUMMY = 14,

  MAPREPLACE_DATASIZE = 16,
};

enum {
  MAPREPLACE_VAR_COUNT = 4,
OUTPUT_HEADER
    output += sprintf("  MAP_REPLACE_DATA_COUNT = (%d),\n", @items.length )
    output += sprintf("};\n")

    output += sprintf("//ここまで\n")
    return output
  end
end


#------------------------------------------------------------------------------
#
#     メイン
#
#------------------------------------------------------------------------------
begin
  puts "start"

  file = File.open(ARGV[0])
  in_data = false
  lines = Array.new

  while line = file.gets
    column = line.split(",")
    if check_line(line) == true then
      in_data = true
      next
    end
    next if in_data == false 
    if column.length < 2 then
      break
    end
    lines << line
  end

  items = MapReplaceArray.new( lines )
  #items.dumpDebug
  output = items.dumpBinary
  File.open(ARGV[1],"wb"){|file|file.write output }
  File.open(ARGV[2],"w"){|file| file.write items.dumpHeader() }

end




