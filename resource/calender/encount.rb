###################################################################
#
# @brief  エンカウント・カレンダーのコンバート
# @author obata
# @date   2009.08.04
#
# === バイナリ・データ・フォーマット ===
# 
# [0] データ数(u16)
# [1]
# [2] 月(u8)              |
# [3] 日(u8)              |----- データ数の分だけ繰り返す
# [4] 出現率の変化値(u8)  |
# 
###################################################################

#==================================================================
#
# @brief 日付文字列から, 月の数値を取り出す
#
# @param str_date 日付を表す文字列(*月*日)
#
# @return 月の値
#
#==================================================================
def GetMonthValue( str_date )

  # "月"の位置を求める
  month_index = str_date.index( "月" )

  # 月・日の値を数値に変換
  val = str_date[0..month_index-1].to_i

  # 月の値を返す
  return val

end
  

#==================================================================
#
# @brief 日付文字列から, 日の数値を取り出す
#
# @param str_date 日付を表す文字列(*月*日)
#
# @return 日の値
#
#==================================================================
def GetDayValue( str_date )

  # 月・日の位置を求める
  month_index = str_date.index( "月" )
  date_index  = str_date.index( "日" )

  # 日の値を数値に変換
  val = str_date[month_index+2..date_index-1].to_i

  # 日の値を返す
  return val

end


#==================================================================
#
# @brief メイン
#
#==================================================================

LINE_INDEX_START      = 1 # 実データの先頭行番号
ROW_INDEX_DATE        = 2 # 日付データの列番号
ROW_INDEX_ENCOUNT_NO  = 3 # 出現率番号の列番号

SRC_FILENAME = "encount.txt"  # 入力ファイル名
DST_FILENAME = "encount.bin"  # 出力ファイル名


line_index = 0;    # 行インデックス
month = Array.new  # 月配列
date  = Array.new  # 日配列
rate  = Array.new  # 出現率配列


#-----------------------------------
# 読み込み
#-----------------------------------

# ファイルを開く
file  = File.open( SRC_FILENAME )

# すべての行について処理する
file.each do |line|

	# 空データが来たら, 終了
	empty_test = line.gsub( /\s/, "" )
	if( empty_test == "" ) then
		break;
	end

  # 実データのみを処理する
	if( LINE_INDEX_START <= line_index ) then

		# タブで区切る
		line_split = line.split( /\t/ )

    # 各データを取り出す
    month << GetMonthValue( line_split[ ROW_INDEX_DATE ] )
    date  << GetDayValue( line_split[ ROW_INDEX_DATE ] )
    rate  << line_split[ ROW_INDEX_ENCOUNT_NO ].to_i

  end

	# 行数をカウント
	line_index += 1
end

# ファイルを閉じる
file.close


#-----------------------------------
# 書き込み
#-----------------------------------

# ファイルを開く
file = File.open( DST_FILENAME, "wb" )

# OUT: データ数(u16)
file.write( [ date.length ].pack( "S" ) )

# すべてのデータを出力する
0.upto( date.length - 1 ) do |i| 

  # OUT: 月(u8), 日(u8), 出現率番号(u8)
  write_data = [ month[i], date[i], rate[i] ]
  file.write( write_data.pack( "CCC" ) )
  
end

# ファイルを閉じる
file.close


#-----------------------------------
# DEBUG:
#-----------------------------------
=begin
puts "data num = " + date.length.to_s
0.upto( date.length - 1 ) do |i|
  puts month[i].to_s + "/" + date[i].to_s + " = " + rate[i].to_s
end
=end
