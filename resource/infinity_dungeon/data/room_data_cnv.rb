##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		room_data_cnv.rb
#*	@brief  無限ダンジョン内トレーナー配置パターン
#*	@author	Masayuki Onoue
#*	@data		2011.04.06
#*
#*/
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"

#define
AREA_NO_COLUMN_INDEX = 0
ROOM_TYPE_COLUMN_INDEX = 1
PROBABILITY_COLUMN_INDEX = 2

class ROOM_DATA
  attr_accessor :room_type, :probability
end

class AREA_DATA
  attr_accessor :area_no, :room_data, :member_num
  def initialize( no )
    @area_no = no
    @room_data = Array.new
    @member_num = 0
  end
  def push_data( data )
    @member_num+=1
    @room_data.push(data)
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

area_no = 0
room_data_list = Array.new
ad = nil
data_csv.each { |item|
  unless item[0] == nil then
    if item[0].include?("#END") then
      room_data_list.push(ad)
      break
    end
  end
  
  if item[0] == nil then
    rd = ROOM_DATA.new
    rd.room_type = item[ROOM_TYPE_COLUMN_INDEX]
    rd.probability = item[PROBABILITY_COLUMN_INDEX]
    ad.push_data(rd)
  else
    unless ad == nil then
      room_data_list.push(ad)
    end
    ad = AREA_DATA.new(area_no)
    rd = ROOM_DATA.new
    rd.room_type    = item[ROOM_TYPE_COLUMN_INDEX]
    rd.probability  = item[PROBABILITY_COLUMN_INDEX]
    ad.push_data(rd)
    area_no+=1
  end
}

mem_max = 0
room_data_list.each { |item|
  if mem_max < item.member_num then
    mem_max = item.member_num
  end
}

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョン内部屋データ
 *	@author	Masayuki Onoue
 *	@date		2011.04.09
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#define INF_AREA_ROOM_SET_MAX (%d)

//-------------------------------------
///	部屋データ
//=====================================
typedef struct {
  u32 tr_pos_type;
  u32 probability;
}INF_ROOM_DATA;

//-------------------------------------
///	部屋設置用データ
//=====================================
typedef struct {
  u8  member_num;
  INF_ROOM_DATA room_setup_data[INF_AREA_ROOM_SET_MAX];
}INF_ROOM_SETUP_PARAM;

static const INF_ROOM_SETUP_PARAM sc_room_setup_param[] = {
EOS
out_file.puts header % [OUT_FILE,mem_max]

cmt = <<EOS
  {
EOS

room_data_list.each { |item|

  out_file.print cmt
  out_file.print "    "
  put_value( item.member_num ,out_file )
  out_file.print "\n    { " 
  item.member_num.times{|i|
    out_file.print "\n      { " 
    put_str( "INF_ROOM_POS_TYPE_"+item.room_data[i].room_type  ,out_file)
    put_value( item.room_data[i].probability ,out_file )
    out_file.puts '}, '
  }
  out_file.print "\n    },\n" 

  out_file.puts '  }, ' 

}
out_file.puts "};"

room_data_list.each_with_index { |item,i|
  out_file.puts "#define INF_ROOM_SETUP_TYPE_" + item.area_no.to_s + " (%d)"%i
}

max = room_data_list.size
out_file.puts "#define INF_ROOM_SETUP_TYPE_MAX (%d) // エリア設定データの最大数"%max
out_file.close

