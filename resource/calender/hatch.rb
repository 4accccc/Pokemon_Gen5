###################################################################
#
# @brief  孵化・カレンダーのコンバート
# @author obata
# @date   2009.08.04
#
# === バイナリ・データ・フォーマット ===
# 
# [0] データ数(u16)
# [1]
# [2] 月(u8)              |
# [3] 日(u8)              |----- データ数の分だけ繰り返す
# [4] 孵化変化番号(s8)    |
# 
###################################################################


#==================================================================
#
# @brief 1対の月・日・孵化変化番号を保持するクラス
#
#==================================================================
class HatchInfo

  # コンストラクタ
  def initialize( m, d, val )
    @month = m
    @day   = d
    @no    = val
  end 

  # ゲッター・メソッド
  def month
    @month
  end

  def day
    @day
  end

  def no
    @no
  end

  # 文字列取得メソッド
  def to_s
    "#@month/#@day = #@no"
  end 

end

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

LINE_INDEX_START = 1 # 実データの先頭行番号
ROW_INDEX_DATE   = 2 # 日付データの列番号
ROW_INDEX_HATCH  = 3 # 孵化変化番号の列番号

SRC_FILENAME = "hatch.txt"  # 入力ファイル名
DST_FILENAME = "hatch.bin"  # 出力ファイル名


line_index = 0;        # 行インデックス
data       = Array.new # 全[月・日・孵化番号]データ


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
    month    = GetMonthValue( line_split[ ROW_INDEX_DATE ] )
    day      = GetDayValue( line_split[ ROW_INDEX_DATE ] )
    hatch_no = line_split[ ROW_INDEX_HATCH ].to_i
    data    << HatchInfo.new( month, day, hatch_no )

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
file.write( [ data.length ].pack( "S" ) )

# すべてのデータを出力する
0.upto( data.length - 1 ) do |i| 

  # OUT: 月(u8), 日(u8), 変化番号(u8)
  write_data = [ data[i].month, data[i].day, data[i].no ]
  file.write( write_data.pack( "CCC" ) )
  
end

# ファイルを閉じる
file.close


#-----------------------------------
# DEBUG:
#-----------------------------------
=begin
puts "data num = " + data.length.to_s
0.upto( data.length - 1 ) do |i|
  puts data[i].to_s
end
=end
