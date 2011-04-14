#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		resort_city_converter.rb
#	@brief  レゾナンスリゾート街データコンバータ  （街情報タブのコンバート）
#	@author	Toru=Nagihashi
#	@data		2011.02.17
#
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../../../tools/headerdata.rb"
require "conv_tool.rb"
require "resort_define_hash.rb"
#-------------------------------------
# 引数チェック
# @retval true  問題なし
# @retval false エラー
#=====================================
TARGETCSV   = ARGV[0]
OUTPUTNAME  = ARGV[1]
def checkArgument
  if TARGETCSV == nil
    puts "引数1にcsvのファイル名を指定してください"
    return false
  end
  if OUTPUTNAME == nil
    puts "引数2に出力ファイル名を指定してください"
    return false
  end
  unless FileTest.exists?(TARGETCSV)
    puts "#{ARGV[0]}が見つかりません"
    return false
  end
  return true
end

#=============================================================================
#
#   モジュール
#
#=============================================================================
#-------------------------------------
# 街データコンバート
#=====================================
CITY_ROW_START    = 1     #行開始位置
CITY_COL_START    = 0     #列開始位置
CITY_COL_LV_LOW   = CITY_COL_START     #列:LV 下限
CITY_COL_LV_HIGH    = 1     #列：LV 上限
CITY_COL_GROUP_00   = 2     #列：弱グループ
CITY_COL_GROUP_01   = 3     #列：中グループ
CITY_COL_GROUP_02   = 4     #列：強グループ

def convertCity( csv, data )
  
  for row in CITY_ROW_START...csv.row_length-1
    
    lv_l      = csv.get( row, CITY_COL_LV_LOW )
    lv_h      = csv.get( row, CITY_COL_LV_HIGH )
    group_00  = csv.get( row, CITY_COL_GROUP_00 )
    group_01  = csv.get( row, CITY_COL_GROUP_01 )
    group_02  = csv.get( row, CITY_COL_GROUP_02 )

    data.write_u16( lv_l )
    data.write_u16( lv_h )
    data.write_u16( group_00 )
    data.write_u16( group_01 )
    data.write_u16( group_02 )

  end
end

#=============================================================================
#
#   メイン処理
#
#=============================================================================
begin

  #引数チェック
  unless checkArgument
    exit(1)
  end

  #csv読込み
  csv = CsvLoader.new
  csv.setSuffix( '#END' )
  csv.load( TARGETCSV )

  #data作成
  data    = BinaryBuilder.new

  #コンバート
  convertCity( csv, data )

  #date書き込み
  data.genelate( "#{OUTPUTNAME}.dat" )

end

