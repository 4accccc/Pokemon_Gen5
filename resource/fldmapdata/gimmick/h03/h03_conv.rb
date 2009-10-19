

# データのインデックス
LINE_TRAIN_1 = 1  # 電車1
LINE_TRAIN_2 = 2  # 電車2

ROW_TRACK    = 1  # トラックNo.
ROW_DISTANCE = 2  # 音が届く距離
ROW_VOLUME   = 5  # 音量


# ファイル読み込み
file = File.open( "h03.txt", "r" )
file_lines = file.readlines
file.close

# データ取得(電車1)
line = file_lines[LINE_TRAIN_1]
item = line.split(/\s/)
train1_track    = item[ROW_TRACK].to_i
train1_distance = item[ROW_DISTANCE].to_f
train1_volume   = item[ROW_VOLUME].to_i

# データ取得(電車2)
line = file_lines[LINE_TRAIN_2]
item = line.split(/\s/)
train2_track    = item[ROW_TRACK].to_i
train2_distance = item[ROW_DISTANCE].to_f
train2_volume   = item[ROW_VOLUME].to_i

# データ出力(電車1)
file = File.open( "train1_3ds_unit_data.bin", "wb" )
data = Array.new
data << train1_track
data << train1_distance
data << train1_volume
file.write( data.pack("IfI") )
file.close

# データ出力(電車2)
file = File.open( "train2_3ds_unit_data.bin", "wb" )
data = Array.new
data << train2_track
data << train2_distance
data << train2_volume
file.write( data.pack("IfI") )
file.close


