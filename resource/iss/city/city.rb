#---------------------------------------------------------------------------------- 
# @brief ゾーン名から, ゾーンIDを取得する
# @param str_zone_id ゾーンIDを表す文字列
# @return 指定したゾーンIDの値
#---------------------------------------------------------------------------------- 
def GetZoneID( str_zone_id )

	# ファイルを開く
	file_def = File.open( "../../../prog/arc/fieldmap/zone_id.h" )

	# 1行ずつ見ていく
	file_def.each do |line|
		if line.index(str_zone_id)!=nil then
			i0 = line.index("(") + 1
			i1 = line.index(")") - 1
			def_val = line[i0..i1].to_i
			return def_val
		end
	end
	
	# ファイルを閉じる
	file_def.close

  puts( "===============================================================" );
  puts( "error: 指定されたゾーンID[# str_zone_id}]は定義されていません。" );
  puts( "===============================================================" );
end


#----------------------------------------------------------------------------------
# @brief main
# @param ARGV[0] コンバート対象ファイル名(エクセルのタブ区切りデータ)
# @param ARGV[1] バイナリデータの出力先ディレクトリ
#----------------------------------------------------------------------------------

# データ・インデックス
ROW_ZONE      = 0   # ゾーンID
ROW_POS_X     = 1   # x座標
ROW_POS_Y     = 2   # y座標
ROW_POS_Z     = 3   # z座標
ROW_VOLUME_1  = 4   # 音量1
ROW_VOLUME_2  = 5   # 音量2
ROW_VOLUME_3  = 6   # 音量3
ROW_VOLUME_4  = 7   # 音量4
ROW_RANGE_X_1 = 8   # x距離1
ROW_RANGE_X_2 = 9   # x距離2
ROW_RANGE_X_3 = 10  # x距離3
ROW_RANGE_X_4 = 11  # x距離4
ROW_RANGE_Y_1 = 12  # y距離1
ROW_RANGE_Y_2 = 13  # y距離2
ROW_RANGE_Y_3 = 14  # y距離3
ROW_RANGE_Y_4 = 15  # y距離4
ROW_RANGE_Z_1 = 16  # z距離1
ROW_RANGE_Z_2 = 17  # z距離2
ROW_RANGE_Z_3 = 18  # z距離3
ROW_RANGE_Z_4 = 19  # z距離4

# 出力ファイル名のリスト
bin_file_list = Array.new

# ファイルデータを読み込み
file = File.open( ARGV[0], "r" )
file_lines = file.readlines
file.close

# 2行目以降の全ラインをコンバート
1.upto( file_lines.size - 1 ) do |i|
  # コンバート対象のデータを取得
  line = file_lines[i]
  in_data = line.split(/\s/)
  if in_data[0]==nil then break end # 空データを発見==>終了
  # 出力データを作成
  out_data = Array.new
  out_data << GetZoneID("ZONE_ID_"+in_data[ROW_ZONE])
  out_data << 0  # padding
  out_data << in_data[ROW_POS_X].to_i
  out_data << in_data[ROW_POS_Y].to_i
  out_data << in_data[ROW_POS_Z].to_i
  out_data << in_data[ROW_VOLUME_1].to_i
  out_data << in_data[ROW_VOLUME_2].to_i
  out_data << in_data[ROW_VOLUME_3].to_i
  out_data << in_data[ROW_VOLUME_4].to_i
  out_data << in_data[ROW_RANGE_X_1].to_i
  out_data << in_data[ROW_RANGE_X_2].to_i
  out_data << in_data[ROW_RANGE_X_3].to_i
  out_data << in_data[ROW_RANGE_X_4].to_i
  out_data << in_data[ROW_RANGE_Y_1].to_i
  out_data << in_data[ROW_RANGE_Y_2].to_i
  out_data << in_data[ROW_RANGE_Y_3].to_i
  out_data << in_data[ROW_RANGE_Y_4].to_i
  out_data << in_data[ROW_RANGE_Z_1].to_i
  out_data << in_data[ROW_RANGE_Z_2].to_i
  out_data << in_data[ROW_RANGE_Z_3].to_i
  out_data << in_data[ROW_RANGE_Z_4].to_i
  # バイナリデータを出力
  filename = ARGV[1] + "/iss_city_unit_" + in_data[ROW_ZONE].downcase + ".bin"
  file = File.open( filename, "wb" )
  file.write( out_data.pack("SSiiiCCCCCCCCCCCCCCCC") )
  file.close
  # 出力ファイル名を記憶
  bin_file_list << filename
end

# 出力したバイナリファイル名を表示
bin_file_list.each do |filename|
  puts "-output: #{filename}"
end

# 出力したバイナリファイルリストを作成
str = "ISS_C_UNIT_DATA = "
bin_file_list.each do |filename|
  str += "\\"
  str += "\n"
  str += "#{filename} "
end
file = File.open( ARGV[1] + "/iss_city_unit.list", "w" )
file.write( str )
file.close 
