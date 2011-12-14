#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		resort_spot_converter.rb
#	@brief  レゾナンスリゾート　アピールデータコンバータ  （いきたいお店タブのコンバート）
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
TARGETCSV   = ARGV[0]
MSG_HEADER  = ARGV[1]
OUTPUTNAME  = ARGV[2]
def checkArgument
  if TARGETCSV == nil
    puts "引数1にcsvのファイル名を指定してください"
    return false
  end
  if MSG_HEADER == nil
    puts "引数2にメッセージヘッダのファイル名を指定してください"
    return false
  end
  if OUTPUTNAME == nil
    puts "引数3に出力ファイル名を指定してください"
    return false
  end
  unless FileTest.exists?(TARGETCSV)
    puts "#{ARGV[0]}が見つかりません"
    return false
  end
  unless FileTest.exists?(MSG_HEADER)
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
# アピールデータコンバート
#=====================================
SPOT_ROW_START    = 1     #行開始位置
SPOT_COL_START    = 1     #列開始位置
SPOT_COL_TAG      = SPOT_COL_START     #列：タグ
SPOT_COL_MSG_M   = 2     #列:メッセージ（男）
SPOT_COL_MSG_F   = 3     #列:メッセージ（女）
SPOT_COL_GROUP    = 4     #列：グループ
SPOT_COL_BASEEXP  = 5     #列：ベース経験値
SPOT_COL_TYPE     = 6     #列：タイプ
SPOT_COL_DATA     = 7     #列：データ
SPOT_COL_EXP      = 8     #列：経験値
SPOT_COL_EFF_MSG_M  = 9     #列：msgid
SPOT_COL_EFF_MSG_F  = 10     #列：msgid

SPOT_COL_DATA_MAX = 6     #タイプ、データ、経験値が何回あるか

def convertAppeal( csv, data, msg_h, header )
  
  #データの作成
  msgdata = HeaderDataArray.new
  msgdata.load( msg_h )

  for row in SPOT_ROW_START...csv.row_length-1
    
    tag = csv.get( row, SPOT_COL_TAG )
    msg_m  = csv.get( row, SPOT_COL_MSG_M )
    msg_f  = csv.get( row, SPOT_COL_MSG_F )
    baseexp  = csv.get( row, SPOT_COL_BASEEXP )
    group  = csv.get( row, SPOT_COL_GROUP )


    #ヘッダに書きこみ
    header.write( "#define RESORT_SPOT_ID_#{tag} (#{row-SPOT_ROW_START})\n" )

    begin
      data.write_u16( msgdata.search( "#{msg_m}", "#{row}行の#{msg_h}" ) )
    rescue HeaderSearchError
      exit(1)
    end
    begin
      data.write_u16( msgdata.search( "#{msg_f}", "#{row}行の#{msg_h}" ) )
    rescue HeaderSearchError
      exit(1)
    end

    if $define_hash[group] == nil
      puts "#{group}が見つかりません"
      exit(1)
    end

    data.write_u16( $define_hash[group] )
    data.write_u16( baseexp )

    for col in 0...SPOT_COL_DATA_MAX
      type  = csv.get( row, SPOT_COL_TYPE + col*5 )
      dat   = csv.get( row, SPOT_COL_DATA + col*5 )
      exp   = csv.get( row, SPOT_COL_EXP + col*5 )
      eff_m = csv.get( row, SPOT_COL_EFF_MSG_M + col*5 )
      eff_f = csv.get( row, SPOT_COL_EFF_MSG_F + col*5 )

      if $define_hash[type] == nil
        puts "#{type}が見つかりません"
        exit(1)
      end
      if $define_hash[dat] == nil
        puts "#{dat}が見つかりません"
        exit(1)
      end
      if $define_hash[exp] == nil
        puts "#{exp}が見つかりません"
        exit(1)
      end

      data.write_u16( $define_hash[type] )
      data.write_u16( $define_hash[dat] )
      data.write_u16( $define_hash[exp] )
      begin
        data.write_u16( msgdata.search( "#{eff_m}", "#{row}行の#{msg_h}" ) )
      rescue HeaderSearchError
        exit(1)
      end
      begin
        data.write_u16( msgdata.search( "#{eff_f}", "#{row}行の#{msg_h}" ) )
      rescue HeaderSearchError
        exit(1)
      end
      data.write_u16( 0 )
    end
  end

  #ヘッダに書きこみ
  header.write( "#define RESORT_SPOT_ID_MAX (#{row-SPOT_ROW_START+1})\n" )
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

  #ヘッダ作成
  header      = TextBuilder.new
  header.write_header( "#{OUTPUTNAME}.h", 
                    "#レゾナンスリゾートアピールIDの定義情報です",
                    "Toru=Nagihashi" )

  #コンバート
  convertAppeal( csv, data, MSG_HEADER, header )

  #date書き込み
  data.genelate( "#{OUTPUTNAME}.dat" )

  #ヘッダ書き込み
  header.genelate( "#{OUTPUTNAME}.h" )
end

