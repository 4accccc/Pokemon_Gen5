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
HINT_MSG_INDEX = 9

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
  put_str( item[HINT_MSG_INDEX] ,out_file )
  out_file.print "},"
  out_file.puts '  // INF_MOVE_CODE_TYPE_' + item[TYPE_COLUMN_INDEX].to_s 
  def_array.push('#define INF_MOVE_CODE_TYPE_' + item[TYPE_COLUMN_INDEX].to_s + " (%d)"  )
}

out_file.puts "};\n\n"

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  トレーナー移動データタイプ定義ファイル
 *	@author	Masayuki Onoue
 *	@date		2011.10.11
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

EOS
out_header_file.puts header % OUT_HEADER_FILE

# 定義値の出力
def_array.each_with_index{|item,i|
  out_header_file.puts item % i
}
out_header_file.close
out_file.close

