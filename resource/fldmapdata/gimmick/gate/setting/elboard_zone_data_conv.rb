######################################################################################
#
# @brief 電光掲示板データコンバータ
# @file elboard_data_conv.rb
# @author obata
# @date   2009.10.27
#
######################################################################################


#-------------------------------------------------------------------------------------
# @brief ゾーンIDを取得する
# @param zone ゾーンIDを表す文字列
# @return 指定したゾーンIDの値
#-------------------------------------------------------------------------------------
def GetZoneID( zone )

  # 定義ファイルを開く
  filename = ENV["PROJECT_RSCDIR"] + "fldmapdata/zonetable/zone_id.h"
  file = File.open( filename, "r" )

  # 指定されたゾーンIDを検索
  file.each do |line|
    if line.index(zone) != nil then
      i0 = line.index("(") + 1
      i1 = line.index(")") - 1
      val = line[i0..i1].to_i
      return val
    end
  end
  file.close

  # 指定されたゾーンIDが定義されていない場合
  abort("ゾーンID:#{zone}は定義されていません")

end

#-------------------------------------------------------------------------------------
# @brief メッセージIDを取得する
# @param gmm_filename gmmファイル名
# @param msg_id       メッセージIDを表す文字列
# @return 指定したメッセージIDの値
#-------------------------------------------------------------------------------------
def GetMsgID( gmm_filename, msg_id )

  # 定義ファイルを開く
  def_filename = 
    ENV["PROJECT_PROGDIR"] + "include/msg/msg_" + File.basename(gmm_filename, ".gmm") + ".h"
  file = File.open( def_filename, "r" )

  # 指定されたメッセージIDを検索
  file.each do |line|
    if line.index(msg_id) != nil then
      i0 = line.index("(") + 1
      i1 = line.index(")") - 1
      val = line[i0..i1].to_i
      return val
    end
  end
  file.close

  # 指定されたメッセージIDが定義されていない場合
  abort("メッセージID:#{msg_id}は定義されていません")

end

#-------------------------------------------------------------------------------------
# @brief 指定した向きに対応する値を取得する
# @param dir 向きを表す文字列(DIR_xxxx)
# @return 指定した向きを表す値
#-------------------------------------------------------------------------------------
def GetDir( dir )

  # 定義ファイルを開く
  def_filename = ENV["PROJECT_PROGDIR"] + "include/field/field_dir.h"
  file = File.open( def_filename, "r" )

  # 指定された向きを検索
  file.each do |line|
    if line.index(dir) != nil then
      i0 = line.index("(") + 1
      i1 = line.index(")") - 1
      val = line[i0..i1].to_i
      return val
    end
  end
  file.close

  # 指定された向きが定義されていない場合
  abort("向き:#{dir}は定義されていません")
end


#-------------------------------------------------------------------------------------
# @brief main 指定したファイルをコンバートする
# @param ARGV[0] コンバート対象ファイル名(エクセルのタブ区切りデータ)
# @param ARGV[1] バイナリデータの出力先ディレクトリ
#-------------------------------------------------------------------------------------
# .gmmファイル名
GMM_FILENAME = "gate.gmm"

# データインデックス
ROW_ZONE_ID            = 0   # ゾーンID
ROW_X                  = 1   # x座標
ROW_Y                  = 2   # y座標
ROW_Z                  = 3   # z座標
ROW_DIR                = 4   # 向き
ROW_MSG_ID_DATE        = 5   # 日付
ROW_MSG_ID_WEATHER     = 6   # 天気
ROW_MSG_ID_PROPAGATION = 7   # 天気
ROW_MSG_ID_INFO_A      = 8   # 情報A
ROW_MSG_ID_INFO_B      = 9   # 情報B
ROW_MSG_ID_INFO_C      = 10  # 情報C
ROW_MSG_ID_INFO_D      = 11  # 情報D
ROW_MSG_ID_INFO_E      = 12  # 情報E
ROW_MSG_ID_INFO_F      = 13  # 情報F
ROW_MSG_ID_INFO_G      = 14  # 情報G
ROW_MSG_ID_INFO_H      = 15  # 情報H
ROW_MSG_ID_INFO_I      = 16  # 情報I
ROW_MSG_ID_CM_MON      = 17  # CM月
ROW_MSG_ID_CM_TUE      = 18  # CM火
ROW_MSG_ID_CM_WED      = 19  # CM水
ROW_MSG_ID_CM_THU      = 20  # CM木
ROW_MSG_ID_CM_FRI      = 21  # CM金
ROW_MSG_ID_CM_SAT      = 22  # CM土
ROW_MSG_ID_CM_SUN      = 23  # CM日
ROW_WEATHER_ZONE_1     = 24  # 天気を表示する場所1
ROW_WEATHER_ZONE_2     = 25  # 天気を表示する場所2
ROW_WEATHER_ZONE_3     = 26  # 天気を表示する場所3
ROW_WEATHER_ZONE_4     = 27  # 天気を表示する場所4
ROW_MONITOR_ANIME_NO   = 28  # モニター・アニメーション番号
                     
# 出力ファイル名のリスト
bin_file_list = Array.new

# ファイルデータを読み込み
file = File.open( ARGV[0], "r" )
file_lines = file.readlines
file.close

# 2行目以降の全ラインをコンバート
1.upto( file_lines.size - 1 ) do |i|
  # 出力データを作成
  line = file_lines[i]
  in_data = line.split(/\s/)
  out_data = Array.new
  out_data << GetZoneID("ZONE_ID_"+in_data[ROW_ZONE_ID])
  out_data << in_data[ROW_X].to_i
  out_data << in_data[ROW_Y].to_i
  out_data << in_data[ROW_Z].to_i
  out_data << GetDir(in_data[ROW_DIR])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_DATE])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_WEATHER])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_PROPAGATION])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_INFO_A])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_INFO_B])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_INFO_C])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_INFO_D])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_INFO_E])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_INFO_F])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_INFO_G])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_INFO_H])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_INFO_I])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_CM_MON])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_CM_TUE])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_CM_WED])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_CM_THU])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_CM_FRI])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_CM_SAT])
  out_data << GetMsgID(GMM_FILENAME, in_data[ROW_MSG_ID_CM_SUN])
  out_data << GetZoneID("ZONE_ID_"+in_data[ROW_WEATHER_ZONE_1])
  out_data << GetZoneID("ZONE_ID_"+in_data[ROW_WEATHER_ZONE_2])
  out_data << GetZoneID("ZONE_ID_"+in_data[ROW_WEATHER_ZONE_3])
  out_data << GetZoneID("ZONE_ID_"+in_data[ROW_WEATHER_ZONE_4])
  out_data << in_data[ROW_MONITOR_ANIME_NO].to_i
  # バイナリデータを出力
  filename = ARGV[1] + "/elboard_zone_data_" + in_data[ROW_ZONE_ID].downcase + ".bin"
  file = File.open( filename, "wb" )
  file.write( out_data.pack("I*") )
  file.close
  # 出力ファイル名を記憶
  bin_file_list << filename
end


# 出力したバイナリファイル名を表示
bin_file_list.each do |filename|
  puts "-output: #{filename}"
end

# 出力したバイナリファイルリストを作成
str = "ELBOARD_ZONE_DATA = "
bin_file_list.each do |filename|
  str += "\\"
  str += "\n"
  str += "#{filename} "
end
file = File.open( ARGV[1] + "/elboard_zone_data.list", "w" )
file.write( str )
file.close
