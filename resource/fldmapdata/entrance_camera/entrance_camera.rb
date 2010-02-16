#########################################################################################
# @brief  出入り口進入時のカメラ演出データ コンバータ
# @file   entrance_camera.rb
# @author obata
# @date   2010.01.19
#########################################################################################

# 参照ファイル名
EXIT_TYPE_DEF_FILE = "./eventdata_type.h"

# 列インデックス
COLUMN_EXIT_TYPE      = 0  # 出入り口タイプ
COLUMN_PITCH          = 1  # ピッチ
COLUMN_YAW            = 2  # ヨー
COLUMN_LENGTH         = 3  # 距離
COLUMN_OFFSET_X       = 4  # ターゲットオフセットx
COLUMN_OFFSET_Y       = 5  # ターゲットオフセットy
COLUMN_OFFSET_Z       = 6  # ターゲットオフセットz
COLUMN_FRAME          = 7  # フレーム数
COLUMN_VALID_FLAG_IN  = 8  # 動作条件( IN )
COLUMN_VALID_FLAG_OUT = 9  # 動作条件( OUT )

# 行インデックス
ROW_HEAD_DATA = 2  # 先頭データ


#========================================================================================
# カメラ演出データ
#========================================================================================
class CameraData 
  def initialize
    @exitType_lavel = nil # 出入り口タイプ(ラベル)
    @exitType       = 0   # 出入り口タイプ(値)
    @pitch          = 0   # ピッチ
    @yaw            = 0   # ヨー
    @length         = 0   # 距離
    @offsetX        = 0   # ターゲットオフセットx
    @offsetY        = 0   # ターゲットオフセットy
    @offsetZ        = 0   # ターゲットオフセットz
    @frame          = 0   # フレーム数
    @validFlag_IN   = 0   # 進入時に有効なデータかどうか
    @validFlag_OUT  = 0   # 退出時に有効なデータかどうか
  end

  # アクセッサ
  attr_accessor :exitType_lavel, :exitType, 
                :pitch, :yaw, :length, :offsetX, :offsetY, :offsetZ, :frame,
                :validFlag_IN, :validFlag_OUT

  #-----------------------------------------
  # @brief バイナリデータを出力する
  # @param pass 出力先ディレクトリへのパス
  def OutputBinaryData( pass )
    # 出力
    filename = "#{pass}/#{@exitType_lavel.downcase}.bin"
    file = File.open( filename, "wb" )
    file.write( [@exitType].pack("I") )
    file.write( [@pitch].pack("I") )
    file.write( [@yaw].pack("I") )
    file.write( [@length].pack("I") )
    file.write( [@offsetX].pack("I") )
    file.write( [@offsetY].pack("I") )
    file.write( [@offsetZ].pack("I") )
    file.write( [@frame].pack("I") )
    file.write( [@validFlag_IN].pack("C") )
    file.write( [@validFlag_OUT].pack("C") )
    file.close()
  end 
  #-----------------------------------------
  # @brief デバッグデータを出力する
  # @param pass 出力先ディレクトリへのパス
  def OutputDebugData( pass )
    # 出力
    filename = "#{pass}/#{@exitType_lavel.downcase}.txt"
    file = File.open( filename, "wb" )
    file.puts( "exitType      = #{@exitType}(#{@exitType_lavel})" )
    file.puts( "pitch         = #{@pitch}" )
    file.puts( "yaw           = #{@yaw}" )
    file.puts( "length        = #{@length}" )
    file.puts( "offsetX       = #{@offsetX}" )
    file.puts( "offsetY       = #{@offsetY}" )
    file.puts( "offsetZ       = #{@offsetZ}" )
    file.puts( "frame         = #{@frame}" )
    file.puts( "validFlag_IN  = #{@validFlag_IN}" )
    file.puts( "validflag_OUT = #{@validFlag_OUT}" )
    file.close()
  end
end


#========================================================================================
# @brief 指定したラベルが存在する行数を取得する
# @param filename 検索対象のファイル名
# @param lavel    検索するラベル名
# @return 指定したラベルが最初に出現する行数
#         存在しない場合 nil
#========================================================================================
def GetFirstLineIndex( filename, lavel )
  # データ取得
  file = File.open( filename, "r" )
  fileData = file.readlines
  file.close

  # 検索
  lineIndex = 0
  fileData.each do |lineData|
    if lineData.index(lavel)!=nil then return lineIndex end
    lineIndex = lineIndex + 1
  end

  # 発見できず
  return nil
end

#========================================================================================
# @brief 出入り口タイプの値を取得する
# @param lavel 出入り口タイプのラベル( EXIT_TYPE_xxxx )
# @return 指定したラベルに対応する値
#========================================================================================
def GetExitTypeValue( lavel )
  baseLineIndex   = GetFirstLineIndex( EXIT_TYPE_DEF_FILE,  "EXIT_TYPE_NONE" )  # 先頭の定義位置
  targetLineIndex = GetFirstLineIndex( EXIT_TYPE_DEF_FILE,  lavel )             # 検索対象ラベルの定義位置

  # エラー処理
  if targetLineIndex == nil then 
    abort( "出入り口タイプ #{lavel} は定義されていません。" )
  end

  # 先頭からのオフセットで値を推測する
  return targetLineIndex - baseLineIndex
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
  cameraData.exitType_lavel = rowItem[ COLUMN_EXIT_TYPE ]
  cameraData.exitType       = GetExitTypeValue( rowItem[ COLUMN_EXIT_TYPE ] )
  cameraData.pitch          = ConvertToNumber( "0x" + rowItem[ COLUMN_PITCH ] )
  cameraData.yaw            = ConvertToNumber( "0x" + rowItem[ COLUMN_YAW ] )
  puts cameraData.yaw
  cameraData.length         = ConvertToNumber( "0x" + rowItem[ COLUMN_LENGTH ] )
  cameraData.offsetX        = ConvertToNumber( "0x" + rowItem[ COLUMN_OFFSET_X ] )
  cameraData.offsetY        = ConvertToNumber( "0x" + rowItem[ COLUMN_OFFSET_Y ] )
  cameraData.offsetZ        = ConvertToNumber( "0x" + rowItem[ COLUMN_OFFSET_Z ] )
  cameraData.frame          = ConvertToNumber( rowItem[ COLUMN_FRAME ] )
  if rowItem[ COLUMN_VALID_FLAG_IN ].include?( "○" ) then cameraData.validFlag_IN = 1 end
  if rowItem[ COLUMN_VALID_FLAG_OUT ].include?( "○" ) then cameraData.validFlag_OUT = 1 end
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
