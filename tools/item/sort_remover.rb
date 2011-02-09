#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		sort_remover.rb
#*	@brief  アイテムデータからソート用データを取り除いたCSVファイルを作成する
#*	@author	Masayuki Onoue
#*	@date		2011.02.09
#*
#*/
#/]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"

#/=============================================================================
#**
#*					定数宣言
#/
#/=============================================================================

# 分離する列
REMOVE_LINE = [26,27] #[sort1,sort2]

if ARGV.size < 3
  puts "引数が足りません。"
  puts "引数1：入力ファイル名"
  puts "引数2：アイテムCSV出力ファイル名"
  puts "引数3：ソート出力ファイル名"
  exit( 1 )
end

CONVERT_FILE_NAME = ARGV[0]
ITEM_FILE_NAME = ARGV[1]
SORT_FILE_NAME = ARGV[2]

# ファイルを開く
in_file = CSV.open(CONVERT_FILE_NAME, 'r' )
item_file = CSV.open(ITEM_FILE_NAME, 'w',nil,"\r\n")
sort_file = CSV.open(SORT_FILE_NAME, 'w',nil,"\r\n")


# アイテムデータを出力
in_file.each do |row|

  # ラインオブジェクトのディープコピー
  line_obj = Marshal.load(Marshal.dump(row))

  # ソーとデータを取り出す
  sort_data = Array.new
 
  # アイテム名を取得
  sort_data << row[2]
 
  # ソーと番号の取得
  REMOVE_LINE.each { |index|
    sort_data.push(line_obj[index])
  }

  # 特定列の削除(逆順に消さないと、消すたびに順番がずれる)
  REMOVE_LINE.reverse_each { |index|
    line_obj.delete_at(index)
  }

  # ソーとデータを書き込む
  sort_file << sort_data

  # アイテムファイルに出力
  item_file << line_obj
end
