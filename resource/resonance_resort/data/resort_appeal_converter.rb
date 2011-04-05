#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		resort_appeal_converter.rb
#	@brief  レゾナンスリゾート　アピールデータコンバータ  （アピールタブのコンバート）
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
    puts "引数1にメッセージヘッダのファイル名を指定してください"
    return false
  end
  if OUTPUTNAME == nil
    puts "引数２に出力ファイル名を指定してください"
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
APPEAL_ROW_START       = 1         #行開始位置
APPEAL_COL_START       = 0         #列開始位置
APPEAL_COL_MSG      = APPEAL_COL_START     #列:メッセージ
APPEAL_COL_TYPE     = 1         #列：タイプ
APPEAL_COL_DATA     = 2         #列：データ
APPEAL_COL_EXP      = 3         #列：経験値

APPEAL_COL_DATA_MAX = 3         #タイプ、データ、経験値が何回あるか
def  convertAppeal( csv, data, msg_h )
  
  #データの作成
  msgdata = HeaderDataArray.new
  msgdata.load( msg_h )

  for row in APPEAL_ROW_START...csv.row_length-1
    
    msg  = csv.get( row, APPEAL_COL_MSG )

    begin
      data.write_u16( msgdata.search( "#{msg}", "#{row}行の#{msg_h}" ) )
    rescue HeaderSearchError
      exit(1)
    end

    for col in 0...APPEAL_COL_DATA_MAX
      type  = csv.get( row, APPEAL_COL_TYPE + col*3 )
      dat  = csv.get( row, APPEAL_COL_DATA + col*3 )
      exp   = csv.get( row, APPEAL_COL_EXP + col*3 )

      data.write_u16( $define_hash[type] )
      data.write_u16( $define_hash[dat] )
      data.write_u16( $define_hash[exp] )
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
  csv = CsvLoader.new
  csv.setSuffix( '#END' )
  csv.load( TARGETCSV )

  #data作成
  data    = BinaryBuilder.new

  #コンバート
  convertAppeal( csv, data, MSG_HEADER )

  #date書き込み
  data.genelate( "#{OUTPUTNAME}.dat" )

end

