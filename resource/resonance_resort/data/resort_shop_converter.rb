#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		resort_people_converter.rb
#	@brief  レゾナンスリゾート　人物データコンバータ  （人物データタブ、人物定義タブのコンバート）
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
ARG_SHOP_CSV    = 0
ARG_SERVICE_CSV = 1
ARG_MSG_H       = 2
ARG_BMODEL_NAIX = 3
ARG_PATCH_H     = 4
ARG_SHOP_OUTPUT = 5
ARG_SERVICE_OUTPUT  = 6
def checkArgument
  if ARGV[ARG_SHOP_CSV] == nil
    puts "引数1にship csvのファイル名を指定してください"
    return false
  end
  if ARGV[ARG_SERVICE_CSV] == nil
    puts "引数２にservice csvのファイル名を指定してください"
    return false
  end
  if ARGV[ARG_MSG_H] == nil
    puts "引数３にgmmラベルのヘッダファイル名を指定してください"
    return false
  end
  if ARGV[ARG_BMODEL_NAIX] == nil
    puts "引数４に配置モデルのnaixファイル名を指定してください"
    return false
  end
  if ARGV[ARG_PATCH_H] == nil
    puts "引数４に配置モデルのnaixファイル名を指定してください"
    return false
  end
  if ARGV[ARG_SHOP_OUTPUT] == nil
    puts "引数5にshop 出力ファイル名を指定してください"
    return false
  end
  if ARGV[ARG_SERVICE_OUTPUT] == nil
    puts "引数5にservice 出力ファイル名を指定してください"
    return false
  end

  for i in 0...ARG_PATCH_H
    unless FileTest.exists?(ARGV[i])
      puts "#{ARGV[i]}が見つかりません"
      return false
    end
  end
  return true
end

#=============================================================================
#
#   モジュール
#
#=============================================================================
#-------------------------------------
# サービスデータコンバート
#=====================================
SERVICE_ROW_START       = 1         #行開始位置
SERVICE_COL_START       = 0         #列開始位置
SERVICE_COL_TAG         = SERVICE_COL_START     #列:タグ
SERVICE_COL_NAME_FLAG   = 1         #列：名前フラグ
SERVICE_COL_NAME        = 2         #列：サービス名
SERVICE_COL_INFO        = 3         #列：サービス説明
SERVICE_COL_MONEY       = 4         #列：値段
SERVICE_COL_KIND        = 5         #列：サービス種類
SERVICE_COL_PARAM_1     = 6         #列：サービス種類ごとの効果１
SERVICE_COL_PARAM_2     = 7         #列：サービス種類ごとの効果２
def  convertService( csv, data, h_define, h_item, gmm, h_service )
  
  #データの作成
  msgdata = HeaderDataArray.new
  msgdata.load( gmm )

  for row in SERVICE_ROW_START...csv.row_length-1
    
    #データ取得
    tag     = csv.get( row, SERVICE_COL_TAG )
    name_flg= csv.get( row, SERVICE_COL_NAME_FLAG )
    name    = csv.get( row, SERVICE_COL_NAME )
    info    = csv.get( row, SERVICE_COL_INFO )
    money   = csv.get( row, SERVICE_COL_MONEY )
    kind    = csv.get( row, SERVICE_COL_KIND );
    param1  = csv.get( row, SERVICE_COL_PARAM_1 );
    param2  = csv.get( row, SERVICE_COL_PARAM_2 )

    #タグを保存
    h_service.store( tag, row-SERVICE_ROW_START )

    #データ書き込み    
    data.write_u16( name_flg )
    #MSGTAGから値への変換
    if name == nil 
      data.write_u16( 0 )
      if name_flg == 1
        puts "名前フラグがONなのにサービス名が入っていません"
        exit(1)
      end
    else
      begin
        data.write_u16( msgdata.search( "#{name}", "#{row}行のgmm" ) )
      rescue HeaderSearchError
        exit(1)
      end
    end
    if info == nil
      data.write_u16( 0 )
      if name_flg == 1
        puts "名前フラグがONなのにサービス情報が入っていません"
        exit(1)
      end
    else
      begin
        data.write_u16( msgdata.search( "#{info}", "#{row}行のgmm" ) )
      rescue HeaderSearchError
        exit(1)
      end
    end
    data.write_u16( money )
    data.write_u16( h_define[kind] )

    if kind == "道具"
      data.write_u16( h_item[param1] )
      data.write_u16( param2 )
    else
      data.write_u16( param1 )
      data.write_u16( 0 )
      if name_flg == 0
        puts "名前フラグOFFは道具だけです"
        exit(1)
      end
    end

  end

end

#-------------------------------------
# お店データコンバート
#=====================================
SHOP_ROW_START         = 1         #行開始位置
SHOP_COL_START         = 0         #列開始位置
SHOP_COL_SHOP   = SHOP_COL_START   #列：お店種類
SHOP_COL_TYPE          = 1         #列：種類
SHOP_COL_LV            = 2         #列：お店レベル
SHOP_COL_NAME          = 3         #列：お店名
SHOP_COL_OUT_MODEL     = 4         #列：外観
SHOP_COL_IN_3DMD       = 5         #列：内装
SHOP_COL_LIST1         = 6         #列：リスト１
SHOP_COL_LIST2         = 7         #列：リスト２
SHOP_COL_LIST3         = 8         #列：リスト３
SHOP_COL_LIST4         = 9         #列：リスト４
SHOP_COL_LIST5         = 10         #列：リスト５
SHOP_COL_LIST6         = 11         #列：リスト６
SHOP_COL_LIST_START    = SHOP_COL_LIST1
SHOP_COL_LIST_END      = SHOP_COL_LIST6
SHOP_COL_END           = 12        #列最後
def convertShop( csv, data, h_define, gmm, bmodel, patch, h_service )

  #データの作成
  msgdata = HeaderDataArray.new
  msgdata.load( gmm )

  bmodeldata = HeaderDataArray.new
  bmodeldata.load( bmodel )

  patchdata = HeaderDataArray.new
  patchdata.load( patch )

  for row in SHOP_ROW_START...csv.row_length-1

    #データ取得
    shop    = csv.get( row, SHOP_COL_SHOP )
    type    = csv.get( row, SHOP_COL_TYPE )
    lv      = csv.get( row, SHOP_COL_LV )
    name    = csv.get( row, SHOP_COL_NAME )
    model   = csv.get( row, SHOP_COL_OUT_MODEL );
    in3dmd  = csv.get( row, SHOP_COL_IN_3DMD );

    #データ書き込み
    data.write_u16( h_define[shop] )
    data.write_u16( 0 ) #TODO
    data.write_u16( lv )

    begin
      data.write_u16( msgdata.search( "#{name}", "#{row}行のgmm" ) )
    rescue HeaderSearchError
      exit(1)
    end

    begin
      data.write_u16( bmodeldata.search( "NARC_output_buildmodel_outdoor_#{model}_nsbmd", "#{row}行ファイル" ) )
    rescue HeaderSearchError
      exit(1)
    end
    begin
      data.write_u16( patchdata.search( "LAND_DATA_PATCH_#{in3dmd}", "#{row}行ファイル" ) )
    rescue HeaderSearchError
      exit(1)
    end

    for i in SHOP_COL_LIST_START...SHOP_COL_LIST_END+1
      list   = csv.get( row, i )
      if list == nil
        data.write_u16(0xFF);
      else
        if h_service[list] == nil
          puts "#{list}がサービスに登録されていません"
          exit(1)
        end
        data.write_u16( h_service[list] )
      end
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
  shopCsv = CsvLoader.new
  shopCsv.setSuffix( '#END' )
  shopCsv.load( ARGV[ARG_SHOP_CSV] )

  serviceCsv = CsvLoader.new
  serviceCsv.setSuffix( '#END' )
  serviceCsv.load( ARGV[ARG_SERVICE_CSV] )

  #data作成
  serviceData = BinaryBuilder.new
  shopData    = BinaryBuilder.new

  #コンバート
  service_hash = Hash.new

  convertService( serviceCsv, serviceData, $define_hash, $item_hash, ARGV[ARG_MSG_H], service_hash )
  convertShop( shopCsv, shopData, $define_hash, ARGV[ARG_MSG_H], ARGV[ARG_BMODEL_NAIX], ARGV[ARG_PATCH_H], service_hash )

  #date書き込み
  shopData.genelate( "#{ARGV[ARG_SHOP_OUTPUT]}.dat" )
  serviceData.genelate( "#{ARGV[ARG_SERVICE_OUTPUT]}.dat" )

end

