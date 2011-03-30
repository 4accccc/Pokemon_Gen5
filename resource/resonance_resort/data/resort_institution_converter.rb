#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		resort_institution_converter.rb
#	@brief  レゾナンスリゾート　施設データコンバータ  （施設データタブのコンバート）
#	@author	Toru=Nagihashi
#	@data		2011.02.17
#
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../../../tools/headerdata.rb"
require "conv_tool.rb"
require "resort_define_hash.rb"
require "../../itemconv/item_hash.rb"
#-------------------------------------
# 引数チェック
# @retval true  問題なし
# @retval false エラー
#=====================================
def checkArgument
  if ARGV[0] == nil
    puts "引数1に施設データcsvのファイル名を指定してください"
    return false
  end
  if ARGV[1] == nil
    puts "引数1にフィールドのイベントヘッダのファイル名を指定してください"
    return false
  end
  if ARGV[2] == nil
    puts "引数２に出力ファイル名を指定してください"
    return false
  end
  unless FileTest.exists?(ARGV[0])
    puts "#{ARGV[0]}が見つかりません"
    return false
  end
  unless FileTest.exists?(ARGV[1])
    puts "#{ARGV[1]}が見つかりません"
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
# 施設データコンバート
#=====================================
INS_ROW_START       = 1         #行開始位置
INS_COL_START       = 0         #列開始位置
INS_COL_NUMBER      = INS_COL_START     #列:番号
INS_COL_GRID_X      = 1         #列：グリッドX
INS_COL_GRID_Z      = 2         #列：グリッドZ
INS_COL_DOOR_ID     = 3         #列：ドアID
def  convertInstitution( csv, data, eventHeader )
  
  
  #データの作成
  evdata = HeaderDataArray.new
  evdata.load( eventHeader )

  for row in INS_ROW_START...csv.row_length-1
    
    #データ取得
    number  = csv.get( row, INS_COL_NUMBER )
    grid_x  = csv.get( row, INS_COL_GRID_X )
    grid_z  = csv.get( row, INS_COL_GRID_Z )
    doorID  = csv.get( row, INS_COL_DOOR_ID )

    #データ書き込み    
    data.write_u16( number )
    data.write_u16( grid_x )
    data.write_u16( grid_z )
    begin
      data.write_u16( evdata.search( "#{doorID}", "#{row}行の#{eventHeader}" ) )
    rescue HeaderSearchError
      exit(1)
    end
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
  insCsv = CsvLoader.new
  insCsv.load( ARGV[0] )

  #data作成
  insData    = BinaryBuilder.new

  #コンバート
  convertInstitution( insCsv, insData, ARGV[1] )

  #date書き込み
  insData.genelate( "#{ARGV[2]}.dat" )

end

