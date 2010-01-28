#----------------------------------------------------------------------------
#
#   隠しアイテムデータコンバーター
#
#   2009.12.18  tamada GAMEFREAK inc.
#
#----------------------------------------------------------------------------
COL_MAPNAME = 0
COL_MAPID = 1
COL_POSX  = 2
COL_POSZ  = 3
COL_ITEM  = 4
COL_NUM   = 5
COL_WORLDMAP = 6
COL_REVIVAL = 7
COL_ID = 8
COL_THROUGH_NUMBER = 9

COL_MAX = 10

#C形式ヘッダ読み込みのため
require "#{ENV["PROJECT_ROOT"]}tools/headerdata.rb"
require "#{ENV["PROJECT_ROOT"]}tools/hash/item_hash.rb"

class ItemSymbolError < Exception; end

def debug_puts( str )
  STDERR.puts str
end

#------------------------------------------------------------
#------------------------------------------------------------
class Dict
  def Dict.getItemSymbol( namae )
    if $item_hash.has_key?( namae ) then
      return $item_hash[ namae ]
    end
    raise ItemSymbolError, "#アイテム\"{namae}\" は存在しません！！"
  end

  def Dict.readZoneID( filename )
    zone_dict = HeaderDataArray.new
    zone_dict.load( filename )
  end

  def Dict.checkZoneID( zone_symbol )
    return @@zone_dict.search( zone_symbol )
  end

  @@zone_dict = Dict.readZoneID( "../../zonetable/zone_id.h" )
end

#------------------------------------------------------------
#------------------------------------------------------------
class HideItem
  attr_reader  :mapname, :mapid, :posx, :posz, :itemno, :num, :world_flag, :revival_flag, :id

  attr_reader :itemname, :mapid_name

  def initialize(mapname, mapid, x, z, itemname, num, world_flag, revival_flag, id)
    @mapname = mapname
    @mapid = Dict.checkZoneID( "ZONE_ID_" + mapid.upcase )
    @posx = x
    @posz = z
    @itemno = Dict.getItemSymbol( itemname )
    @num = num
    @world_flag = world_flag
    @revival_flag = revival_flag
    @id = id

    @mapid_name = mapid
    @itemname = itemname
  end
end

def readItemData( filename )
  debug_puts filename
  items = Array.new

  lines = File.open(filename).readlines

  #ヘッダ部分読み飛ばし
  while line = lines.shift
    column = line.split(",")
    if column.length >= COL_MAX then break end
    debug_puts("search title:#{line}")
=begin
    if column[0 .. 6] == [
      "マップ名",	"MAPラベル","X","Y",
      "アイテム名",	"個数",	"反応",	"特殊",	"番号" ]
      break
    end
=end
  end

  #データ部分読み込み
  while line = lines.shift
    column = line.split(",")
    if column.length < COL_MAX then
      break
    end
    items << HideItem.new( column[COL_MAPNAME], column[COL_MAPID],
                          column[COL_POSX], column[COL_POSZ],
                          column[COL_ITEM], column[COL_NUM], column[COL_WORLDMAP],
                          column[COL_REVIVAL], column[COL_ID] )
  end
  return items
end

#------------------------------------------------------------
#
# 隠しアイテム用データファイル作成
#
#------------------------------------------------------------
def getCDAT( items )
  output = ""
  #output += sprintf("static const u16 hide_item_data[%d] = {\n", items.length)
  output += sprintf("static const HIDE_ITEM_DATA hide_item_data[%d] = {\n", items.length)
  output += sprintf("\n")
  items.each{|item|
    output += sprintf("\t{ // %s\n", item.itemname )
    output += sprintf("\t\t%3d, \n", item.id )
    output += sprintf("\t\t%d,%d, // flags\n", item.world_flag, item.revival_flag )
    output += sprintf("\t\t%3d, // %s %s\n", item.mapid, item.mapid_name, item.mapname )
    output += sprintf("\t\t%3d,%3d, // x, y\n", item.posx, item.posz )
    output += sprintf("\t},\n")

=begin
    output += sprintf("\t %3d, // %d %d %3d(%d,%d) %s %s\n",
                      item.id,
                      item.world_flag, item.revival_flag, 
                      item.mapid, item.posx, item.posz, item.mapid_name, item.mapname )
=end
  }
  output += sprintf( "}; // end of hide_item_data\n" )
  return output
end

#------------------------------------------------------------
#
# 隠しアイテム用スクリプトファイル生成
#
#------------------------------------------------------------
def getEV( items )
  output =<<HEADSTRING
//======================================================================
//
//
//    hide_item.ev
//
//    スクリプトファイル：hide_item用
//    このファイルはhide_item.xlsから自動生成されています。
//
//======================================================================

  .text

  .include  "scr_seq_def.h"
 	//.include "msg_item_get_scr.h
  //.include  "hide_item.h"

//--------------------------------------------------------------
//               スクリプトテーブル
//--------------------------------------------------------------

HEADSTRING
  items.each_with_index{|item, index|
    output += sprintf("_EVENT_DATA  ev_hide_item_%03d\n", index )
  }

  output += <<MIDDLESTRING
//--------------------------------------------------------------
//               スクリプト本体 開始
//--------------------------------------------------------------
MIDDLESTRING

  items.each_with_index{|item, index|
    output += sprintf("EVENT_START ev_hide_item_%03d\n", index)
    output += sprintf("//%s ( %d, %d )\n", item.mapname, item.posx, item.posz )
    output += sprintf("\t_ITEM_EVENT_HIDEITEM( %d, %d, FLAG_HIDEITEM_%03d )// %s\n",
                      item.itemno, item.num, item.id, item.itemname )
    output += sprintf("EVENT_END\n\n")
  }

  output += <<TAILSTRING
//--------------------------------------------------------------
//               スクリプト本体 終了
//--------------------------------------------------------------
TAILSTRING
  return output
end

#------------------------------------------------------------
#
#   メイン
#
#------------------------------------------------------------
items = readItemData( ARGV[0] )

File.open(ARGV[1],"w"){|file|
  output = getCDAT( items )
  file.write output
}

File.open(ARGV[2],"w"){|file|
  output = getEV( items )
  file.write output
}




