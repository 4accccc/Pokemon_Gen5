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
# @brief 指定したフラグ番号のトラックビットを取得する
# @param flag_state 各トラックのフラグ番号を保持する配列
# @param flag_no    トラックビットを求めるフラグ番号
# @return 指定したフラグ番号を操作するためのトラックビット
#----------------------------------------------------------------------------------
def CalcTrackBit( flag_state, flag_no )

  track_bit = 0

  # 全トラックの状態を見る
  0.upto(flag_state.size-1) do |i|
    if flag_state[i]==flag_no then 
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

# データ・インデックス
ROW_SEQ_LABEL    = 0   # シーケンスラベル
ROW_FLAG_NO_TR01 = 1   # フラグ番号(トラック1)
ROW_FLAG_NO_TR02 = 2   # フラグ番号(トラック2)
ROW_FLAG_NO_TR03 = 3   # フラグ番号(トラック3)
ROW_FLAG_NO_TR04 = 4   # フラグ番号(トラック4)
ROW_FLAG_NO_TR05 = 5   # フラグ番号(トラック5)
ROW_FLAG_NO_TR06 = 6   # フラグ番号(トラック6)
ROW_FLAG_NO_TR07 = 7   # フラグ番号(トラック7)
ROW_FLAG_NO_TR08 = 8   # フラグ番号(トラック8)
ROW_FLAG_NO_TR09 = 9   # フラグ番号(トラック9)
ROW_FLAG_NO_TR10 = 10  # フラグ番号(トラック10)
ROW_FLAG_NO_TR11 = 11  # フラグ番号(トラック11)
ROW_FLAG_NO_TR12 = 12  # フラグ番号(トラック12)
ROW_FLAG_NO_TR13 = 13  # フラグ番号(トラック13)
ROW_FLAG_NO_TR14 = 14  # フラグ番号(トラック14)
ROW_FLAG_NO_TR15 = 15  # フラグ番号(トラック15)
ROW_FLAG_NO_TR16 = 16  # フラグ番号(トラック16)
ROW_FADE_FRAME   = 17  # フェード時間

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
  # フラグ番号配列を作成
  flag_state = Array.new
  flag_state << in_data[ROW_FLAG_NO_TR01].to_i
  flag_state << in_data[ROW_FLAG_NO_TR02].to_i
  flag_state << in_data[ROW_FLAG_NO_TR03].to_i
  flag_state << in_data[ROW_FLAG_NO_TR04].to_i
  flag_state << in_data[ROW_FLAG_NO_TR05].to_i
  flag_state << in_data[ROW_FLAG_NO_TR06].to_i
  flag_state << in_data[ROW_FLAG_NO_TR07].to_i
  flag_state << in_data[ROW_FLAG_NO_TR08].to_i
  flag_state << in_data[ROW_FLAG_NO_TR09].to_i
  flag_state << in_data[ROW_FLAG_NO_TR10].to_i
  flag_state << in_data[ROW_FLAG_NO_TR11].to_i
  flag_state << in_data[ROW_FLAG_NO_TR12].to_i
  flag_state << in_data[ROW_FLAG_NO_TR13].to_i
  flag_state << in_data[ROW_FLAG_NO_TR14].to_i
  flag_state << in_data[ROW_FLAG_NO_TR15].to_i
  flag_state << in_data[ROW_FLAG_NO_TR16].to_i
  # 出力データを作成
  out_data = Array.new
  out_data << GetSeqNo(in_data[ROW_SEQ_LABEL])
  out_data << 0  # padding
  out_data << CalcTrackBit(flag_state, 0)
  out_data << CalcTrackBit(flag_state, 1)
  out_data << CalcTrackBit(flag_state, 2)
  out_data << CalcTrackBit(flag_state, 3)
  out_data << CalcTrackBit(flag_state, 4)
  out_data << CalcTrackBit(flag_state, 5)
  out_data << CalcTrackBit(flag_state, 6)
  out_data << CalcTrackBit(flag_state, 7)
  out_data << CalcTrackBit(flag_state, 8)
  out_data << 0  # padding
  out_data << in_data[ROW_FADE_FRAME].to_i
  # バイナリデータを出力
  filename = ARGV[1] + "/iss_switch_data_" + in_data[ROW_SEQ_LABEL].downcase + ".bin"
  file = File.open( filename, "wb" )
  file.write( out_data.pack("SSSSSSSSSSSSS") )
  file.close
  # 出力ファイル名を記憶
  bin_file_list << filename
end

# 出力したバイナリファイル名を表示
bin_file_list.each do |filename|
  puts "-output: #{filename}"
end 
