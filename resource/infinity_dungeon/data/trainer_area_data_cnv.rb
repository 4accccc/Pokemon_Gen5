##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		trainer_area_data_cnv.rb
#*	@brief  ROMごとのトレーナーの出現パターン
#*	@author	Masayuki Onoue
#*	@data		2011.04.27
#*
#*/
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"

START_AREA_COLUMN_INDEX = 1
END_AREA_COLUMN_INDEX = 2
VERSION_COLUMN_INDEX = 3

B2_ONLY = "B"
W2_ONLY = "W"

def put_str( value ,file )
  file.print value.to_s+" ,"
end

def put_value( value ,file )
  file.print value
  file.print " ,"
end

OUT_FILE = ARGV[0]
CSV_FILE = ARGV[1]

# 書き込みファイルオープン
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
 *	@brief  無限ダンジョン：ROMごとのトレーナーの出現パターン
 *	@author	Masayuki Onoue
 *	@date		2011.04.27
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "pm_version.h"

//-------------------------------------
///	トレーナーデータ
//=====================================
typedef struct {
  u8 start_area;
  u8 end_area;
  u8 version;
}INF_TR_AREA_DATA;

static const INF_TR_AREA_DATA sc_tr_area_data[] = {
EOS
out_file.puts header % [OUT_FILE]

index_max = 0
data_csv.each_with_index { |item,index|
  out_file.printf " { "
  put_value( item[START_AREA_COLUMN_INDEX].to_i-1  ,out_file)
  put_value( item[END_AREA_COLUMN_INDEX].to_i-1  ,out_file)

  version = ""
  # Version判定
  if item[VERSION_COLUMN_INDEX] == B2_ONLY then
    version = "VERSION_B2"
  elsif item[VERSION_COLUMN_INDEX] == W2_ONLY then
    version = "VERSION_W2"
  else
    puts item[VERSION_COLUMN_INDEX]
    raise "%d行目->バージョン指定文字に誤りがあります"%(index+2)  # インデックスは０スタートでかつ、1行目はタイトル行なので+2
  end

  put_str( version ,out_file)

  index_max = index + 1
  out_file.printf "},\n"
}
out_file.printf "};\n"

out_file.printf "#define INF_TR_AREA_DATA_MAX (%d)\n"%index_max

data_csv.close
out_file.close
