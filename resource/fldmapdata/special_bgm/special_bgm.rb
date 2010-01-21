###################################################################################
# @brief  特殊BGMデータコンバータ
# @file   special_bgm.rb
# @author obata
# @date   2010.01.20
###################################################################################

#==================================================================================
# 定数
#==================================================================================
# 列インデックス
ROW_FLAG_NO   = 0  # フラグNo.
ROW_ZONE_ID   = 1  # ゾーンID
ROW_SOUND_IDX = 2  # BGM No.


#==================================================================================
# 定型文
#==================================================================================
SUMMARY = "// resource/fldmapdata/special_bgm/special_bgm.rb により自動生成" 
INCLUDE = 
"
#include \"sound/wb_sound_data.sadl\"
#include \"../../../resource/fldmapdata/flagwork/flag_define.h\"
#include \"../../../resource/fldmapdata/zonetable/zone_id.h\"
" 
STRUCT = 
"
typedef struct
{
  u16 flagNo;
  u16 zoneID;
  u32 soundIdx;

} SPECIAL_BGM;
" 

#==================================================================================
# 行データ
#==================================================================================
class Row
  def initialize
    @flagNo_lavel   = 0  # フラグNo.
    @zoneID_lavel   = 0  # ゾーンID
    @soundIdx_lavel = 0  # BGM No.
  end
  attr_accessor :flagNo_lavel, :zoneID_lavel, :soundIdx_lavel
end

#----------------------------------------------------------------------------------
# @brief 特殊BGMテーブルを取得する
# @param 全ての行データを持つ配列
# @return 特殊BGMテーブルを定義する文字列
#----------------------------------------------------------------------------------
def GetSpecialBGMTable( rows )
  string = "static const SPECIAL_BGM specialBGMTable[] = \n"
  string += "{\n"

  rows.each do |row|
    string += "  {#{row.flagNo_lavel}, #{row.zoneID_lavel}, #{row.soundIdx_lavel}},\n"
  end

  string += "};"

  return string
end


#----------------------------------------------------------------------------------
# @brief フラグの実値を取得する
# @param flagName フラグ名
# @return 指定したフラグの値
#----------------------------------------------------------------------------------
def GetFlagNo( flagName ) 
  # 定義ファイルを開く
  filename = ENV["PROJECT_RSCDIR"] + "fldmapdata/flagwork/flag_define.h"
  file = File.open( filename, "r" )

  # 指定されたフラグIDを検索
  file.each do |line|
		if line.index(/#define.*#{flagName}\s*(\d*)\s*/) != nil then 
			return $1.to_i
		end
  end
  file.close

  # 指定されたフラグIDが定義されていない場合
  abort("フラグID:#{flagName}は定義されていません") 
end

#----------------------------------------------------------------------------------
# @brief ゾーンIDの実値を取得する
# @param zoneName ゾーンIDラベル名
# @return 指定したゾーンIDの値
#----------------------------------------------------------------------------------
def GetZoneID( zoneName ) 
  # 定義ファイルを開く
  filename = ENV["PROJECT_RSCDIR"] + "fldmapdata/zonetable/zone_id.h"
  file = File.open( filename, "r" )

  # 指定されたゾーンIDを検索
  file.each do |line|
		if line.index(/#define.*#{zoneName}\s*\((\d*)\)\s*/) != nil then 
			return $1.to_i
		end
  end
  file.close

  # 指定されたゾーンIDが定義されていない場合
  abort("ゾーンID:#{zoneName}は定義されていません") 
end

#----------------------------------------------------------------------------------
# @brief BGM No.の実値を取得する
# @param bgmLavel BGMラベル名
# @return 指定したBGMの値
#----------------------------------------------------------------------------------
def GetSoundIdx( bgmLavel ) 
  # 定義ファイルを開く
  filename = ENV["PROJECT_RSCDIR"] + "sound/wb_sound_data.sadl"
  file = File.open( filename, "r" )

  # 指定されたゾーンIDを検索
  file.each do |line|
		if line.index(/#define.*#{bgmLavel}\s*(\d*)\s*/) != nil then 
			return $1.to_i
		end
  end
  file.close

  # 指定されたBGMが定義されていない場合
  abort("BGM:#{bgmLavel}は定義されていません") 
end


#==================================================================================
# @brief メイン
# @param ARGV[0] コンバート対象のタブ区切りデータファイルのパス
#==================================================================================
# ファイルデータ取得
file = File.open( ARGV[0], "r" )
fileData = file.readlines
file.close

# 行データ配列
rows = Array.new

# データ抽出
1.upto( fileData.size - 1 ) do |rowIndex|
  rowData = fileData[rowIndex]
  rowItem = rowData.split(/\s/)
  row = Row.new
  row.flagNo_lavel   = rowItem[ROW_FLAG_NO]
  row.zoneID_lavel   = rowItem[ROW_ZONE_ID]
  row.soundIdx_lavel = rowItem[ROW_SOUND_IDX]
  rows << row
end

# ラベルチェック
rows.each do |row|
  GetFlagNo( row.flagNo_lavel )
  GetZoneID( row.zoneID_lavel )
  GetSoundIdx( row.soundIdx_lavel )
end

# 出力
file = File.open( "special_bgm.cdat", "w" )
file.write( SUMMARY )
file.write( INCLUDE )
file.write( STRUCT )
file.write( GetSpecialBGMTable(rows) )
file.close 
