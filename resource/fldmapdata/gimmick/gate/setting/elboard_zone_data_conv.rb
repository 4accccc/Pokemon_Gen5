######################################################################################
#
# @brief 電光掲示板データコンバータ
# @file elboard_data_conv.rb
# @author obata
# @date   2009.10.27
#
######################################################################################
require "conv_tool"

#-------------------------------------------------------------------------------------
# @brief 指定した文字列に対応するモニターのアニメ番号を取得する
# @param no_str アニメ番号を文字列で指定
# @return 指定した文字列に対応するモニターアニメ番号
#-------------------------------------------------------------------------------------
def GetMonitorAnimeNo( no_str )

  # ハッシュテーブル作成
  hash_table = Hash::new
  hash_table["T02"]   = 0
  hash_table["C01"]   = 1
  hash_table["C02"]   = 2
  hash_table["C03"]   = 3
  hash_table["C04"]   = 4
  hash_table["C05"]   = 5
  hash_table["C06"]   = 6
  hash_table["C07"]   = 7
  hash_table["C08_W"] = 8
  hash_table["C08_B"] = 9
  hash_table["TG"]    = 10
  hash_table["ST"]    = 11
  hash_table["WF"]    = 12
  hash_table["BC"]    = 13
  hash_table["D03"]   = 14
  hash_table["D08"]   = 15
  hash_table["D10"]   = 16
  hash_table["D12"]   = 17
  hash_table["D13"]   = 18

  # 存在しないキーが指定されたらコンバートを止める
  if hash_table.has_key?(no_str) != true then
    abort("モニターのアニメ番号[#{no_str}]は定義されていません")
  end

  # 指定されたキーに対応する値を返す
  return hash_table[no_str]
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
  out_data << GetMonitorAnimeNo(in_data[ROW_MONITOR_ANIME_NO])
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
