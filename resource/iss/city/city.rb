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
  puts( "error: 指定されたゾーンID[#{str_zone_id}]は定義されていません。" );
  puts( "===============================================================" );
end


#==================================================================================
# ■街ISSユニットデータ
#==================================================================================
class UnitData
  def initialize()
    @zoneID_lavel = nil        # ゾーンID ( ラベル )
    @zoneID       = 0          # ゾーンID
    @gridX        = 0          # x座標[grid]
    @gridY        = 0          # y座標[grid]
    @gridZ        = 0          # z座標[grid]
    @volume       = Array.new  # ボリューム
    @distanceX    = Array.new  # 距離x
    @distanceY    = Array.new  # 距離y
    @distanceZ    = Array.new  # 距離z
  end

  attr_accessor :zoneID_lavel, :zoneID, :gridX, :gridY, :gridZ, 
                :volume, :distanceX, :distanceY, :distanceZ

  #--------------------------------------
  # @brief バイナリデータを出力する
  # @param pass 出力先のパス
  def OutBinaryData( pass ) 
    # 出力データ作成
    outData = Array.new
    outData << @zoneID
    outData << 0  # padding
    outData << @gridX
    outData << @gridY
    outData << @gridZ
    @volume.each    do |vol|  outData << vol  end
    @distanceX.each do |dist| outData << dist end
    @distanceY.each do |dist| outData << dist end
    @distanceZ.each do |dist| outData << dist end
    # 出力
    filename = "#{pass}/iss_city_unit_#{@zoneID_lavel.downcase}.bin"
    file = File.open( filename, "wb" )
    file.write( outData.pack( "SSiiiCCCCCCCCCCCCCCCCCCCCCCCC" ) )
    file.close
  end
end


#----------------------------------------------------------------------------------
# @brief main
# @param ARGV[0] コンバート対象ファイル名(エクセルのタブ区切りデータ)
# @param ARGV[1] バイナリデータの出力先ディレクトリ
#----------------------------------------------------------------------------------

# 行インデックス
ROW_ZONE_ID    = 0  # ゾーンID
ROW_POS_X      = 1  # x座標
ROW_POS_Y      = 2  # y座標
ROW_POS_Z      = 3  # z座標
ROW_VOLUME     = 4  # ボリューム
ROW_DISTANCE_X = 5  # x距離
ROW_DISTANCE_Y = 6  # y距離
ROW_DISTANCE_Z = 7  # z距離

# 列インデックス
COLUMN_ZONE_ID        = 0  # ゾーンID
COLUMN_POS_X          = 1  # x座標
COLUMN_POS_Y          = 1  # y座標
COLUMN_POS_Z          = 1  # z座標
COLUMN_VOLUME_SPACE_0 = 6  # 音量空間データ0
COLUMN_VOLUME_SPACE_1 = 5  # 音量空間データ1
COLUMN_VOLUME_SPACE_2 = 4  # 音量空間データ2
COLUMN_VOLUME_SPACE_3 = 3  # 音量空間データ3
COLUMN_VOLUME_SPACE_4 = 2  # 音量空間データ4
COLUMN_VOLUME_SPACE_5 = 1  # 音量空間データ5


# ユニット配列
units = Array.new


#-------------------------
# ファイルデータの読み込み
#-------------------------
file = File.open( ARGV[0], "r" )
fileLines = file.readlines
file.close


#-------------------------
# 先頭行リストの洗い出し
#-------------------------
validRowList = Array.new
0.upto( fileLines.size - 1 ) do |rowIdx|
  line = fileLines[ rowIdx ]
  if line.index( "ZONE_ID_" ) != nil then
    validRowList << rowIdx
  end
end


#-------------------------
# リストの全データを取得
#-------------------------
validRowList.each do |baseRowIdx|

  unit = UnitData.new

  # ゾーンID
  line  = fileLines[ baseRowIdx + ROW_ZONE_ID ]
  items = line.split(/\t/)
  unit.zoneID_lavel = items[ COLUMN_ZONE_ID ]
  unit.zoneID       = GetZoneID( items[ COLUMN_ZONE_ID ] )

  # x座標
  line  = fileLines[ baseRowIdx + ROW_POS_X ]
  items = line.split(/\t/)
  unit.gridX = items[ COLUMN_POS_X ].to_i

  # y座標
  line  = fileLines[ baseRowIdx + ROW_POS_Y ]
  items = line.split(/\t/)
  unit.gridY = items[ COLUMN_POS_Y ].to_i

  # z座標
  line  = fileLines[ baseRowIdx + ROW_POS_Z ]
  items = line.split(/\t/)
  unit.gridZ = items[ COLUMN_POS_Z ].to_i

  # ボリューム
  line  = fileLines[ baseRowIdx + ROW_VOLUME ]
  items = line.split(/\t/) 
  unit.volume << items[ COLUMN_VOLUME_SPACE_0 ].to_i
  unit.volume << items[ COLUMN_VOLUME_SPACE_1 ].to_i
  unit.volume << items[ COLUMN_VOLUME_SPACE_2 ].to_i
  unit.volume << items[ COLUMN_VOLUME_SPACE_3 ].to_i
  unit.volume << items[ COLUMN_VOLUME_SPACE_4 ].to_i
  unit.volume << items[ COLUMN_VOLUME_SPACE_5 ].to_i

  # x距離
  line  = fileLines[ baseRowIdx + ROW_DISTANCE_X ]
  items = line.split(/\t/) 
  unit.distanceX << items[ COLUMN_VOLUME_SPACE_0 ].to_i
  unit.distanceX << items[ COLUMN_VOLUME_SPACE_1 ].to_i
  unit.distanceX << items[ COLUMN_VOLUME_SPACE_2 ].to_i
  unit.distanceX << items[ COLUMN_VOLUME_SPACE_3 ].to_i
  unit.distanceX << items[ COLUMN_VOLUME_SPACE_4 ].to_i
  unit.distanceX << items[ COLUMN_VOLUME_SPACE_5 ].to_i

  # y距離
  line  = fileLines[ baseRowIdx + ROW_DISTANCE_Y ]
  items = line.split(/\t/) 
  unit.distanceY << items[ COLUMN_VOLUME_SPACE_0 ].to_i
  unit.distanceY << items[ COLUMN_VOLUME_SPACE_1 ].to_i
  unit.distanceY << items[ COLUMN_VOLUME_SPACE_2 ].to_i
  unit.distanceY << items[ COLUMN_VOLUME_SPACE_3 ].to_i
  unit.distanceY << items[ COLUMN_VOLUME_SPACE_4 ].to_i
  unit.distanceY << items[ COLUMN_VOLUME_SPACE_5 ].to_i

  # z距離
  line  = fileLines[ baseRowIdx + ROW_DISTANCE_Z ]
  items = line.split(/\t/) 
  unit.distanceZ << items[ COLUMN_VOLUME_SPACE_0 ].to_i
  unit.distanceZ << items[ COLUMN_VOLUME_SPACE_1 ].to_i
  unit.distanceZ << items[ COLUMN_VOLUME_SPACE_2 ].to_i
  unit.distanceZ << items[ COLUMN_VOLUME_SPACE_3 ].to_i
  unit.distanceZ << items[ COLUMN_VOLUME_SPACE_4 ].to_i
  unit.distanceZ << items[ COLUMN_VOLUME_SPACE_5 ].to_i

  # 配列に追加
  units << unit
end


#-------------------------
# バイナリデータ出力
#-------------------------
units.each do |unitData|
  unitData.OutBinaryData( "./bin" )
end
