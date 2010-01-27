#---------------------------------------------------------------------------------- 
# @brief シーケンスラベルから, シーケンス番号を取得する
# @param seq_label シーケンスラベル
# @return 指定したラベルのシーケンス番号
#---------------------------------------------------------------------------------- 
def GetSeqNo( seq_label )

	# ファイルを開く
	file_def = File.open( "../../../prog/include/sound/wb_sound_data.sadl" )

	# 1行ずつ見ていく
	file_def.each do |line|
		if line.index(seq_label)!=nil then
			i0 = line.index(seq_label) + seq_label.length
			i1 = line.length - 1
			def_val = line[i0..i1].to_i
			return def_val
		end
	end
	
	# ファイルを閉じる
	file_def.close

  # ラベルが見つからず
  abort( "error: 指定シーケンスラベル[#{seq_label}]は定義されていません。" );
end

#----------------------------------------------------------------------------------
# @brief トラックビットを作成する
# @param 各トラックの開放状態を保持する配列
# @return 指定した配列から求めた開放トラックビット
#----------------------------------------------------------------------------------
def CalcTrackBit( track_state )

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
# @brief main
# @param ARGV[0] コンバート対象ファイル名(エクセルのタブ区切りデータ)
# @param ARGV[1] バイナリデータの出力先ディレクトリ
#----------------------------------------------------------------------------------

#行インデックス
LINE_FIRST_DATA = 2  # 先頭データ

# データ・インデックス
ROW_SOUND_IDX  = 0   # ゾーンID
ROW_TRACK_1    = 1   # 操作するトラックのOn/Off (トラック1)
ROW_TRACK_2    = 2   # 操作するトラックのOn/Off (トラック2)
ROW_TRACK_3    = 3   # 操作するトラックのOn/Off (トラック3)
ROW_TRACK_4    = 4   # 操作するトラックのOn/Off (トラック4)
ROW_TRACK_5    = 5   # 操作するトラックのOn/Off (トラック5)
ROW_TRACK_6    = 6   # 操作するトラックのOn/Off (トラック6)
ROW_TRACK_7    = 7   # 操作するトラックのOn/Off (トラック7)
ROW_TRACK_8    = 8   # 操作するトラックのOn/Off (トラック8)
ROW_TRACK_9    = 9   # 操作するトラックのOn/Off (トラック9)
ROW_TRACK_10   = 10  # 操作するトラックのOn/Off (トラック10)
ROW_TRACK_11   = 11  # 操作するトラックのOn/Off (トラック11)
ROW_TRACK_12   = 12  # 操作するトラックのOn/Off (トラック12)
ROW_TRACK_13   = 13  # 操作するトラックのOn/Off (トラック13)
ROW_TRACK_14   = 14  # 操作するトラックのOn/Off (トラック14)
ROW_TRACK_15   = 15  # 操作するトラックのOn/Off (トラック15)
ROW_TRACK_16   = 16  # 操作するトラックのOn/Off (トラック16)

# 出力ファイル名のリスト
bin_file_list = Array.new

# ファイルデータを読み込み
file = File.open( ARGV[0], "r" )
file_lines = file.readlines
file.close

# 3行目以降の全ラインをコンバート
LINE_FIRST_DATA.upto( file_lines.size - 1 ) do |i|
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
  out_data << GetSeqNo( in_data[ROW_SOUND_IDX] )
  out_data << CalcTrackBit( track_state )
  out_data << 0  # padding
  # バイナリデータを出力
  filename = ARGV[1] + "/iss_bridge_data_" + in_data[ROW_SOUND_IDX].downcase + ".bin"
  file = File.open( filename, "wb" )
  file.write( out_data.pack("ISS") )
  file.close
  # 出力ファイル名を記憶
  bin_file_list << filename
end

# 出力したバイナリファイル名を表示
bin_file_list.each do |filename|
  puts "-output: #{filename}"
end 
