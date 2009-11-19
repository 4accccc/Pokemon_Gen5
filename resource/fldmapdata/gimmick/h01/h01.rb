#################################################################################
# @brief h01.xlsコンバータ
# @file h01.rb
# @author obata
# @date 2009.11.19
#################################################################################

#--------------------------------------------------------------------------------
# @brief main
# @param ARGV[0] コンバート対象ファイルのパス
# @param ARGV[1] コンバート後の出力先のパス
#--------------------------------------------------------------------------------
# データインデックス(行)
ROW_WIND      = 1  # 風
ROW_SHIP      = 2  # 船
ROW_TRAILER_L = 3  # トレーラー(左)
ROW_TRAILER_R = 4  # トレーラー(右)
# データインデックス(列)
COLUMN_TRACK_NO        = 1  # トラックNo.
COLUMN_3DS_RANGE       = 2  # 音が届く距離
COLUMN_WIND_MIN_HEIGHT = 3  # 風音が鳴り出す高さ
COLUMN_WIND_MAX_HEIGHT = 4  # 風音が最大になる高さ
COLUMN_3DS_VOLUME      = 5  # 音量
COLUMN_MIN_WAIT        = 6  # 最短待機時間
COLUMN_MAX_WAIT        = 7  # 最長待機時間

# 入力データ読み込み
file = File.open( ARGV[0], "r" )
file_data = file.readlines
file.close

# 風データ
in_data = file_data[ROW_WIND].split(/\s/)
out_data = Array.new
out_data << (1<<(in_data[COLUMN_TRACK_NO].to_i - 1))
out_data << 0  # padding
out_data << in_data[COLUMN_WIND_MIN_HEIGHT].to_f
out_data << in_data[COLUMN_WIND_MAX_HEIGHT].to_f
file = File.open( ARGV[1]+"./wind_data.bin", "wb" )
file.write( out_data.pack("SSff") )
file.close 
# 船(3DS)データ
in_data = file_data[ROW_SHIP].split(/\s/)
out_data = Array.new
out_data << in_data[COLUMN_TRACK_NO].to_i
out_data << in_data[COLUMN_3DS_RANGE].to_f
out_data << in_data[COLUMN_3DS_VOLUME].to_i
file = File.open( ARGV[1]+"./ship_3dsu_data.bin", "wb" )
file.write( out_data.pack("ifi") )
file.close 
# 船(待ち)データ
in_data = file_data[ROW_SHIP].split(/\s/)
out_data = Array.new
out_data << in_data[COLUMN_MIN_WAIT].to_i
out_data << in_data[COLUMN_MAX_WAIT].to_i
file = File.open( ARGV[1]+"./ship_wait_data.bin", "wb" )
file.write( out_data.pack("II") )
file.close 
# 左トレーラ(3DS)データ
in_data = file_data[ROW_TRAILER_L].split(/\s/)
out_data = Array.new
out_data << in_data[COLUMN_TRACK_NO].to_i
out_data << in_data[COLUMN_3DS_RANGE].to_f
out_data << in_data[COLUMN_3DS_VOLUME].to_i
file = File.open( ARGV[1]+"./trailer1_3dsu_data.bin", "wb" )
file.write( out_data.pack("ifi") )
file.close
# 左トレーラ(待ち)データ
in_data = file_data[ROW_TRAILER_L].split(/\s/)
out_data = Array.new
out_data << in_data[COLUMN_MIN_WAIT].to_i
out_data << in_data[COLUMN_MAX_WAIT].to_i
file = File.open( ARGV[1]+"./trailer1_wait_data.bin", "wb" )
file.write( out_data.pack("II") )
file.close 
# 右トレーラ(3DS)データ
in_data = file_data[ROW_TRAILER_R].split(/\s/)
out_data = Array.new
out_data << in_data[COLUMN_TRACK_NO].to_i
out_data << in_data[COLUMN_3DS_RANGE].to_f
out_data << in_data[COLUMN_3DS_VOLUME].to_i
file = File.open( ARGV[1]+"./trailer2_3dsu_data.bin", "wb" )
file.write( out_data.pack("ifi") )
file.close
# 右トレーラ(待ち)データ
in_data = file_data[ROW_TRAILER_R].split(/\s/)
out_data = Array.new
out_data << in_data[COLUMN_MIN_WAIT].to_i
out_data << in_data[COLUMN_MAX_WAIT].to_i
file = File.open( ARGV[1]+"./trailer2_wait_data.bin", "wb" )
file.write( out_data.pack("II") )
file.close 
