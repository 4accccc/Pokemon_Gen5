##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		achieve_table_cnv.rb
#*	@brief  達成用関数テーブルコンバータ
#*	@author	Masayuki Onoue
#*	@data		2011.07.5
#*
#*/
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"

H_FILE_NAME = ARGV[0]
CSV_FILE_NAME = ARGV[1]
OUT_FILE_NAME = ARGV[2]

# メダルデータの読み込み
data_h    = open( H_FILE_NAME ,"r")
data_csv  = CSV.open( CSV_FILE_NAME ,"r")
out_file  = open( OUT_FILE_NAME ,"w" )

# 読み飛ばし
2.times{
  data_csv.shift
}

# テーブル生成
id_offset = 0
table_hash = Hash.new
data_h.each_with_index{|line,index|
  # 最後は飛ばす
  if line.include?("MDLRECID_MAX") then break end

  # ヘッダの解釈
  if  /#define\s+(.*)\s+\((.*)\)/ =~ line then
    if line.include?("MDLRECID_START") then
      id_offset = $2.strip.to_i
    else
      id = $2.strip.to_i - id_offset
      #puts id.to_s+"-"+$1.to_s+","
      table_hash[ $1 ] = id
    end
  end
}

size = table_hash.size
table_arry = Array.new( size ,"NULL")
id_table_arry = Array.new( size )
MDLRECID_INDEX  = 0
PROC_NAME_INDEX = 1
# CSVファイルの解釈
data_csv.each_with_index{|line,index|
  if line.include?("#END") then break end
  #puts line[MDLRECID_INDEX] + line[PROC_NAME_INDEX]
  unless table_hash.key?( line[MDLRECID_INDEX] ) then
    raise "未定義のメダルレコードIDが記入されています->" + line[MDLRECID_INDEX]
  end
  array_index = table_hash[ line[MDLRECID_INDEX] ]
  table_arry[array_index] = line[PROC_NAME_INDEX]
  id_table_arry[array_index] = line[MDLRECID_INDEX]
}

header =<<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		mdlachieve_proc_table.cdat
 *	@brief  メダル達成処理関数テーブル
 *	@author	Masayuki Onoue
 *	@date		2011.07.05
 *	@note		/resource/medal/converter/%sが出力しています
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

MDLACHIEVEPROC sc_achieve_proc[] = {
EOS

out_file.puts header%[File.basename(__FILE__)]

# 関数名を書き込む
table_arry.each_with_index{|proc_name,i|
  unless id_table_arry[i] == nil then
    out_file.puts "  "+proc_name +", // " + id_table_arry[i].to_s
  else
    out_file.puts "  "+proc_name +","
  end
}
out_file.puts "};"

data_h.close
data_csv.close
out_file.close

