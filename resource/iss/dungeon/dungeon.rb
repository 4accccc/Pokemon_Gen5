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

  # テンポの値域を [-512, 512] に補正
  if tempo < -512 then tempo = -512 end
  if 512 < tempo then tempo = 512 end

  # テンポの値域 [-512, 512] を [0, 512] に変換する
  tempo = ( tempo + 512 ) / 2 

  return tempo
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
ROW_ZONE   = 0  # ゾーンID
ROW_KEY    = 1  # キー
ROW_TEMPO  = 2  # テンポ
ROW_REVERB = 3  # リバーブ

# 出力ファイル名のリスト
bin_file_list = Array.new

# ユニット登録データ
entry_data = Array.new

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
  out_data << ConvertKey(in_data[ROW_KEY].to_i)
  out_data << ConvertTempo(in_data[ROW_TEMPO].to_i)
  out_data << ConvertReverb(in_data[ROW_REVERB].to_i)
  # バイナリデータを出力
  filename = ARGV[1] + "/iss_dungeon_data_" + in_data[ROW_ZONE].downcase + ".bin"
  file = File.open( filename, "wb" )
  file.write( out_data.pack("SsSS") )
  file.close
  # 出力ファイル名を記憶
  bin_file_list << filename
  # ユニット登録データを記憶
  entry_data << "ZONE_ID_"+in_data[ROW_ZONE]
end

# 出力したバイナリファイル名を表示
bin_file_list.each do |filename|
  puts "-output: #{filename}"
end

# 出力したバイナリファイルリストを作成
str = "ISS_D_DATA = "
bin_file_list.each do |filename|
  str += "\\"
  str += "\n"
  str += "#{filename} "
end
file = File.open( ARGV[1] + "/iss_dungeon_data.list", "w" )
file.write( str )
file.close

# ユニット登録テーブルを出力
filename = "entry_table.cdat"
file = File.open( filename, "w")
file.puts("typedef struct")
file.puts("{")
file.puts("  u16 zoneID;")
file.puts("  ARCDATID datID;")
file.puts("} ENTRY_DATA;")
file.puts
file.puts("static const ENTRY_DATA entry_table[] =") 
file.puts("{")
0.upto(entry_data.size-1) do |i|
  zone_id = entry_data[i]
  dat_id = i
  file.puts("  { #{zone_id}, #{dat_id} },")
end
file.puts("};")
file.close
