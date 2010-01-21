###################################################################################
# @brief  特殊BGMデータコンバータ
# @file   special_bgm.rb
# @author obata
# @date   2010.01.20
###################################################################################

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
  u32 flagNo;
  u32 zoneID;
  u32 soundIdx;

} SPECIAL_BGM;
" 

#==================================================================================
# 行データ
#==================================================================================
class Row
  def initialize
    @flagNo   = 0  # フラグNo.
    @zoneID   = 0  # ゾーンID
    @soundIdx = 0  # BGM No.
  end
  attr_accessor :flagNo, :zoneID, :soundIdx
end

#----------------------------------------------------------------------------------
# @brief 特殊BGMテーブルを取得する
# @param 全Rowデータの配列
# @return 特殊BGMテーブルを定義する文字列
#----------------------------------------------------------------------------------
def GetSpecialBGMTable( rows )
  string = "static const SPECIAL_BGM specialBGMTable[] = \n"
  string += "{\n"

  rows.each do |row|
    string += "  {#{row.flagNo}, #{row.zoneID}, #{row.soundIdx}},\n"
  end

  string += "};"

  return string
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
  row.flagNo   = rowItem[0]
  row.zoneID   = rowItem[1]
  row.soundIdx = rowItem[2]
  rows << row
end

# 出力
file = File.open( "special_bgm.cdat", "w" )
file.write( SUMMARY )
file.write( INCLUDE )
file.write( STRUCT )
file.write( GetSpecialBGMTable(rows) )
file.close 
