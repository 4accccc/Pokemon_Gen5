###############################################################################
#
# @brief WBカレンダーコンバータ
# @file  weather_wb.rb
# @author obata
# @date   2010.01.28
#
###############################################################################


#==============================================================================
# ■定数
#==============================================================================
# 列インデックス
COLUMN_DATE       = 3  # 日にち
COLUMN_FIRST_ZONE = 4  # 最初のゾーン

# 行インデックス
ROW_ZONE_ID    = 0  # ゾーンID
ROW_FIRST_DATE = 1  # 1月1日


#==============================================================================
# ■天気データ
#
# ※1日の天気を保持する。
#==============================================================================
class WeatherData

  def initialize
    @date_lavel    = nil   # 日にち ( ラベル )
    @weather_lavel = nil   # 天気   ( ラベル )
    @month         = 0     # 月
    @day           = 0     # 日
    @weather       = 0     # 天気
  end

  attr_reader :date_lavel, :weather_lavel, :month, :day, :weather

  #----------------------------------------
  # @brief 日付を設定する
  # @param date_lavel 日にち ( ラベル )
  #----------------------------------------
  def SetDate( date_lavel )
    # ラベル整形
    date_lavel.strip!
    date_lavel =~ /\s*(\d+)月\s*(\d+)日\s*/  # パターンマッチ
    # マッチせず
    if $1 == nil || $2 == nil then abort( "date lavel error. (#{date_lavel})" ) end
    # マッチした日付を記憶
    @date_lavel = date_lavel
    @month      = $1.to_i
    @day        = $2.to_i
  end

  #----------------------------------------
  # @brief 天気を設定する
  # @param weather_lavel 天気 ( ラベル )
  #----------------------------------------
  def SetWeather( weather_lavel )
    # ラベル整形
    weather_lavel.strip!
    # 参照ファイルデータ取得
    file = File.open( "#{ENV["PROJECT_PROGDIR"]}include/field/weather_no.h", "r" )
    lines = file.readlines
    file.close
    # 検索
    lines.each do |line|
      line =~ /#define\s*WEATHER_NO_#{weather_lavel}\s*\((\d+)\)/  # パターンマッチ
      # 発見
      if $1 != nil then
        @weather_lavel = weather_lavel
        @weather       = $1.to_i
        return
      end
    end
    # ラベルが見つからず
    abort( "weather lavel error. (#{weather_lavel})" )
  end

end

#==============================================================================
# ■カレンダー
#
# ※1年の天気を保持する。
#==============================================================================
class Calender

  def initialize
    @zoneID_lavel = nil        # ゾーンID (ラベル)
    @zoneID       = 0          # ゾーンID
    @weatherData  = Array.new  # 天気配列
  end

  attr_reader :zoneID_lavel, :zoneID, :weatherData

  #----------------------------------------
  # @brief ゾーンIDを設定する
  # @param zoneID_lavel ゾーンIDのラベル
  #----------------------------------------
  def SetZoneID( zoneID_lavel )
    # ラベル整形
    zoneID_lavel.strip!
    # 参照ファイルデータ取得
    file = File.open( "#{ENV["PROJECT_RSCDIR"]}fldmapdata/zonetable/zone_id.h", "r" )
    lines = file.readlines
    file.close
    # 検索
    lines.each do |line|
      line  =~ /#define\s*ZONE_ID_#{zoneID_lavel}\s*\((\s*\d+\s*)\)/  # パターンマッチ
      # 発見
      if $1 != nil then
        @zoneID_lavel = zoneID_lavel
        @zoneID       = $1.to_i
        return
      end
    end
    # ラベルが見つからず
    abort( "zoneID lavel error. (#{zoneID_lavel})" )
  end

  #----------------------------------------
  # @brief 天気データを登録する
  # @param data 登録する天気データ
  #----------------------------------------
  def AddWeatherData( data )
    lastData = @weatherData.last
    # 日付の昇順にしか登録できない
    if lastData != nil then
      if data.month < lastData.month then abort( "weather data month order error\n" )
      elsif data.month == lastData.month && data.day <= lastData.day then abort( "weather data day order error\n" ) end
    end
    # 追加
    @weatherData << data
  end

  #----------------------------------------
  # @brief 指定した日付の天気を取得する
  # @param month 月
  # @param day   日
  # @return 指定した日付の天気データ
  #----------------------------------------
  def GetWeatherData( month, day )
    # 検索
    @weatherData.each do |data|
      # 発見
      if data.month == month && data.day == day then return data end
    end
    # 登録されていない
    abort( "weather data not found #{month}/#{day}" )
  end

  #----------------------------------------
  # @brief 年間の天気データを取得する
  # @return 全ての日付の天気配列
  #----------------------------------------
  def GetAnnualWeather()
    annualData = Array.new
    @weatherData.each do |data|
      annualData << data.weather
    end
    return annualData
  end

  #----------------------------------------
  # @brief デバッグデータを出力する
  # @param destPass 出力先のパス
  #----------------------------------------
  def DebugOut( destPass )
    # ファイルを開く
    filename = "#{destPass}/debug_#{@zoneID_lavel.downcase}.txt"
    file = File.open( filename, "w" )
    # 出力 ( ゾーンID )
    file.puts( "#{@zoneID_lavel}(#{@zoneID})" )
    # 出力 ( 天気 )
    @weatherData.each do |data|
      file.puts( "#{"%2s" % data.month}/#{"%2s" % data.day} = #{data.weather_lavel}(#{data.weather})" )
    end
    # ファイルを閉じる
    file.close
  end

end


#==============================================================================
# ■メイン
#==============================================================================

#------------------------------------------------------------------------------
# 全カレンダー解析
#------------------------------------------------------------------------------
allCalenders = Array.new

# ファイル全データ取得
file = File.open( "weather_wb.txt", "r" )
lines = file.readlines
file.close

# 登録ゾーン数
zoneNum = lines[ ROW_ZONE_ID ].split(/\t/).size - COLUMN_FIRST_ZONE

# すべてのゾーンのカレンダー作成
COLUMN_FIRST_ZONE.upto( COLUMN_FIRST_ZONE + zoneNum - 1 ) do |columnIdx|

  calender = Calender.new

  # ゾーンID取得
  zoneID_lavel = lines[ ROW_ZONE_ID ].split(/\t/)[ columnIdx ]
  calender.SetZoneID( zoneID_lavel )

  # DEBUG:
  print "#{"%-8s" % zoneID_lavel} start --> "

  # すべての日付データ取得
  ROW_FIRST_DATE.upto( lines.size - 1 ) do |rowIdx|
    items = lines[ rowIdx ].split(/\t/)
    weatherData = WeatherData.new
    weatherData.SetDate( items[ COLUMN_DATE ] )
    weatherData.SetWeather( items[ columnIdx ] )
    calender.AddWeatherData( weatherData )
  end

  # DEBUG:
  puts "finish!"

  # 配列に追加
  allCalenders << calender

end

#------------------------------------------------------------------------------
# デバッグデータ出力
#------------------------------------------------------------------------------
allCalenders.each do |calender|
  # DEBUG:
  puts "output debug data: #{calender.zoneID_lavel}"
  # 出力
  calender.DebugOut("./debug")
end

#------------------------------------------------------------------------------
# インデックスデータ出力
#------------------------------------------------------------------------------
# ファイルを開く
file = File.open( "./bin/weather_index.bin", "w" )

# 出力 ( 登録ゾーン数 )
file.write( [allCalenders.size].pack("S") )

0.upto( allCalenders.size - 1 ) do |calenderIdx| 
  calender = allCalenders[ calenderIdx ]
  # ゾーン / 先頭データのインデックス
  file.write( [calender.zoneID, calenderIdx * 366].pack("SS") )
end 

# ファイルを閉じる
file.close

#------------------------------------------------------------------------------
# 天気データ出力
#------------------------------------------------------------------------------
# ファイルを開く
file = File.open( "./bin/weather_data.bin", "w" )

outData = Array.new
allCalenders.each do |calender|
  annualData = calender.GetAnnualWeather()
  outData.concat( annualData )
end 
file.write( outData.pack("C*") )

# ファイルを閉じる
file.close
