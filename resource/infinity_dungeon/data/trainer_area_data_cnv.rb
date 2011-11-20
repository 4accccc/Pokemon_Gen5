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
require File.dirname(__FILE__) + '/../../../tools/hash/trtype_hash.rb'
require File.dirname(__FILE__) + '/../../../tools/btl_subway/bts_tr_arr.rb'

START_AREA_COLUMN_INDEX = 1
END_AREA_COLUMN_INDEX = 2
VERSION_COLUMN_INDEX = 3

B2_ONLY = "B"
W2_ONLY = "W"
WW2_BOTH = "WB"

def put_str( value ,file )
  file.print value.to_s+" ,"
end

def put_value( value ,file )
  file.print value
  file.print " ,"
end

# トレーナータイプごとに人数をカウントする
def get_tr_cnt_data( data_csv,tr_cnt_array )
  data_csv.shift
  data_csv.shift
  tr_type_cnt = 0
  data_csv.each{|line|
    # 終了判定
    if line[0].include?("#END") then break end

    # データ収集
    if $trtype_hash.key?(line[0]) then
      10.times{|i|
        tr_type = $trtype_hash[ line[0] ]
        tr_cnt_array[i][tr_type] = line[i+1]
      }
      tr_type_cnt+=1
    else
      raise "不明なトレーナータイプ指定%s\n"%[line[0]]
    end
  }
  return tr_type_cnt
end

# カウントしたデータを出力する
def out_put_tr_cnt_data( tr_cnt_file ,tr_cnt_array ,tr_type_list )
  10.times{|i|
    tr_cnt_file.print "  { // エリア%d\n"%[i+1]
    tr_cnt_file.printf "    "

    # 全データ出力
    tr_type_list.each_with_index{|tr_type,cnt|
      if tr_cnt_array[i].key?(tr_type) then
        tr_cnt_file.printf "%3d,"%[tr_cnt_array[i][tr_type].to_i]
      end
      # 改行
      if ((1+cnt)%5 == 0 ) then tr_cnt_file.printf "\n    " end
    }
    tr_cnt_file.puts "  },"
  }
  tr_cnt_file.puts "};"
end

OUT_FILE        = ARGV[0]
OUT_HEADER_FILE = ARGV[1]
CSV_FILE        = ARGV[2]
TR_DATA_FILE    = ARGV[3]
TR_CNT_FILE_W   = ARGV[4]
TR_CNT_FILE_B   = ARGV[5]
OUT_TR_CNT_FILE = ARGV[6]

tr_type_array = Array.new

# 配置人数データの読み込み
tr_cnt_array_w = Array.new
tr_cnt_array_b = Array.new
tr_type_list = Array.new
10.times{|i|
  tr_cnt_array_w[i] = Hash.new
  tr_cnt_array_b[i] = Hash.new
}

# タイプリスト（BとWで必ず同じ）
tr_type_cnt = 0
data_csv = CSV.open( TR_CNT_FILE_W ,"r")
data_csv.shift
data_csv.shift
data_csv.each{|line|
  if line[0].include?("#END") then break end
  if $trtype_hash.key?(line[0]) then
    tr_type_list << $trtype_hash[ line[0] ]
    tr_type_cnt+=1
  else
    raise "不明なトレーナータイプ指定%s\n"%[line[0]]
  end
}
data_csv.close

# データの読み込み(ホワイト)
data_csv = CSV.open( TR_CNT_FILE_W ,"r")
get_tr_cnt_data( data_csv ,tr_cnt_array_w)
data_csv.close

# データの読み込み(ブラック)
puts TR_CNT_FILE_B
data_csv = CSV.open( TR_CNT_FILE_B ,"r")
get_tr_cnt_data( data_csv ,tr_cnt_array_b)
data_csv.close

# ===================================================
# トレーナーデータファイル
tr_data_file = open( TR_DATA_FILE ,"r" )
tr_data_file.each_with_index{|line,i|
  split_data = line.split(/\t/)
  tr_type_index = $bts_trainername_array[ split_data[1].to_i ]
  tr_type = $trtype_hash[ tr_type_index ]
  tr_type_array[i] = tr_type
}
tr_data_file.close

# 書き込みファイルオープン
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
 *	@brief  無限ダンジョン：バージョンごとのトレーナーの出現パターン
 *	@author	Masayuki Onoue
 *	@date		2011.04.27
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

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
  elsif item[VERSION_COLUMN_INDEX] == WW2_BOTH then
    version = "0"
  else
    puts item[VERSION_COLUMN_INDEX]
    raise "%d行目->バージョン指定文字に誤りがあります"%(index+2)  # インデックスは０スタートでかつ、1行目はタイトル行なので+2
  end

  put_str( version ,out_file)

  out_index = tr_type_list.index( tr_type_array[index] )
  if out_index.nil? then
    puts tr_type_array[index]
    raise "トレーナータイプが発見できませんでした。:No%d\n"%[index+1]
  end
  put_value( out_index ,out_file)

  index_max = index + 1
  out_file.printf "},\n"

}
out_file.printf "};\n"

# ===================================================
# トレーナーデータ定義

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョン：バージョンごとのトレーナーの出現パターン
 *	@author	Masayuki Onoue
 *	@date		2011.10.11
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

EOS
out_header_file.puts header % [OUT_HEADER_FILE]

out_header_file.printf "#define INF_TR_AREA_DATA_MAX (%d)\n"%index_max
out_header_file.printf "#define INF_TR_TYPE_MAX (%d)\n"%tr_type_cnt

data_csv.close
out_header_file.close
out_file.close

# ===================================================
# トレーナー比率出力

header =  <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  無限ダンジョン：トレーナー配置データ
 *	@author	Masayuki Onoue
 *	@date		2011.11.19
 *  @note   コンバータによって出力されたファイルです
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// ホワイト用
static const u8 sc_inf_tr_setdata_w[10][INF_TR_TYPE_MAX] = 
{
EOS
tr_cnt_file = open( OUT_TR_CNT_FILE ,"w" )
tr_cnt_file.puts header % [OUT_HEADER_FILE]
out_put_tr_cnt_data( tr_cnt_file ,tr_cnt_array_w ,tr_type_list)

header =  <<EOS

// ブラック用
static const u8 sc_inf_tr_setdata_b[10][INF_TR_TYPE_MAX] = 
{
EOS
tr_cnt_file.puts header
out_put_tr_cnt_data( tr_cnt_file ,tr_cnt_array_b ,tr_type_list)

tr_cnt_file.close

