#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		resort_npc_converter.rb
#	@brief  レゾナンスリゾート　NPCデータコンバータ 
#	@author	Toru=Nagihashi
#	@data		2011.04.7
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
NPCCSV      = ARGV[0]
KUTIKOMICSV = ARGV[1]
OFFRECOCSV  = ARGV[2]
MSG_HEADER  = ARGV[3]
OUTPUTNAME_NPC      = ARGV[4]
OUTPUTNAME_KUTIKOMI = ARGV[5]
OUTPUTNAME_OFFRECO  = ARGV[6]
def checkArgument
  if NPCCSV == nil
    puts "引数1にcsvのファイル名を指定してください"
    return false
  end
  if KUTIKOMICSV == nil
    puts "引数2にcsvのファイル名を指定してください"
    return false
  end
  if OFFRECOCSV == nil
    puts "引数3にcsvのファイル名を指定してください"
    return false
  end
  if MSG_HEADER == nil
    puts "引数4にメッセージヘッダのファイル名を指定してください"
    return false
  end
  if OUTPUTNAME_NPC == nil
    puts "引数5に出力ファイル名を指定してください"
    return false
  end
  if OUTPUTNAME_KUTIKOMI == nil
    puts "引数6に出力ファイル名を指定してください"
    return false
  end
  if OUTPUTNAME_OFFRECO == nil
    puts "引数7に出力ファイル名を指定してください"
    return false
  end
  unless FileTest.exists?(NPCCSV)
    puts "#{NPCCSV}が見つかりません"
    return false
  end
  unless FileTest.exists?(KUTIKOMICSV)
    puts "#{KUTIKOMICSV}が見つかりません"
    return false
  end
  unless FileTest.exists?(OFFRECOCSV)
    puts "#{OFFRECOCSV}が見つかりません"
    return false
  end
  unless FileTest.exists?(MSG_HEADER)
    puts "#{MSG_HEADER}が見つかりません"
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
# クチコミデータコンバート
#=====================================
KUTIKOMI_ROW_START    = 1     #行開始位置
KUTIKOMI_COL_START    = 0     #列開始位置
KUTIKOMI_COL_TAG      = KUTIKOMI_COL_START     #列:タグ
KUTIKOMI_COL_QUESTION      = 2     #列:質問文
KUTIKOMI_COL_ANSWER_NUM    = 3     #列:回答数
KUTIKOMI_COL_ANSWER_START  = 4     #列：回答開始

def convertKutikomi( csv, data, msg_h, tagHash )
  
  #データの作成
  msgdata = HeaderDataArray.new
  msgdata.load( msg_h )

  for row in KUTIKOMI_ROW_START...csv.row_length-1
    
    tag   = csv.get( row, KUTIKOMI_COL_TAG )
    ques  = csv.get( row, KUTIKOMI_COL_QUESTION )
    num   = csv.get( row, KUTIKOMI_COL_ANSWER_NUM )

    tagHash.store( tag, row-KUTIKOMI_ROW_START )

    begin
      data.write_u16( msgdata.search( "#{ques}", "#{row}行の#{msg_h}" ) )
    rescue HeaderSearchError
      exit(1)
    end

    data.write_u16( num )

    for index in KUTIKOMI_COL_ANSWER_START...KUTIKOMI_COL_ANSWER_START + num.to_i - 1
      answer   = csv.get( row, index )
      begin
        data.write_u16( msgdata.search( "#{answer}", "#{row}行の#{msg_h}" ) )
      rescue HeaderSearchError
        exit(1)
      end
    end


  end
end

#-------------------------------------
# NPCのデータコンバート
#=====================================
NPC_ROW_START    = 1     #行開始位置
NPC_COL_START    = 0     #列開始位置
NPC_COL_VIEW   = NPC_COL_START     #列:見た目
NPC_COL_GRID_X   = 1     #列:X座標
NPC_COL_GRID_Z   = 2     #列:Y座標
NPC_COL_CITY_LV  = 3     #列：街レベル
NPC_COL_MOVE     = 4     #列：移動
NPC_COL_LIMIT_X  = 5     #列：X制限
NPC_COL_LIMIT_Z  = 6     #列：Y制限
NPC_COL_TYPE     = 7     #列：タイプ
NPC_COL_KIND     = 8     #列：種類
def convertNpc( csv, data, tagHash )

  for row in NPC_ROW_START...csv.row_length-1
    
    view    = csv.get( row, NPC_COL_VIEW )
    gridx   = csv.get( row, NPC_COL_GRID_X )
    gridz   = csv.get( row, NPC_COL_GRID_Z )
    move    = csv.get( row, NPC_COL_MOVE )
    limit_x = csv.get( row, NPC_COL_LIMIT_X )
    limit_z = csv.get( row, NPC_COL_LIMIT_Z )
    type    = csv.get( row, NPC_COL_TYPE )
    kind    = csv.get( row, NPC_COL_KIND )

    data.write_u16( $define_hash[ view ] )
    data.write_u16( gridx )
    data.write_u16( gridz )
    data.write_u16( $define_hash[ move ] )
    data.write_u16( limit_x )
    data.write_u16( limit_z )
    data.write_u16( $define_hash[ type ] )
    data.write_u16( tagHash[ kind ] )

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
  npcCsv = CsvLoader.new
  npcCsv.setSuffix( '#END' )
  npcCsv.load( NPCCSV )

  kutikomiCsv = CsvLoader.new
  kutikomiCsv.setSuffix( '#END' )
  kutikomiCsv.load( KUTIKOMICSV )

  offrecoCsv = CsvLoader.new
  offrecoCsv.setSuffix( '#END' )
  offrecoCsv.load( OFFRECOCSV )

  #data作成
  npcData       = BinaryBuilder.new
  kutikomiData  = BinaryBuilder.new
  offrecoData   = BinaryBuilder.new

  #それぞれのタグを渡すためのハッシュ
  tagHash = Hash.new

  #コンバート
  convertKutikomi( kutikomiCsv, kutikomiData, MSG_HEADER, tagHash )
  #convertOffreco( offrecoCsv, offrecoData, MSG_HEADER, tagHash )
  convertNpc( npcCsv, npcData, tagHash )

  #date書き込み
  npcData.genelate( "#{OUTPUTNAME_NPC}.dat" )
  kutikomiData.genelate( "#{OUTPUTNAME_KUTIKOMI}.dat" )
  offrecoData.genelate( "#{OUTPUTNAME_OFFRECO}.dat" )

end

