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
CITYCSV     = ARGV[0]
PRESENTCSV  = ARGV[1]
MSGHEADER   = ARGV[2]
OUTPUTCITY  = ARGV[3]
OUTPUTPRE   = ARGV[4]
def checkArgument
  if CITYCSV == nil
    puts "引数1にcsvのファイル名を指定してください"
    return false
  end
  if PRESENTCSV == nil
    puts "引数2にcsvのファイル名を指定してください"
    return false
  end
  if MSGHEADER == nil
    puts "引数2にメッセージヘッダを指定してください"
    return false
  end
  if OUTPUTCITY == nil
    puts "引数3に出力ファイル名を指定してください"
    return false
  end
  if OUTPUTPRE == nil
    puts "引数3に出力ファイル名を指定してください"
    return false
  end
  unless FileTest.exists?(CITYCSV)
    puts "#{CITYCSV}が見つかりません"
    return false
  end
  unless FileTest.exists?(PRESENTCSV)
    puts "#{PRESENTCSV}が見つかりません"
    return false
  end
  unless FileTest.exists?(MSGHEADER)
    puts "#{MSGHEADER}が見つかりません"
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
PRE_COL_PERCENT    = 1                #列：確率１
PRE_COL_ITEM       = 2                #列：道具１

PRE_COL_CEL_NUM   =  2
PRE_COL_ITEM_MAX   = 6

def convertPresent( csv, data, tag_h )
  
  for row in PRE_ROW_START...csv.row_length-1
    
    tag      = csv.get( row, PRE_COL_TAG )

    tag_h.store( tag, row-PRE_ROW_START)

    for cnt in 0...PRE_COL_ITEM_MAX
      per     = csv.get( row, PRE_COL_PERCENT+cnt*PRE_COL_CEL_NUM )
      item    = csv.get( row, PRE_COL_ITEM+cnt*PRE_COL_CEL_NUM )

      if per == nil || item == nil 
        data.write_u16(0)
        data.write_u16(0)
      else
        data.write_u16(per)
        if item == "BP" 
          data.write_u16(1000)
        else
          data.write_u16( $item_hash[ item ] )
        end
      end
    end
  end
end


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
CITY_COL_EXP        = 5     #列：経験値
CITY_COL_CITY_RANK  = 6     #列：肩書き
CITY_COL_PRE_LIST   = 7     #列：おみやげリスト
CITY_COL_PRE_PER    = 8     #列：受け取り確率

def convertCity( csv, data, msg_h, tag_h )
  
  #データの作成
  msgdata = HeaderDataArray.new
  msgdata.load( msg_h )

  for row in CITY_ROW_START...csv.row_length-1
    
    lv_l      = csv.get( row, CITY_COL_LV_LOW )
    lv_h      = csv.get( row, CITY_COL_LV_HIGH )
    group_00  = csv.get( row, CITY_COL_GROUP_00 )
    group_01  = csv.get( row, CITY_COL_GROUP_01 )
    group_02  = csv.get( row, CITY_COL_GROUP_02 )
    exp       = csv.get( row, CITY_COL_EXP )
    rank      = csv.get( row, CITY_COL_CITY_RANK )
    pre       = csv.get( row, CITY_COL_PRE_LIST )
    per       = csv.get( row, CITY_COL_PRE_PER )

    data.write_u16( lv_l )
    data.write_u16( lv_h )
    data.write_u16( group_00 )
    data.write_u16( group_01 )
    data.write_u16( group_02 )
    data.write_u16( exp )
    begin
      data.write_u16( msgdata.search( "#{rank}", "#{row}行のgmm" ) )
    rescue HeaderSearchError
      exit(1)
    end
    if tag_h[ pre ] == nil 
      exit(1)
    end
    data.write_u16( tag_h[ pre ] )
    data.write_u16( per )

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
  cityCsv = CsvLoader.new
  cityCsv.setSuffix( '#END' )
  cityCsv.load( CITYCSV )

  preCsv = CsvLoader.new
  preCsv.setSuffix( '#END' )
  preCsv.load( PRESENTCSV )

  #data作成
  cityData    = BinaryBuilder.new

  preData    = BinaryBuilder.new

  #伝達用データ
  tag_hash = Hash.new

  #おみやげコンバート
  convertPresent( preCsv, preData, tag_hash )

  #街コンバート
  convertCity( cityCsv, cityData, MSGHEADER, tag_hash )

  #date書き込み
  cityData.genelate( "#{OUTPUTCITY}.dat" )
  preData.genelate( "#{OUTPUTPRE}.dat" )

end

