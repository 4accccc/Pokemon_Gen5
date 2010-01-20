#########################################################################################
# @brief  出入り口進入時のカメラ演出データ コンバータ
# @file   entrance_camera.rb
# @author obata
# @date   2010.01.19
#########################################################################################

# 参照ファイル名
EXIT_TYPE_DEF_FILE = "./eventdata_type.h"

# 列インデックス
ROW_EXIT_TYPE = 0  # 出入り口タイプ
ROW_PITCH     = 1  # ピッチ
ROW_YAW       = 2  # ヨー
ROW_LENGTH    = 3  # 距離
ROW_OFFSET_X  = 4  # ターゲットオフセットx
ROW_OFFSET_Y  = 5  # ターゲットオフセットy
ROW_OFFSET_Z  = 6  # ターゲットオフセットz
ROW_FRAME     = 7  # フレーム数

# 行インデックス
LINE_HEAD_DATA = 2  # 先頭データ


#========================================================================================
# カメラ演出データ
#========================================================================================
class CameraData 
  def initialize
    @exitType_lavel # 出入り口タイプ(ラベル)
    @exitType = 0   # 出入り口タイプ(値)
    @pitch    = 0   # ピッチ
    @yaw      = 0   # ヨー
    @length   = 0   # 距離
    @offsetX  = 0   # ターゲットオフセットx
    @offsetY  = 0   # ターゲットオフセットy
    @offsetZ  = 0   # ターゲットオフセットz
    @frame    = 0   # フレーム数
  end

  # アクセッサ
  attr_accessor :exitType_lavel, :exitType, 
                :pitch, :yaw, :length, :offsetX, :offsetY, :offsetZ, :frame

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
    file.close()
  end 
  #-----------------------------------------
  # @brief デバッグデータを出力する
  # @param pass 出力先ディレクトリへのパス
  def OutputDebugData( pass )
    # 出力
    filename = "#{pass}/#{@exitType_lavel.downcase}.txt"
    file = File.open( filename, "wb" )
    file.puts( "exitType = #{@exitType}(#{@exitType_lavel})" )
    file.puts( "pitch    = #{@pitch}" )
    file.puts( "yaw      = #{@yaw}" )
    file.puts( "length   = #{@length}" )
    file.puts( "offsetX  = #{@offsetX}" )
    file.puts( "offsetY  = #{@offsetY}" )
    file.puts( "offsetZ  = #{@offsetZ}" )
    file.puts( "frame    = #{@frame}" )
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

puts fileData.size

# 値を取得
LINE_HEAD_DATA.upto( fileData.size - 1 ) do |lineIndex|
  lineData = fileData[lineIndex]
  lineItem = lineData.split(/\s/)
  # 取得
  cameraData = CameraData.new
  cameraData.exitType_lavel = lineItem[ROW_EXIT_TYPE]
  cameraData.exitType       = GetExitTypeValue( lineItem[ROW_EXIT_TYPE] )
  cameraData.pitch          = ConvertToNumber( "0x" + lineItem[ROW_PITCH] )
  cameraData.yaw            = ConvertToNumber( "0x" + lineItem[ROW_YAW] )
  cameraData.length         = ConvertToNumber( "0x" + lineItem[ROW_LENGTH] )
  cameraData.offsetX        = ConvertToNumber( "0x" + lineItem[ROW_OFFSET_X] )
  cameraData.offsetY        = ConvertToNumber( "0x" + lineItem[ROW_OFFSET_Y] )
  cameraData.offsetZ        = ConvertToNumber( "0x" + lineItem[ROW_OFFSET_Z] )
  cameraData.frame          = ConvertToNumber( lineItem[ROW_FRAME] )
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
