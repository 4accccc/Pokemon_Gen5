#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		resort_preset_converter.rb
#	@brief  レゾナンスリゾート　ROM内人物データコンバータ 
#	@author	Toru=Nagihashi
#	@data		2011.9.4
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
PRESETCSV   = ARGV[0]
MSG_HEADER  = ARGV[1]
OBJCODE     = ARGV[2]
SHOP_HEADER = ARGV[3]
SPOT_HEADER = ARGV[4]
OUTPUTNAME  = ARGV[5]
def checkArgument
  if PRESETCSV == nil || MSG_HEADER == nil || OBJCODE == nil ||
    SHOP_HEADER == nil || SPOT_HEADER == nil || OUTPUTNAME == nil

    puts "USAGE\n"
    puts "ruby resort_preset_converter.rb preset_csv msg_header objcode shopid.h spotid.h outputname "
  end

  
  for i in 0...4
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
# プリセットのデータコンバート
#=====================================
PRESET_ROW_START    = 1     #行開始位置
PRESET_COL_START    = 0     #列開始位置
PRESET_COL_TAG      = PRESET_COL_START     #列:タグ
PRESET_COL_TYPE       = 1     #列:タイプ
PRESET_COL_NAME       = 2     #列：名前
PRESET_COL_VIEW       = 3     #列：見た目
PRESET_COL_SEX        = 4  #列：性別
PRESET_COL_ID         = 5     #列：ID
PRESET_COL_WANTJOB    = 6     #列：建てたい施設
PRESET_COL_WANTSPOT   = 7     #列：行きたいお店
PRESET_COL_INTRO      = 8     #列：挨拶
PRESET_COL_PHRASE     = 9     #列：口癖
PRESET_COL_THANKS     =10     #列：感謝
PRESET_COL_CELEB_GREET      = 11     #列：セレブ挨拶
PRESET_COL_CELEB_WANTSPOT   = 12     #列：セレブお店行きたい
PRESET_COL_CELEB_GOOUT      = 13     #列：セレブ行ってきます
PRESET_COL_CELEB_SHOP_GREET = 14     #列：セレブお店挨拶
PRESET_COL_CELEB_SHOP_OK    = 15     #列：セレブお店OK
PRESET_COL_CELEB_SHOP_NG    = 16     #列：セレブお店NG

PRESET_COL_CELEB_MSG_START  = PRESET_COL_CELEB_GREET     #列：セレブメッセージ開始
PRESET_COL_CELEB_MSG_END    = PRESET_COL_CELEB_SHOP_NG    #列：セレブメッセージ終了
def convertPreset( csv, data, header, code_h, msg_h, shop_h, spot_h )

  objdata = HeaderDataArray.new
  objdata.load( code_h )

  msgdata = HeaderDataArray.new
  msgdata.load( msg_h )

  shopdata = HeaderDataArray.new
  shopdata.load( shop_h )

  spotdata = HeaderDataArray.new
  spotdata.load( spot_h )

  for row in PRESET_ROW_START...csv.row_length-1
    
    tag     = csv.get( row, PRESET_COL_TAG )
    type     = csv.get( row, PRESET_COL_TYPE )
    name    = csv.get( row, PRESET_COL_NAME )
    view    = csv.get( row, PRESET_COL_VIEW )
    sex     = csv.get( row, PRESET_COL_SEX )
    id      = csv.get( row, PRESET_COL_ID )
    job     = csv.get( row, PRESET_COL_WANTJOB )
    spot    = csv.get( row, PRESET_COL_WANTSPOT )
    intro   = csv.get( row, PRESET_COL_INTRO )
    phrase  = csv.get( row, PRESET_COL_PHRASE )
    thanks  = csv.get( row, PRESET_COL_THANKS )


    #タグを保存
    header.write( "#define RESORT_PRESET_ID_#{tag} (#{row-PRESET_ROW_START})\n" )

    #タイプ
    data.write_u16( $define_hash[ type ] )

    #名前
    begin
      data.write_u16( msgdata.search( "#{name}", "#{row}行の名前" ) )
    rescue HeaderSearchError
      exit(1)
    end
    #見た目
    begin
      data.write_u16( objdata.search( "#{view}", "#{row}行の見た目" ) )
    rescue HeaderSearchError
      exit(1)
    end

    #性別
    data.write_u16( $define_hash[ sex ] )

    #ID
    data.write_u16( id )

    #建てたい施設
    begin
      data.write_u16( shopdata.search( "RESORT_SHOP_ID_#{job}", "#{row}行の建てたい施設" ) )
    rescue HeaderSearchError
      exit(1)
    end
    #行きたいお店
    begin
      data.write_u16( spotdata.search( "RESORT_SPOT_ID_#{spot}", "#{row}行のいきたいお店" ) )
    rescue HeaderSearchError
      exit(1)
    end
    #挨拶
    begin
      data.write_u16( msgdata.search( "#{intro}", "#{row}行の挨拶" ) )
    rescue HeaderSearchError
      exit(1)
    end
    #口癖
    begin
      data.write_u16( msgdata.search( "#{phrase}", "#{row}行の口癖" ) )
    rescue HeaderSearchError
      exit(1)
    end
    #感謝
    begin
      data.write_u16( msgdata.search( "#{thanks}", "#{row}行の感謝" ) )
    rescue HeaderSearchError
      exit(1)
    end

    if $define_hash[ type ] == "セレブ特定出現"
      for msg in PRESET_COL_CELEB_MSG_START...PRESET_COL_CELEB_MSG_END+1
        msgid  = csv.get( row, msg )
        begin
          data.write_u16( msgdata.search( "#{msgid}", "#{rom}行のセレブ文" ) )
        rescue HeaderSearchError
          exit(1)
        end
      end
    else
      for msg in PRESET_COL_CELEB_MSG_START...PRESET_COL_CELEB_MSG_END+1
        data.write_u16(0)
      end
    end


  end

  #ヘッダに書きこみ
  header.write( "#define RESORT_PRESET_ID_MAX (#{row-PRESET_ROW_START+1})\n" )

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
  csv.load( PRESETCSV )


  #data作成
  data       = BinaryBuilder.new

  #ヘッダ作成
  header      = TextBuilder.new
  header.write_header( "#{OUTPUTNAME}.h", 
                    "#レゾナンスリゾートROM内人物データDの定義情報です",
                    "Toru=Nagihashi" )
  #コンバート
  convertPreset( csv, data, header, OBJCODE, MSG_HEADER, SHOP_HEADER, SPOT_HEADER )

  #ヘッダ書き込み
  header.genelate( "#{OUTPUTNAME}.h" )

  #date書き込み
  data.genelate( "#{OUTPUTNAME}.dat" )

end


