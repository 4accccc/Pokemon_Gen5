##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		trainer_move_type_cnv.rb
#*	@brief  無限ダンジョン内トレーナー移動パターン
#*	@author	Masayuki Onoue
#*	@data		2011.04.06
#*
#*/
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"

#define
NO_COLUMN_INDEX = 0
TYPE_COLUMN_INDEX = 1
MOVE_CODE_COLUMN_INDEX = 2
EVENT_TYPE_COLUMN_INDEX = 3
DIRECTION_COLUMN_INDEX = 4
PARAMETER_0_COLUMN_INDEX = 5
PARAMETER_1_COLUMN_INDEX = 6
LIMIT_X_COLUMN_INDEX = 7
LIMIT_Z_COLUMN_INDEX = 8

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

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  トレーナー移動データ
 *	@author	Masayuki Onoue
 *	@date		2011.02.25
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//-------------------------------------
///	MMDL用パラメータ
//=====================================
typedef struct {
  u16 move_code;
  u16 ev_type;
  u16 direction;
  u16 param0;
  u16 param1;
  s32 lim_x;
  s32 lim_z;
}INF_MMDL_PARAM;

static const INF_MMDL_PARAM sc_mmdl_param[] = {
EOS
out_file.puts header % OUT_FILE

def_array = Array.new
data_csv.each { |item|

  if item[0].include?("#END") then
    break
  end
  
  out_file.print '    { ' 
  put_str( item[MOVE_CODE_COLUMN_INDEX] ,out_file )
  put_str( item[EVENT_TYPE_COLUMN_INDEX] ,out_file )
  put_str( item[DIRECTION_COLUMN_INDEX] ,out_file )
  put_value( item[PARAMETER_0_COLUMN_INDEX] ,out_file )
  put_value( item[PARAMETER_1_COLUMN_INDEX] ,out_file )
  put_value( item[LIMIT_X_COLUMN_INDEX] ,out_file )
  put_value( item[LIMIT_Z_COLUMN_INDEX] ,out_file )
  out_file.print "},"
  out_file.puts '  // INF_MOVE_CODE_TYPE_' + item[TYPE_COLUMN_INDEX].to_s 
  def_array.push('#define INF_MOVE_CODE_TYPE_' + item[TYPE_COLUMN_INDEX].to_s + " (%d)"  )
}

out_file.puts "};\n\n"

out_file.puts "// ナンバー定義"
# 定義値の出力
def_array.each_with_index{|item,i|
  out_file.puts item % i
}

out_file.close

