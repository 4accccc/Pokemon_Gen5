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
require "resort_present.rb"
require "../../itemconv/item_hash.rb"
#-------------------------------------
# 引数チェック
# @retval true  問題なし
# @retval false エラー
#=====================================
CITYCSV     = ARGV[0]
PRESENTHEADER  = ARGV[1]
MSGHEADER   = ARGV[2]
OUTPUTCITY  = ARGV[3]
def checkArgument
  if CITYCSV == nil
    puts "引数1にcsvのファイル名を指定してください"
    return false
  end
  if PRESENTHEADER == nil
    puts "引数2におみやげへっだのファイル名を指定してください"
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
  unless FileTest.exists?(CITYCSV)
    puts "#{CITYCSV}が見つかりません"
    return false
  end
  unless FileTest.exists?(PRESENTHEADER)
    puts "#{PRESENTHEADER}が見つかりません"
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
CITY_COL_PRIZE      = 9     #列：街ご褒美
CITY_COL_CELEB      = 10    #列：セレブ抽選確率
CITY_COL_ITEM       = 11    #列：ご褒美アイテム

def convertCity( csv, data, msg_h, pre_h )
  
  #データの作成
  msgdata = HeaderDataArray.new
  msgdata.load( msg_h )

  predata = HeaderDataArray.new
  predata.load( pre_h )

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
    prize     = csv.get( row, CITY_COL_PRIZE )
    celeb     = csv.get( row, CITY_COL_CELEB )
    item      = csv.get( row, CITY_COL_ITEM )

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
    begin
      preid = predata.search( "RESORT_LIST_ID_#{pre}", "#{row}行のおみやげリスト" )

      if $item_list_hash[pre] == "おみやげ用"
        data.write_u16( preid )
      else
        put "おみやげ用以外の抽選アイテムリストが設定されています"
        exit(1)
      end
    rescue HeaderSearchError
      exit(1)
    end
    data.write_u16( per )
    if $define_hash[prize] == nil
      puts "#{prize}がおかしいです"
      exit(1)
    end
    data.write_u16( $define_hash[prize] )
    data.write_u16( celeb )
    if $item_hash.has_key?(item) == false
      puts "アイテム名がおかしいです#{item}"
      exit(1)
    end
    data.write_u16( $item_hash[item] )
    data.write_u16( 0 ) #ダミー

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

  #data作成
  cityData    = BinaryBuilder.new

  preData    = BinaryBuilder.new

  #街コンバート
  convertCity( cityCsv, cityData, MSGHEADER, PRESENTHEADER )

  #date書き込み
  cityData.genelate( "#{OUTPUTCITY}.dat" )
end

