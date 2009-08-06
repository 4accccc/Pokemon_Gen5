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

  # アクセッサ(ゲッター)メソッド
  def weather
    @weather
  end

  def month
    @month
  end

  def day
    @day
  end

  # アクセッサ(セッター)メソッド
  def weather=( w )
    @weather = w
  end

  def month=( m )
    @month = m
  end

  def day=( d )
    @day = d
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

  # データ数取得
  def GetDataNum
    @data.length
  end

  # データ取得メソッド
  def GetZoneID
    @zone_id
  end

  def GetWeatherNo( i )
    @data[i].weather
  end

  def GetMonth( i )
    @data[i].month
  end

  def GetDay( i )
    @data[i].day
  end

  # データ設定メソッド
  def SetWeatherNo( i, weather_no )
    @data[i].weather = weather_no
  end

  def SetMonth( i, month )
    @data[i].month = month
  end

  def SetDay( i, day )
    @data[i].day = day
  end

  # データ追加メソッド
  def AddData( weather_info )
    @data << weather_info
  end

  # 圧縮メソッド
  def Compress
    prev = -1
    del  = false
    @data.reject! do |d| 
      # 前日と同じ天気なら, 削除する
      if d.weather == prev then 
        del = true  
      else
        del = false
      end   
      prev = d.weather
      del
    end
  end

  # 文字列取得メソッド
  def to_s
    str = "zone id = #@zone_id\n"
    str += "data num = "
    str += (@data.length).to_s
    str += "\n"
    @data.each do |d|
      str += d.to_s
      str += "\n"
    end
    str
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

info_array = Array.new

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
  zone_id    = GetZoneIDValue( zone_name )
  info_array << ZoneWeatherInfo.new( zone_id )
end

# 全データを読み込む
LINE_INDEX_START.upto( lines.length - 1 ) do |i|
  # 日にちを取得し, 各ゾーンのデータに設定
  item   = lines[i].split( /\t/ ) 
  month  = GetMonthValue( item[ ROW_INDEX_DATE ] )
  day    = GetDayValue( item[ ROW_INDEX_DATE ] )
  info_array.each do |info|
    info.AddData( WeatherInfo.new( month, day, 0 ) )
  end

  # 各ゾーンの天気を取得
  0.upto( info_array.length - 1 ) do |j|
    str_weather = item[ ROW_INDEX_FIRST_ZONE + j ].strip
    weather_no  = GetWeatherNo( str_weather )
    info_array[j].SetWeatherNo( i - LINE_INDEX_START, weather_no )
  end
end


# DEBUG:
=begin
info_array.each do |info|
  puts info.to_s
end
=end


#-----------------------------------
# 圧縮
#----------------------------------- 
# 前日と同じ天気のデータを消去
info_array.each do |info|
  info.Compress
end

# DEBUG:
=begin
info_array.each do |info|
  puts info.to_s
end
=end

#-----------------------------------
# オフセット計算
#----------------------------------- 
offset = Array.new

pos = 2 + 4 * info_array.length
0.upto( info_array.length - 1 ) do |i|
  offset << pos
  pos += 2
  pos += info_array[i].GetDataNum * 3
end


#-----------------------------------
# 書き込み
#-----------------------------------

# ファイルを開く
file = File.open( DST_FILENAME, "wb" )

# OUT: 登録ゾーン数(u16)
file.write( [ info_array.length ].pack( "S" ) )

# すべてのゾーンIDとオフセットの組を出力する
0.upto( info_array.length - 1 ) do |i|
  # OUT: ゾーンID(u16), オフセット(u16)
  file.write( [ info_array[i].GetZoneID, offset[i] ].pack( "SS" ) )
end

# 全ゾーンのデータを出力
info_array.each do |info|
  # OUT: データ数(u16)
  file.write( [ info.GetDataNum ].pack( "S" ) )
  # OUT: 月(u8)・日(u8)・天気(u8)
  0.upto( info.GetDataNum - 1 ) do |i|
    file.write( [ info.GetMonth(i), info.GetDay(i), info.GetWeatherNo(i) ].pack( "CCC" ) )
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
