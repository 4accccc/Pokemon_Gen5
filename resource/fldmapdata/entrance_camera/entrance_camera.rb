#########################################################################################
# @brief  出入り口進入時のカメラ演出データ コンバータ
# @file   entrance_camera.rb
# @author obata
# @date   2010.01.19
#########################################################################################

# 参照ファイル名
EXIT_TYPE_DEF_FILE = "./eventdata_type.h"

# 列インデックス
COLUMN_EXIT_TYPE      =  0  # 出入り口タイプ
COLUMN_PITCH          =  1  # ピッチ
COLUMN_YAW            =  2  # ヨー
COLUMN_LENGTH         =  3  # 距離
COLUMN_TARGET_MODE    =  4  # ターゲット座標指定モード
COLUMN_TARGET_X       =  5  # ターゲット座標 x
COLUMN_TARGET_Y       =  6  # ターゲット座標 x
COLUMN_TARGET_Z       =  7  # ターゲット座標 x
COLUMN_OFFSET_X       =  8  # ターゲットオフセット x
COLUMN_OFFSET_Y       =  9  # ターゲットオフセット y
COLUMN_OFFSET_Z       = 10  # ターゲットオフセット z
COLUMN_FRAME          = 11  # フレーム数
COLUMN_VALID_FLAG_IN  = 12  # 動作条件( IN )
COLUMN_VALID_FLAG_OUT = 13  # 動作条件( OUT )
COLUMN_TARGET_BIND    = 14  # 動作設定( TargetBind )
COLUMN_CAMERA_AREA    = 15  # 動作設定( CameraArea )

# 行インデックス
ROW_HEAD_DATA = 2  # 先頭データ


#========================================================================================
# カメラ演出データ
#========================================================================================
class CameraData 
  def initialize
    @exitType_label   = nil # 出入り口タイプ(ラベル)
    @exitType         = 0   # 出入り口タイプ(値)
    @pitch            = 0   # ピッチ
    @yaw              = 0   # ヨー
    @length           = 0   # 距離
    @targetMode_label = nil # ターゲット座標指定モード ( ラベル )
    @targetMode       = 0   # ターゲット座標指定モード ( 値 )
    @targetX          = 0   # ターゲット座標 x
    @targetY          = 0   # ターゲット座標 y
    @targetZ          = 0   # ターゲット座標 z
    @offsetX          = 0   # ターゲットオフセットx
    @offsetY          = 0   # ターゲットオフセットy
    @offsetZ          = 0   # ターゲットオフセットz
    @frame            = 0   # フレーム数
    @validFlag_IN     = 0   # 進入時に有効なデータかどうか
    @validFlag_OUT    = 0   # 退出時に有効なデータかどうか
    @targetBind_label = nil # 動作設定 ( TargetBind・ラベル )
    @targetBind       = 0   # 動作設定 ( TargetBind・値 )
    @cameraArea_label = nil # 動作設定 ( CameraArea・ラベル )
    @cameraArea       = 0   # 動作設定 ( CameraArea・値 )
  end

  # アクセッサ
  attr_accessor :exitType_label, :exitType, 
                :pitch, :yaw, :length, 
                :targetMode_label, :targetMode, :targetX, :targetY, :targetZ,
                :offsetX, :offsetY, :offsetZ, :frame,
                :validFlag_IN, :validFlag_OUT,
                :targetBind_label, :targetBind, :cameraArea_label, :cameraArea

  #-----------------------------------------
  # @brief バイナリデータを出力する
  # @param pass 出力先ディレクトリへのパス
  def OutputBinaryData( pass )
    # 出力
    filename = "#{pass}/#{GetFileName()}.bin"
    file = File.open( filename, "wb" )
    file.write( [@exitType].pack("I") )
    file.write( [@pitch].pack("I") )
    file.write( [@yaw].pack("I") )
    file.write( [@length].pack("I") )
    file.write( [@targetMode].pack("I") )
    file.write( [@targetX].pack("I") )
    file.write( [@targetY].pack("I") )
    file.write( [@targetZ].pack("I") )
    file.write( [@offsetX].pack("I") )
    file.write( [@offsetY].pack("I") )
    file.write( [@offsetZ].pack("I") )
    file.write( [@frame].pack("I") )
    file.write( [@validFlag_IN].pack("I") )
    file.write( [@validFlag_OUT].pack("I") )
    file.write( [@targetBind].pack("I") )
    file.write( [@cameraArea].pack("I") )
    file.close()
  end 
  #-----------------------------------------
  # @brief デバッグデータを出力する
  # @param pass 出力先ディレクトリへのパス
  def OutputDebugData( pass )
    # 出力
    filename = "#{pass}/#{GetFileName()}.txt"
    file = File.open( filename, "wb" )
    file.puts( "exitType      = #{@exitType}(#{@exitType_label})" )
    file.puts( "pitch         = 0x#{@pitch.to_s(16)}" )
    file.puts( "yaw           = 0x#{@yaw.to_s(16)}" )
    file.puts( "length        = 0x#{@length.to_s(16)}" )
    file.puts( "targetMode    = #{@targetMode}(#{@targetMode_label})" )
    file.puts( "targetX       = 0x#{@targetX.to_s(16)}" )
    file.puts( "targetY       = 0x#{@targetY.to_s(16)}" )
    file.puts( "targetZ       = 0x#{@targetZ.to_s(16)}" )
    file.puts( "offsetX       = 0x#{@offsetX.to_s(16)}" )
    file.puts( "offsetY       = 0x#{@offsetY.to_s(16)}" )
    file.puts( "offsetZ       = 0x#{@offsetZ.to_s(16)}" )
    file.puts( "frame         = #{@frame}" )
    file.puts( "validFlag_IN  = #{@validFlag_IN}" )
    file.puts( "validflag_OUT = #{@validFlag_OUT}" )
    file.puts( "targetBind    = #{@targetBind}(#{@targetBind_label})" )
    file.puts( "cameraArea    = #{@cameraArea}(#{@cameraArea_label})" )
    file.close()
  end
  #-----------------------------------------
  # @brief ファイル名を取得する
  # @return ファイル名
  def GetFileName() 
    type = @exitType_label
    # ラベル名変換ハッシュを作成
    exitTypeHash = Hash.new
    exitTypeHash[ "EXIT_TYPE_SP1" ] = "EXIT_TYPE_SP01"
    exitTypeHash[ "EXIT_TYPE_SP2" ] = "EXIT_TYPE_SP02"
    exitTypeHash[ "EXIT_TYPE_SP3" ] = "EXIT_TYPE_SP03"
    exitTypeHash[ "EXIT_TYPE_SP4" ] = "EXIT_TYPE_SP04"
    exitTypeHash[ "EXIT_TYPE_SP5" ] = "EXIT_TYPE_SP05"
    exitTypeHash[ "EXIT_TYPE_SP6" ] = "EXIT_TYPE_SP06"
    exitTypeHash[ "EXIT_TYPE_SP7" ] = "EXIT_TYPE_SP07"
    exitTypeHash[ "EXIT_TYPE_SP8" ] = "EXIT_TYPE_SP08"
    exitTypeHash[ "EXIT_TYPE_SP9" ] = "EXIT_TYPE_SP09"
    # ラベル名を変換
    if exitTypeHash.has_key?( @exitType_label ) == true then
      type = exitTypeHash[ type ]
    end
    # ファイル名を作成
    filename = "#{type.downcase}"
    return filename
  end
end


#========================================================================================
# @brief 指定したラベルが存在する行数を取得する
# @param filename 検索対象のファイル名
# @param label    検索するラベル名
# @return 指定したラベルが最初に出現する行数
#         存在しない場合 nil
#========================================================================================
def GetFirstLineIndex( filename, label )
  # データ取得
  file = File.open( filename, "r" )
  fileData = file.readlines
  file.close

  # 検索
  lineIndex = 0
  fileData.each do |lineData|
    if lineData.index(label)!=nil then return lineIndex end
    lineIndex = lineIndex + 1
  end

  # 発見できず
  return nil
end

#========================================================================================
# @brief 出入り口タイプの値を取得する
# @param label 出入り口タイプのラベル( EXIT_TYPE_xxxx )
# @return 指定したラベルに対応する値
#========================================================================================
def GetExitTypeValue( label )
  baseLineIndex   = GetFirstLineIndex( EXIT_TYPE_DEF_FILE,  "EXIT_TYPE_NONE" )  # 先頭の定義位置
  targetLineIndex = GetFirstLineIndex( EXIT_TYPE_DEF_FILE,  label )             # 検索対象ラベルの定義位置

  # エラー処理
  if targetLineIndex == nil then 
    abort( "出入り口タイプ #{label} は定義されていません。" )
  end

  # 先頭からのオフセットで値を推測する
  return targetLineIndex - baseLineIndex
end

#========================================================================================
# @brief ターゲット指定モードの値を取得する
# @param label ターゲット指定モードのラベル
# @return 指定したラベルに対応する値
#========================================================================================
def GetTargetModeValue( label )
  modeHash = Hash.new
  modeHash[ "auto"   ] = 0
  modeHash[ "manual" ] = 1

  # ラベル名エラー
  if modeHash.has_key?( label ) == false then 
    abort( "ターゲット座標Mode に誤りがあります。" ) 
  end

  # ラベルに対応する値を返す
  return modeHash[ label ]
end

#========================================================================================
# @brief on/offの値を取得する
# @param label ラベル ( on or off )
# @return 指定したラベルに対応する値
#========================================================================================
def GetOnOffValue( label )
  hash = Hash.new
  hash[ "on"  ] = 0
  hash[ "off" ] = 1

  # ラベル名エラー
  if hash.has_key?( label ) == false then 
    abort( "on/off 指定に誤りがあります。" ) 
  end

  # ラベルに対応する値を返す
  return hash[ label ]
end

#========================================================================================
# @brief 文字列を数値に変換する
# @param string 整数値を表す文字列(10進文字列or16進文字列)
# @return 指定した文字列を変換した値
#========================================================================================
def ConvertToNumber( string )
  if string=~/0x(?:\d|[a-f])+/ then return string.hex  # 16進
  else return string.to_i end  # 10進
end


#========================================================================================
# メイン
#
# @param ARGV[0] コンバート対象のタブ区切りテキストデータ(パス指定)
# @param ARGV[1] コンバート後のバイナリデータの出力先
#======================================================================================== 
# カメラ演出データ配列
cameraDataArray = Array.new

# データ取得
file = File.open( ARGV[0], "r" )
fileData = file.readlines
file.close

# 値を取得
ROW_HEAD_DATA.upto( fileData.size - 1 ) do |rowIdx|
  rowData = fileData[ rowIdx ]
  rowItem = rowData.split(/\s/)
  # 取得
  cameraData = CameraData.new
  cameraData.exitType_label   = rowItem[ COLUMN_EXIT_TYPE ]
  cameraData.exitType         = GetExitTypeValue( rowItem[ COLUMN_EXIT_TYPE ] )
  cameraData.pitch            = ConvertToNumber( "0x" + rowItem[ COLUMN_PITCH ] )
  cameraData.yaw              = ConvertToNumber( "0x" + rowItem[ COLUMN_YAW ] )
  cameraData.length           = ConvertToNumber( "0x" + rowItem[ COLUMN_LENGTH ] )
  cameraData.targetMode_label = rowItem[ COLUMN_TARGET_MODE ]
  cameraData.targetMode       = GetTargetModeValue( rowItem[ COLUMN_TARGET_MODE ] )
  cameraData.targetX          = ConvertToNumber( "0x" + rowItem[ COLUMN_TARGET_X ] )
  cameraData.targetY          = ConvertToNumber( "0x" + rowItem[ COLUMN_TARGET_Y ] )
  cameraData.targetZ          = ConvertToNumber( "0x" + rowItem[ COLUMN_TARGET_Z ] )
  cameraData.offsetX          = ConvertToNumber( "0x" + rowItem[ COLUMN_OFFSET_X ] )
  cameraData.offsetY          = ConvertToNumber( "0x" + rowItem[ COLUMN_OFFSET_Y ] )
  cameraData.offsetZ          = ConvertToNumber( "0x" + rowItem[ COLUMN_OFFSET_Z ] )
  cameraData.frame            = ConvertToNumber( rowItem[ COLUMN_FRAME ] )
  if rowItem[ COLUMN_VALID_FLAG_IN ].include?( "○" ) then cameraData.validFlag_IN = 1 end
  if rowItem[ COLUMN_VALID_FLAG_OUT ].include?( "○" ) then cameraData.validFlag_OUT = 1 end
  cameraData.targetBind_label = rowItem[ COLUMN_TARGET_BIND ]
  cameraData.cameraArea_label = rowItem[ COLUMN_CAMERA_AREA ]
  cameraData.targetBind       = GetOnOffValue( rowItem[ COLUMN_TARGET_BIND ] )
  cameraData.cameraArea       = GetOnOffValue( rowItem[ COLUMN_CAMERA_AREA ] )
  # 配列に登録
  cameraDataArray << cameraData
end

# デバッグ出力
cameraDataArray.each do |cameraData|
  cameraData.OutputDebugData( ARGV[1] )
end

# バイナリ出力
cameraDataArray.each do |cameraData|
  cameraData.OutputBinaryData( ARGV[1] )
end
