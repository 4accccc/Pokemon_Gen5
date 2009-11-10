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
ROW_DEFVOL_1  = 1   # デフォルト音量(トラック1)
ROW_DEFVOL_2  = 2   # デフォルト音量(トラック2)
ROW_DEFVOL_3  = 3   # デフォルト音量(トラック3)
ROW_DEFVOL_4  = 4   # デフォルト音量(トラック4)
ROW_DEFVOL_5  = 5   # デフォルト音量(トラック5)
ROW_DEFVOL_6  = 6   # デフォルト音量(トラック6)
ROW_DEFVOL_7  = 7   # デフォルト音量(トラック7)
ROW_DEFVOL_8  = 8   # デフォルト音量(トラック8)
ROW_DEFVOL_9  = 9   # デフォルト音量(トラック9)
ROW_DEFVOL_10 = 10  # デフォルト音量(トラック10)
ROW_DEFVOL_11 = 11  # デフォルト音量(トラック11)
ROW_DEFVOL_12 = 12  # デフォルト音量(トラック12)
ROW_DEFVOL_13 = 13  # デフォルト音量(トラック13)
ROW_DEFVOL_14 = 14  # デフォルト音量(トラック14)
ROW_DEFVOL_15 = 15  # デフォルト音量(トラック15)
ROW_DEFVOL_16 = 16  # デフォルト音量(トラック16)

# 出力ファイル名のリスト
bin_file_list = Array.new

# ファイルデータを読み込み
file = File.open( ARGV[0], "r" )
file_lines = file.readlines
file.close

# 3行目以降の全ラインをコンバート
2.upto( file_lines.size - 1 ) do |i|
  # コンバート対象のデータを取得
  line = file_lines[i]
  in_data = line.split(/\s/)
  if in_data[0]==nil then break end # 空データを発見==>終了
  # 出力データを作成
  out_data = Array.new
  out_data << GetZoneID("ZONE_ID_"+in_data[ROW_ZONE])
  out_data << 0  # padding
  out_data << in_data[ROW_DEFVOL_1].to_i
  out_data << in_data[ROW_DEFVOL_2].to_i
  out_data << in_data[ROW_DEFVOL_3].to_i
  out_data << in_data[ROW_DEFVOL_4].to_i
  out_data << in_data[ROW_DEFVOL_5].to_i
  out_data << in_data[ROW_DEFVOL_6].to_i
  out_data << in_data[ROW_DEFVOL_7].to_i
  out_data << in_data[ROW_DEFVOL_8].to_i
  out_data << in_data[ROW_DEFVOL_9].to_i
  out_data << in_data[ROW_DEFVOL_10].to_i
  out_data << in_data[ROW_DEFVOL_11].to_i
  out_data << in_data[ROW_DEFVOL_12].to_i
  out_data << in_data[ROW_DEFVOL_13].to_i
  out_data << in_data[ROW_DEFVOL_14].to_i
  out_data << in_data[ROW_DEFVOL_15].to_i
  out_data << in_data[ROW_DEFVOL_16].to_i
  # バイナリデータを出力
  filename = ARGV[1] + "/iss_zone_data_" + in_data[ROW_ZONE].downcase + ".bin"
  file = File.open( filename, "wb" )
  file.write( out_data.pack("SSCCCCCCCCCCCCCCCC") )
  file.close
  # 出力ファイル名を記憶
  bin_file_list << filename
end

# 出力したバイナリファイル名を表示
bin_file_list.each do |filename|
  puts "-output: #{filename}"
end

# 出力したバイナリファイルリストを作成
str = "ISS_Z_DATA = "
bin_file_list.each do |filename|
  str += "\\"
  str += "\n"
  str += "#{filename} "
end
file = File.open( ARGV[1] + "/iss_zone_data.list", "w" )
file.write( str )
file.close
