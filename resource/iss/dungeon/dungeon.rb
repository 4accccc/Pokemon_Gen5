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
# @brief キーの値をプログラムで使用する値に変換する
# @param key 変換する値
# @return 変換後の値
#----------------------------------------------------------------------------------
def ConvertKey( key )

  # キーの値は64倍する(64=1半音)
  return key * 64
end

#----------------------------------------------------------------------------------
# @brief テンポの値をプログラムで使用する値に変換する
# @param tempo 変換する値
# @return 変換後の値
#----------------------------------------------------------------------------------
def ConvertTempo( tempo )

  # 256 が等倍になるように変換
  return tempo * 256
end

#----------------------------------------------------------------------------------
# @brief リバーブの値をプログラムで使用する値に変換する
# @param reverb 変換する値
# @return 変換後の値
#----------------------------------------------------------------------------------
def ConvertReverb( reverb )

  # リバーブの値は64倍する
  return reverb * 64
end


#----------------------------------------------------------------------------------
# @brief main
# @param ARGV[0] コンバート対象ファイル名(エクセルのタブ区切りデータ)
# @param ARGV[1] バイナリデータの出力先ディレクトリ
#----------------------------------------------------------------------------------

# データ・インデックス
ROW_ZONE          = 0   # ゾーンID
ROW_KEY_SPRING    = 1   # キー(春)
ROW_KEY_SUMMER    = 2   # キー(夏)
ROW_KEY_AUTUMN    = 3   # キー(秋)
ROW_KEY_WINTER    = 4   # キー(冬)
ROW_TEMPO_SPRING  = 5   # テンポ(春)
ROW_TEMPO_SUMMER  = 6   # テンポ(夏)
ROW_TEMPO_AUTUMN  = 7   # テンポ(秋)
ROW_TEMPO_WINTER  = 8   # テンポ(冬)
ROW_REVERB_SPRING = 9   # リバーブ(春)
ROW_REVERB_SUMMER = 10  # リバーブ(夏)
ROW_REVERB_AUTUMN = 11  # リバーブ(秋)
ROW_REVERB_WINTER = 12  # リバーブ(冬)

# 出力ファイル名のリスト
bin_file_list = Array.new

# ファイルデータを読み込み
file = File.open( ARGV[0], "r" )
file_lines = file.readlines
file.close

# 3行目以降の全ラインをコンバート
2.upto( file_lines.size - 1 ) do |i|
  # コンバート対象のデータを取得
  line    = file_lines[i]
  in_data = line.split(/\s/)
  # 空データを発見==>終了
  if in_data[0]==nil then break end 
  # 出力データを作成
  out_data = Array.new
  out_data << GetZoneID("ZONE_ID_"+in_data[ROW_ZONE])  # ゾーンID
  out_data << 0   # padding
  0.upto(3) do |season|
    out_data << ConvertKey(in_data[ROW_KEY_SPRING+season].to_i)  # キー
  end
  0.upto(3) do |season|
    out_data << ConvertTempo(in_data[ROW_TEMPO_SPRING+season].to_f)  # テンポ
  end
  0.upto(3) do |season|
    out_data << ConvertReverb(in_data[ROW_REVERB_SPRING+season].to_i)  # リバーブ
  end
  # バイナリデータを出力
  filename = ARGV[1] + "/iss_dungeon_data_" + in_data[ROW_ZONE].downcase + ".bin"
  file = File.open( filename, "wb" )
  file.write( out_data.pack("SSssssSSSSSSSS") )
  file.close
  # 出力ファイル名を記憶
  bin_file_list << filename
end

# 出力したバイナリファイル名を表示
bin_file_list.each do |filename|
  puts "-output: #{filename}"
end 
