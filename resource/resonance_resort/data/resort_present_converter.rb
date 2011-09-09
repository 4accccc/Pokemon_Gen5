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
require "../../itemconv/item_hash.rb"
#-------------------------------------
# 引数チェック
# @retval true  問題なし
# @retval false エラー
#=====================================
PRESENTCSV  = ARGV[0]
OUTPUTPRE   = ARGV[1]
def checkArgument
  if PRESENTCSV == nil
    puts "引数2にcsvのファイル名を指定してください"
    return false
  end
  if OUTPUTPRE == nil
    puts "引数3に出力ファイル名を指定してください"
    return false
  end
  unless FileTest.exists?(PRESENTCSV)
    puts "#{PRESENTCSV}が見つかりません"
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
# おみやげデータコンバート
#=====================================
PRE_ROW_START      = 1     #行開始位置
PRE_COL_START      = 0     #列開始位置
PRE_COL_TAG        = PRE_COL_START    #列:タグ
PRE_COL_USE        = 1                #列:用途
PRE_COL_PERCENT    = 2                #列：確率１
PRE_COL_ITEM       = 3                #列：道具１

PRE_COL_CEL_NUM   =  2
PRE_COL_ITEM_MAX   = 10

def convertPresent( csv, data, header, hash )
  
  use_hash = Hash.new()

  for row in PRE_ROW_START...csv.row_length-1
    
    tag      = csv.get( row, PRE_COL_TAG )
    use      = csv.get( row, PRE_COL_USE )

    use_hash.store( tag, use )

    header.write( "#define RESORT_LIST_ID_#{tag}  (#{row-PRE_ROW_START})\n")

    for cnt in 0...PRE_COL_ITEM_MAX
      per     = csv.get( row, PRE_COL_PERCENT+cnt*PRE_COL_CEL_NUM )
      item    = csv.get( row, PRE_COL_ITEM+cnt*PRE_COL_CEL_NUM )

      if per == nil || item == nil 
        data.write_u16(0)
        data.write_u16(0)
      else
        data.write_u16(per)
        if $item_hash[ item ] == nil
          p "そんな名前のアイテムは存在しません"
          exit(1)
        else
          data.write_u16( $item_hash[ item ] )
        end
      end
    end
  end
  header.write( "#define RESORT_LIST_ID_MAX  (#{row-PRE_ROW_START+1})\n")

  #定義をruby用ハッシュテーブルに書き込み
  hash.write( "$item_list_hash = {\n" )
  use_hash.each{ |k,v|
    hash.write( "\t\"#{k}\"=>\"#{v}\",\n" )
  }
  hash.write( "}\n" )

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
  preCsv = CsvLoader.new
  preCsv.setSuffix( '#END' )
  preCsv.load( PRESENTCSV )

  #data作成
  preData    = BinaryBuilder.new

  #h作成
  header  = TextBuilder.new
  header.write_header( "#{OUTPUTPRE}.h", 
                    "#レゾナンスリゾートバイナリの定義情報です",
                    "Toru=Nagihashi" )

  #rb用ハッシュテーブル作成
  hash = TextBuilder.new
  hash.write( "#! ruby -Ks\n" )

  #おみやげコンバート
  convertPresent( preCsv, preData, header, hash )

  #rb書き込み
  hash.genelate( "#{OUTPUTPRE}.rb" )

  #date書き込み
  preData.genelate( "#{OUTPUTPRE}.dat" )

  #ヘッダ書き込み
  header.genelate( "#{OUTPUTPRE}.h" )
end

