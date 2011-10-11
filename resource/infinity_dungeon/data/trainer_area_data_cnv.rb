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
SETLV_COLUMN_INDEX = 4

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

OUT_FILE        = ARGV[0]
OUT_HEADER_FILE = ARGV[1]
CSV_FILE        = ARGV[2]

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

lv1_tr_cnt_w = Array.new(10,0)
lv1_tr_cnt_b = Array.new(10,0)

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

  set_lv = ""
  # トレーナーの出現確率判定
  if item[SETLV_COLUMN_INDEX] == "◎" then
    set_lv = "INF_TRAINER_SETLV_1"  # 100%
  elsif item[SETLV_COLUMN_INDEX] == "○" then
    set_lv = "INF_TRAINER_SETLV_2"
  elsif item[SETLV_COLUMN_INDEX] == "×" then
    set_lv = "INF_TRAINER_SETLV_3"
  else
    raise "◎、○、×以外が指定されています。"
  end

  put_str( set_lv ,out_file)

  index_max = index + 1
  out_file.printf "},\n"

  if item[SETLV_COLUMN_INDEX] == "◎"  then
    if item[VERSION_COLUMN_INDEX] == B2_ONLY then
      # トレーナーでータ数をチェック
      (item[START_AREA_COLUMN_INDEX].to_i-1).upto( item[END_AREA_COLUMN_INDEX].to_i-1  ){|i|
        lv1_tr_cnt_b[i] += 1
      }
    elsif item[VERSION_COLUMN_INDEX] == W2_ONLY then
      # トレーナーでータ数をチェック
      (item[START_AREA_COLUMN_INDEX].to_i-1).upto( item[END_AREA_COLUMN_INDEX].to_i-1  ){|i|
        lv1_tr_cnt_w[i] += 1
      }
    elsif item[VERSION_COLUMN_INDEX] == WW2_BOTH then
      # トレーナーでータ数をチェック
      (item[START_AREA_COLUMN_INDEX].to_i-1).upto( item[END_AREA_COLUMN_INDEX].to_i-1  ){|i|
        lv1_tr_cnt_w[i] += 1
        lv1_tr_cnt_b[i] += 1
      }
    end
  end

}
out_file.printf "};\n"

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

# 最大数チェック
lv1_tr_cnt_w.each_with_index{|data,i|
  puts "Wエリア%d:%d"%[i,data]
  if data < 32 then
    raise "エリア%s（ホワイト）：出現率100%%[◎]が設定されているものが少なすぎます(%d/32)\n"%[i+1,data]
  end
}
lv1_tr_cnt_b.each_with_index{|data,i|
  puts "Bエリア%d:%d"%[i,data]
  if data < 32 then
    raise "エリア%s（ブラック）：出現率100%%[◎]が設定されているものが少なすぎます(%d/32)\n"%[i+1,data]
  end
}

data_csv.close
out_header_file.close
out_file.close
