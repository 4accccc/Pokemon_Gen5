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
# @brief 開放トラックビットを作成する
# @param 各トラックの開放状態を保持する配列
# @return 指定した配列から求めた開放トラックビット
#----------------------------------------------------------------------------------
def CalcOpenTrackBit( track_state )

  track_bit = 0

  # 全トラックの状態を見る
  0.upto(track_state.size-1) do |i|
    if track_state[i]=="○" then 
      track_bit = track_bit | (1 << i)
    end
  end
  return track_bit
end

#----------------------------------------------------------------------------------
# @brief 閉鎖トラックビットを作成する
# @param 各トラックの開放状態を保持する配列
# @return 指定した配列から求めた閉鎖トラックビット
#----------------------------------------------------------------------------------
def CalcCloseTrackBit( track_state )

  # 開放トラックビットを反転
  track_bit = CalcOpenTrackBit( track_state )
  return ~track_bit 
end

#----------------------------------------------------------------------------------
# @brief main
# @param ARGV[0] コンバート対象ファイル名(エクセルのタブ区切りデータ)
# @param ARGV[1] バイナリデータの出力先ディレクトリ
#----------------------------------------------------------------------------------

# データ・インデックス
ROW_ZONE       = 0   # ゾーンID
ROW_TRACK_1    = 1   # デフォルト音量(トラック1)
ROW_TRACK_2    = 2   # デフォルト音量(トラック2)
ROW_TRACK_3    = 3   # デフォルト音量(トラック3)
ROW_TRACK_4    = 4   # デフォルト音量(トラック4)
ROW_TRACK_5    = 5   # デフォルト音量(トラック5)
ROW_TRACK_6    = 6   # デフォルト音量(トラック6)
ROW_TRACK_7    = 7   # デフォルト音量(トラック7)
ROW_TRACK_8    = 8   # デフォルト音量(トラック8)
ROW_TRACK_9    = 9   # デフォルト音量(トラック9)
ROW_TRACK_10   = 10  # デフォルト音量(トラック10)
ROW_TRACK_11   = 11  # デフォルト音量(トラック11)
ROW_TRACK_12   = 12  # デフォルト音量(トラック12)
ROW_TRACK_13   = 13  # デフォルト音量(トラック13)
ROW_TRACK_14   = 14  # デフォルト音量(トラック14)
ROW_TRACK_15   = 15  # デフォルト音量(トラック15)
ROW_TRACK_16   = 16  # デフォルト音量(トラック16)
ROW_FADE_FRAME = 17  # フェード時間

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
  # トラック状態配列を作成
  track_state = Array.new
  track_state << in_data[ROW_TRACK_1]
  track_state << in_data[ROW_TRACK_2]
  track_state << in_data[ROW_TRACK_3]
  track_state << in_data[ROW_TRACK_4]
  track_state << in_data[ROW_TRACK_5]
  track_state << in_data[ROW_TRACK_6]
  track_state << in_data[ROW_TRACK_7]
  track_state << in_data[ROW_TRACK_8]
  track_state << in_data[ROW_TRACK_9]
  track_state << in_data[ROW_TRACK_10]
  track_state << in_data[ROW_TRACK_11]
  track_state << in_data[ROW_TRACK_12]
  track_state << in_data[ROW_TRACK_13]
  track_state << in_data[ROW_TRACK_14]
  track_state << in_data[ROW_TRACK_15]
  track_state << in_data[ROW_TRACK_16]
  # 出力データを作成
  out_data = Array.new
  out_data << GetZoneID("ZONE_ID_"+in_data[ROW_ZONE])
  out_data << CalcOpenTrackBit( track_state )
  out_data << CalcCloseTrackBit( track_state )
  out_data << in_data[ROW_FADE_FRAME].to_i
  # バイナリデータを出力
  filename = ARGV[1] + "/iss_zone_data_" + in_data[ROW_ZONE].downcase + ".bin"
  file = File.open( filename, "wb" )
  file.write( out_data.pack("SSSS") )
  file.close
  # 出力ファイル名を記憶
  bin_file_list << filename
end

# 出力したバイナリファイル名を表示
bin_file_list.each do |filename|
  puts "-output: #{filename}"
end 
