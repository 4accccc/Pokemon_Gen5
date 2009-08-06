###################################################################
#
# @brief  天気カレンダーのコンバート
# @author obata
# @date   2009.08.04
#
# === バイナリ・データ・フォーマット ===
# 
# [0] zone_num (登録ゾーン数:u16)
# [1]
# [2] zone_id (ゾーンID:u16)								|
# [3]																				|--- x zone_num
# [4] offset (データへのオフセット:u16)			|
# [5]																				|
# [x+0] date_num (登録日付数:u16)											|
# [x+1]																								|
# [y+0] month   (月:u8)							|									|--- x zone_num
# [y+1] date    (日:u8)							|--- x date_num		|
# [y+2] weather (天気:u8)						|									|
# 
###################################################################


#==================================================================
#
# @brief 1対の月・日・天気を保持するクラス
#
#==================================================================
class WeatherInfo

  # コンストラクタ
  def initialize( month, day, weather )
    @month   = month    # 月
    @day     = day      # 日
    @weather = weather  # 天気
  end

  # 文字列取得メソッド
  def to_s
    "#@month/#@day = #@weather"
  end 

end


#==================================================================
#
# @brief 1つのゾーンのデータを保持するクラス
#
#==================================================================
class ZoneWeatherInfo

  # コンストラクタ
  def initialize( zone )
    @zone_id = zone        # ゾーンID
    @data    = Array.new   # 天気情報配列
  end

  # データ追加メソッド
  def AddData( data )
    @data << data
  end

  # 圧縮メソッド
  def Compress
    prev = -1
    del  = false;
    @data.reject! do |d| 
      # 前日と同じ天気なら, 削除する
      if d.weather == prev then 
        del = true  
      elsif
        del = false
      end   
      prev = d.weather
      del
    end
  end

  # 文字列取得メソッド
  def to_s
    str = "zone id = #@zone_id, data num = #data.length\n"
    @data.each do |d|
      str += d.to_s
      str += "\n"
    end
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
# @brief ゾーン名から, ゾーンIDを取得する
#
# @param name ゾーン名(例：T01)
#
# @return 指定ゾーンのID
#==================================================================
def GetZoneIDValue( name )

	zone_id = 0;

	# 検索する定数名を作成
	def_name = "ZONE_ID_" + name

	# ゾーンID定義ファイルを開く
	file_def = File.open( "../../prog/arc/fieldmap/zone_id.h" )

	# 1行ずつ見ていく
	file_def.each do |line|

		# 検索定数名が見つかった場合
		if( line.index( def_name ) != nil ) then

			# ()内の文字列を数値に変換する
			i0 = line.index( "(" ) + 1
			i1 = line.index( ")" ) - 1
			str_num = line[ i0..i1 ]
			zone_id = str_num.to_i 
			break
			
		end
		
	end
	
	# ファイルを閉じる
	file_def.close

	return zone_id
end


#==================================================================
#
# @brief 天気名を数値に変換する
#
# @param name 天気名( 例: SUNNY )
#
# @return 指定された天気の番号
#
#==================================================================
def GetWeatherNo( name )

  weather_no = 0

  # 定数名を作成
  def_name = "WEATHER_NO_" + name

  # 天気番号が定義されているファイルを開く
  file = File.open( "../../prog/include/field/weather_no.h" )

  # 定数名を検索
  file.each do |line|
    # 発見
    if line.index( def_name ) != nil then 
      # ( )内の値を取得
      i0         = line.index( "(" ) + 1
      i1         = line.index( ")" ) - 1
      str        = line[i0..i1];
      weather_no = str.to_i
      break
    end
  end

  # ファイルを閉じる
  file.close

  return weather_no
end


#==================================================================
#
# @brief メイン
#
#==================================================================

LINE_INDEX_ZONE_NAME  = 1 # 登録ゾーン名が指定されている行番号
LINE_INDEX_START      = 2 # 実データの先頭行番号
ROW_INDEX_DATE        = 2 # 日付データの列番号
ROW_INDEX_FIRST_ZONE  = 3 # 最初のゾーン指定の列番号

SRC_FILENAME = "weather.txt"  # 入力ファイル名
DST_FILENAME = "weather.bin"  # 出力ファイル名


zone_id    = Array.new # ゾーンID配列
month      = Array.new # 月配列(366日分)
date       = Array.new # 日にち配列(366日分)
weather    = Array.new # 天気配列(各ゾーン366日分)


#-----------------------------------
# 全データの読み込み
#-----------------------------------
# ファイル読み込み
file  = File.open( SRC_FILENAME )
lines = file.readlines
file.close

# 登録されている全ゾーンIDを取得
item  = lines[ LINE_INDEX_ZONE_NAME ].split( /\t/ )
ROW_INDEX_FIRST_ZONE.upto( item.length - 1 ) do |i|
  zone_name  = item[i].strip
  zone_id   << GetZoneIDValue( zone_name )
end

# すべての日にちを取得
LINE_INDEX_START.upto( lines.length - 1 ) do |i|
  item   = lines[i].split( /\t/ )
  month << GetMonthValue( item[ ROW_INDEX_DATE ] )
  date  << GetDayValue( item[ ROW_INDEX_DATE ] )
end

# 登録されている全ゾーンを処理する
0.upto( zone_id.length - 1 ) do |i|
  array = Array.new
  # 366日すべての天気を取得
  LINE_INDEX_START.upto( lines.length - 1 ) do |j|
    item         = lines[j].split( /\t/ )
    str_weather  = item[ ROW_INDEX_FIRST_ZONE + i ].strip
    array       << GetWeatherNo( str_weather )
  end
  weather << array
end


#-----------------------------------
# 圧縮
#----------------------------------- 
month_comp   = Array.new  
date_comp    = Array.new
weather_comp = Array.new

0.upto( zone_id.length - 1 ) do |i|
  # 前日と天気が変わらない日を削除(月)
  index = 0
  temp = month.reject { |val|
    del = false
    if( index == 0 ) then
      del = false
    elsif( weather[i][index-1] == weather[i][index] ) then
      del = true
    end
    index += 1
    del         # ブロック最後の評価で削除するかどうかが決まる
  } 
  month_comp << temp

  # 前日と天気が変わらない日を削除(日)
  index = 0
  temp = date.reject { |val|
    del = false
    if( index == 0 ) then
      del = false
    elsif( weather[i][index-1] == weather[i][index] ) then
      del = true
    end
    index += 1
    del         # ブロック最後の評価で削除するかどうかが決まる
  }
  date_comp << temp

  # 前日と天気が変わらない日を削除(天気)
  prev = -1 
  temp = weather[i].reject { |val|
    del = ( prev == val )
    prev = val
    del         # ブロック最後の評価で削除するかどうかが決まる
  }
  weather_comp << temp
end


=begin
0.upto( weather_comp[1].length - 1 ) do |i|
  puts month_comp[1][i].to_s + "/" + date_comp[1][i].to_s + " = " + weather_comp[1][i]
end
=end


#-----------------------------------
# オフセット計算
#----------------------------------- 
offset = Array.new

pos = 2 + 4 * zone_id.length
0.upto( zone_id.length - 1 ) do |i|
  offset << pos
  pos += 2
  pos += weather_comp[i].length * 3
end


=begin
0.upto( zone_id.length - 1 ) do |i|
  puts offset[i]
end
=end


#-----------------------------------
# 書き込み
#-----------------------------------

# ファイルを開く
file = File.open( DST_FILENAME, "wb" )

# OUT: 登録ゾーン数(u16)
file.write( [ zone_id.length ].pack( "S" ) )

# すべてのゾーンIDとオフセットの組を出力する
0.upto( zone_id.length - 1 ) do |i|
  # OUT: ゾーンID(u16), オフセット(u16)
  file.write( [ zone_id[i], offset[i] ].pack( "SS" ) )
end

# 全ゾーンのデータを出力
0.upto( zone_id.length - 1 ) do |i|
  # OUT: データ数(u16)
  file.write( [ weather_comp[i].length ].pack( "S" ) )
  # OUT: 月(u8)・日(u8)・天気(u8)
  0.upto( weather_comp[i].length - 1 ) do |j|
    file.write( [ month_comp[i][j], date_comp[i][j], weather_comp[i][j] ].pack( "CCC" ) )
  end
end

# ファイルを閉じる
file.close


#-----------------------------------
# DEBUG:
#-----------------------------------
=begin
0.upto( zone_id.length - 1 ) do |i|
  puts zone_id[i]
end
=end

=begin
0.upto( date.length - 1 ) do |i|
  str = month[i].to_s + "/" + date[i].to_s
  0.upto( zone_id.length - 1 ) do |j|
    str += " "
    str += weather[j][i].to_s
  end
  puts str
end
=end
