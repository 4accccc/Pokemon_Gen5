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

class POS_DATA
  attr_accessor :move_type, :pos_x, :pos_z
end

class ROOM_DATA
  attr_accessor :type_name, :member_num, :pos_data
  def initialize
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

OUT_FILE = ARGV[0]
CSV_FILE = ARGV[1]

out_file = open( OUT_FILE ,"w" )

# データの読み込み
data_csv = CSV.open( CSV_FILE ,"r")

# データの読み飛ばし
data_csv.shift

room_data_list = Array.new
rd = nil
data_csv.each { |item|
  unless item[0] == nil then
    if item[0].include?("#END") then
      room_data_list.push(rd)
      break
    end
  end
  
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
    rd.push_data(pos)
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

#define ROOM_TR_MAX (4)

//-------------------------------------
///	部屋データ
//=====================================
typedef struct {
  u16 move_type;
  u32 g_x;
  u32 g_y;
}INF_ROOM_TR_DATA;

//-------------------------------------
///	配置パラメータ
//=====================================
typedef struct {
  u8  member_num;
  INF_ROOM_TR_DATA tr_move_data[ROOM_TR_MAX];
}INF_ROOM_SET_TR_PARAM;

static const INF_ROOM_SET_TR_PARAM sc_room_tr_param[] = {
EOS
out_file.puts header % OUT_FILE

cmt = <<EOS
  { // INF_MOVE_CODE_TYPE_%s
EOS

room_data_list.each { |item|

  out_file.print cmt % item.type_name
  out_file.print "    "
  put_value( item.member_num ,out_file )
  out_file.print "\n    { " 
  item.member_num.times{|i|
    out_file.print "\n      { " 
    put_str( "INF_MOVE_CODE_TYPE_"+item.pos_data[i].move_type,out_file )
    put_value( item.pos_data[i].pos_x,out_file )
    put_value( item.pos_data[i].pos_z,out_file )
    out_file.puts '}, '
  }
  out_file.print "\n    },\n" 

  out_file.puts '  }, ' 

}
out_file.puts "};"

max = room_data_list.size
out_file.puts "#define INF_TR_ROOM_POS_TYPE_MAX (%d) // 部屋タイプの最大数"%max
out_file.close

