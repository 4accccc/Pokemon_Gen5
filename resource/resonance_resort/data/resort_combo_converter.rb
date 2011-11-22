#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		resort_combo_converter.rb
#	@brief  レゾナンスリゾート　コンボコンバータ  （コンボ初回抽選と連続抽選シートのコンバータ）
#	@author	Toru=Nagihashi
#	@data		2011.11.21
#
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../../../tools/headerdata.rb"
require "conv_tool.rb"
#-------------------------------------
# 引数チェック
# @retval true  問題なし
# @retval false エラー
#=====================================
INITCSV   = ARGV[0]
NEXTCSV   = ARGV[1]
OUTPUTNAME1  = ARGV[2]
OUTPUTNAME2  = ARGV[3]
OUTPUTNAME3  = ARGV[4]
def checkArgument
  if INITCSV == nil
    puts "引数1にcsvのファイル名を指定してください"
    return false
  end
  if NEXTCSV == nil
    puts "引数2にcsvのファイル名を指定してください"
    return false
  end
  if OUTPUTNAME1 == nil
    puts "引数3に出力ファイル名を指定してください"
    return false
  end
  if OUTPUTNAME2 == nil
    puts "引数4に出力ファイル名を指定してください"
    return false
  end
  if OUTPUTNAME3 == nil
    puts "引数5に出力ファイル名を指定してください"
    return false
  end
  unless FileTest.exists?(INITCSV)
    puts "#{INITCSV}が見つかりません"
    return false
  end
  unless FileTest.exists?(NEXTCSV)
    puts "#{NEXTCSV}が見つかりません"
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
# 見た目データコンバート
#=====================================
VIEW_ROW_START    = 1     #行開始位置
VIEW_COL_START    = 2     #列開始位置
VIEW_COL_NUM      = 7     #列の個数
VIEW_COL_END      = VIEW_COL_START + VIEW_COL_NUM

def convertViewRnd( csv, data )

  for row in VIEW_ROW_START...csv.row_length-2
    for col in VIEW_COL_START...VIEW_COL_END
      value = csv.get( row, col )
      data.write_u16( value )
    end
  end

end

#-------------------------------------
# お店データコンバート
#=====================================
SHOP_ROW_START    = 1     #行開始位置
SHOP_COL_START    = 2     #列開始位置
SHOP_COL_NUM      = 7     #列の個数
SHOP_COL_END      = SHOP_COL_START + SHOP_COL_NUM

def convertShopRnd( csv, data )

  for row in SHOP_ROW_START...csv.row_length-2
    for col in SHOP_COL_START...SHOP_COL_END
      value = csv.get( row, col )
      data.write_u16( value )
    end
  end

end

#-------------------------------------
# お店データコンバート
#=====================================
NEXT_ROW_START    = 1     #行開始位置

NEXT_COL_NEXT_NUM = 7

NEXT_COL_TBL_RND  = 1
NEXT_COL_NEXT_RND_START = 2
NEXT_COL_NEXT_RND_END   = NEXT_COL_NEXT_RND_START + NEXT_COL_NEXT_NUM

def convertNextRnd( csv, data )

  for row in NEXT_ROW_START...csv.row_length-1
    for col in NEXT_COL_TBL_RND...NEXT_COL_NEXT_RND_END
      value = csv.get( row, col )
      data.write_u16( value )
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

  #見た目からの値csv読込み
  view_csv = CsvLoader.new
  view_csv.setPrefix( '#VIEW_START' )
  view_csv.setSuffix( '#VIEW_END' )
  view_csv.load( INITCSV )

  #お店種類からの値csv読み込み
  shop_csv = CsvLoader.new
  shop_csv.setPrefix( '#SHOP_START' )
  shop_csv.setSuffix( '#SHOP_END' )
  shop_csv.load( INITCSV )

  #連続抽選からの値csv読み込み
  next_csv = CsvLoader.new
  next_csv.setSuffix( '#END' )
  next_csv.load( NEXTCSV )

  #data作成
  view_data    = BinaryBuilder.new
  shop_data    = BinaryBuilder.new
  next_data    = BinaryBuilder.new

  #コンバート
  convertViewRnd( view_csv, view_data )
  convertShopRnd( shop_csv, shop_data )
  convertNextRnd( next_csv, next_data )

  #date書き込み
  view_data.genelate( "#{OUTPUTNAME1}.dat" )
  shop_data.genelate( "#{OUTPUTNAME2}.dat" )
  next_data.genelate( "#{OUTPUTNAME3}.dat" )
end

