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
#-------------------------------------
# 引数チェック
# @retval true  問題なし
# @retval false エラー
#=====================================
def checkArgument
  if ARGV[0] == nil
    puts "引数1にcsvのファイル名を指定してください"
    return false
  end
  if ARGV[1] == nil
    puts "引数３にgmmラベルのヘッダファイル名を指定してください"
    return false
  end
  if ARGV[2] == nil
    puts "引数4にスクリプトヘッダファイル名を指定してください"
    return false
  end
  if ARGV[3] == nil
    puts "引数5に出力ファイル名を指定してください"
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
  unless FileTest.exists?(ARGV[2])
    puts "#{ARGV[2]}が見つかりません"
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
# 人物データコンバート
#=====================================
PEOPLE_ROW_START         = 1         #行開始位置
PEOPLE_COL_START         = 0         #列開始位置
PEOPLE_COL_PERSON_TYPE   = PEOPLE_COL_START #列：人物タイプ
PEOPLE_COL_KIND          = 1         #列：種類
PEOPLE_COL_PERSONALITY   = 2         #列：性格
PEOPLE_COL_VIEW          = 3         #列：見た目
PEOPLE_COL_SEX           = 4         #列：性別
PEOPLE_COL_SCRID         = 5         #列：SCRID
PEOPLE_COL_MSGID_START   = 6         #列：MSGID開始
PEOPLE_COL_MSGID_END     = 11        #列：MSGID終了
PEOPLE_COL_END           = 12        #列最後
def convertPeople( csv, data, hashNumber, gmm, script )

  #データの作成
  msgdata = HeaderDataArray.new
  msgdata.load( gmm )

  scdata = HeaderDataArray.new
  scdata.load( script )  

  for row in PEOPLE_ROW_START...csv.row_length-1

    #データ取得
    person  = csv.get( row, PEOPLE_COL_PERSON_TYPE )
    kind    = csv.get( row, PEOPLE_COL_KIND )
    seikaku = csv.get( row, PEOPLE_COL_PERSONALITY )
    view    = csv.get( row, PEOPLE_COL_VIEW );
    sex     = csv.get( row, PEOPLE_COL_SEX );
    scrid   = csv.get( row, PEOPLE_COL_SCRID )

    #データ書き込み
    data.write_u16( hashNumber[person] )
    data.write_u16( hashNumber[kind] )
    data.write_u16( hashNumber[seikaku] )
    data.write_u16( hashNumber[view] )
    data.write_u16( hashNumber[sex] )

    #SCRIDから値への変換
    begin
      data.write_u16( scdata.search( "#{scrid}", "#{row}行のスクリプト" ) )
    rescue HeaderSearchError
      exit(1)
    end

    for i in PEOPLE_COL_MSGID_START...PEOPLE_COL_MSGID_END+1
      msg   = csv.get( row, i )
      if msg == nil
        data.write_u16(0);
        next;
      end

      #MSGTAGから値への変換
      begin
        data.write_u16( msgdata.search( "#{msg}", "#{row}行のgmm" ) )
      rescue HeaderSearchError
        exit(1)
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
  csv = CsvLoader.new
  csv.setSuffix( '#END' )
  csv.load( ARGV[0] )

  #data作成
  data    = BinaryBuilder.new

  #コンバート
  convertPeople( csv, data, $define_hash, ARGV[1], ARGV[2] )

  #date書き込み
  data.genelate( "#{ARGV[3]}.dat" )

end

