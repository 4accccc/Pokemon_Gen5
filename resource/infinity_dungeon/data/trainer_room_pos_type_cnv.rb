##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		trainer_room_pos_type_cnv.rb
#*	@brief  無限ダンジョン内トレーナー配置パターン
#*	@author	Masayuki Onoue
#*	@data		2011.04.06
#*
#*/
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"

#define
TYPE_COLUMN_INDEX = 0
MOVE_TYPE_COLUMN_INDEX = 1
POS_X_COLUMN_INDEX = 2
POS_Z_COLUMN_INDEX = 3
ZONE_COLUMN_INDEX = 4
VERSION_COLUMN_INDEX = 5

BW2_MULTI  = "WB"
B2_ONLY = "B"
W2_ONLY = "W"

BW2_MULTI_DEF_WORD  = "INF_MULTI_DUNGEON"
B2_ONLY_DEF_WORD    = "INF_B2_ONLY_DUNGEON"
W2_ONLY_DEF_WORD    = "INF_W2_ONLY_DUNGEON"

class POS_DATA
  attr_accessor :move_type, :pos_x, :pos_z
end

class ROOM_DATA
  attr_accessor :type_name, :member_num, :pos_data, :zone, :version
  def initialize
    @version = nil
    @zone = nil
    @member_num = 0
    @pos_data = Array.new
  end
  def push_data( data )
    @member_num+=1
    @pos_data.push(data)
  end
end

def put_value( value ,file )
  file.print value.to_i
  file.print " ,"
end

def put_str( value ,file )
  file.print value.to_s+" ,"
end

OUT_FILE        = ARGV[0]
OUT_HEADER_FILE = ARGV[1]
CSV_FILE        = ARGV[2]

out_file = open( OUT_FILE ,"w" )
out_header_file = open( OUT_HEADER_FILE ,"w" )

# データの読み込み
data_csv = CSV.open( CSV_FILE ,"r")

# データの読み飛ばし
data_csv.shift

room_data_list = Array.new
rd = nil
end_flg = FALSE
data_csv.each_with_index { |item,index|
  unless item[0] == nil then
    if item[0].include?("#END") then
      room_data_list.push(rd)
      end_flg = TRUE
    end
  end
 
  if end_flg == FALSE then
    if item[0] == nil then
      pos = POS_DATA.new
      pos.move_type = item[MOVE_TYPE_COLUMN_INDEX]
      pos.pos_x = item[POS_X_COLUMN_INDEX]
      pos.pos_z = item[POS_Z_COLUMN_INDEX]
      rd.push_data(pos)
    else
      unless rd == nil then
        room_data_list.push(rd)
      end
      rd = ROOM_DATA.new
      pos = POS_DATA.new
      pos.move_type = item[MOVE_TYPE_COLUMN_INDEX]
      pos.pos_x = item[POS_X_COLUMN_INDEX]
      pos.pos_z = item[POS_Z_COLUMN_INDEX]
      rd.type_name = item[TYPE_COLUMN_INDEX]
      rd.zone = item[ZONE_COLUMN_INDEX]

      # Version判定
      if item[VERSION_COLUMN_INDEX] == BW2_MULTI then
        rd.version = BW2_MULTI_DEF_WORD
      elsif item[VERSION_COLUMN_INDEX] == B2_ONLY then
        rd.version = B2_ONLY_DEF_WORD
      elsif item[VERSION_COLUMN_INDEX] == W2_ONLY then
        rd.version = W2_ONLY_DEF_WORD
      else
        raise "%d行目->バージョン指定文字に誤りがあります"%(index+2)  # インデックスは０スタートでかつ、1行目はタイトル行なので+2
      end
      rd.push_data(pos)
    end
  end
}

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョン内トレーナー配置データ
 *	@author	Masayuki Onoue
 *	@date		2011.02.25
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

static const INF_ROOM_SET_TR_PARAM sc_room_tr_param[] = {
EOS
out_file.puts header % [OUT_FILE]

cmt = <<EOS
  { // INF_ROOM_POS_TYPE_%s
EOS

room_data_list.each { |item|

  out_file.print cmt % item.type_name
  out_file.print "    "
  put_str( item.version  ,out_file)
  put_value( item.member_num ,out_file )
  put_str( "ROOM_TYPE_" +item.zone,out_file )
  out_file.print "\n    { " 
  item.member_num.times{|i|
    out_file.print "\n      { " 
    put_str( "INF_MOVE_CODE_TYPE_"+item.pos_data[i].move_type  ,out_file)
    put_value( item.pos_data[i].pos_x,out_file )
    put_value( item.pos_data[i].pos_z,out_file )
    out_file.puts '}, '
  }
  out_file.print "\n    },\n" 

  out_file.puts '  }, ' 

}
out_file.puts "};"

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョン内トレーナー配置データ定義
 *	@author	Masayuki Onoue
 *	@date		2011.10.11
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
EOS
out_header_file.puts header % [OUT_HEADER_FILE]

room_data_list.each_with_index { |item,i|
  out_header_file.puts "#define INF_ROOM_POS_TYPE_" + item.type_name + " (%d)"%i
}

max = room_data_list.size
out_header_file.puts "#define INF_ROOM_POS_TYPE_NONE (%d) // トレーナーを配置しない"%max
out_header_file.puts "#define INF_ROOM_POS_TYPE_MAX  (%d)"%[max+1]

out_file.close
out_header_file.close



header =  <<EOS
#/[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		%s
#*	@brief  無限ダンジョン内部屋データハッシュファイル
#*	@author	Masayuki Onoue
#*	@date		2011.11.18
#*  @note   コンバータによって出力されたファイルです
#*
#*/
#/]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$room_type_hash = {
EOS
file_name = "room_type_hash.rb"
out_file = open( file_name ,"w" )
out_file.puts header % [file_name]

room_data_list.each_with_index { |item,i|
  out_file.puts '  "%s" => %d,'%[item.type_name,i]
}
# 何も置かない部屋タイプ登録
out_file.puts '  "NONE" => %d,'%[room_data_list.size]
out_file.puts "}"

out_file.close

# チェック用データ出力
header =  <<EOS
#/[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		%s
#*	@brief  無限ダンジョン内部屋人数ハッシュファイル
#*	@author	Masayuki Onoue
#*	@date		2011.11.18
#*  @note   コンバータによって出力されたファイルです
#*
#*/
#/]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$room_member_hash = {
EOS
file_name = "room_member_num_hash.rb"
out_file = open( file_name ,"w" )
out_file.puts header % [file_name]

room_data_list.each { |item|
  out_file.puts '  "%s" => %d,'%[item.type_name,item.member_num]
}
# 何も置かない部屋タイプ登録
out_file.puts '  "NONE" => 0,'
out_file.puts "}"
 
