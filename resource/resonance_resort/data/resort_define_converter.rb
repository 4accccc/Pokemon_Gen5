#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		resort_define_converter.rb
#	@brief  レゾナンスリゾート　定義データコンバータ  （定義タブのコンバート）
#	@author	Toru=Nagihashi
#	@data		2011.03.03
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
def checkArgument
  if ARGV[0] == nil
    puts "引数1にcsvのファイル名を指定してください"
    return false
  end
  if ARGV[1] == nil
    puts "引数5に出力ファイル名を指定してください"
    return false
  end
  if ARGV[2] == nil
    puts "引数5に出力ヘッダファイル名を指定してください"
    return false
  end
  unless FileTest.exists?(ARGV[0])
    puts "#{ARGV[0]}が見つかりません"
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
# 定義コンバート
#=====================================
DEFINE_ROW_START         = 1         #行開始位置
DEFINE_COL_START         = 0         #列開始位置
DEFINE_COL_PERSON_TYPE   = DEFINE_COL_START #列：人物タイプ
DEFINE_COL_RESIDENT_TYPE = 2         #列：住民タイプ
DEFINE_COL_PROXY_TYPE    = 4         #列：代理人タイプ
DEFINE_COL_TURIST_TYPE   = 6         #列：観光客タイプ
DEFINE_COL_VISITOR_TYPE  = 8         #列：滞在者フラグ
DEFINE_COL_SECRETARY_TYPE = 10       #列：秘書タイプ
DEFINE_COL_NPC_TYPE      = 12        #列：NPCタイプ
DEFINE_COL_PERSONALITY   = 14        #列：性格
DEFINE_COL_VIEW          = 16        #列：見た目
DEFINE_COL_SEX           = 18        #列：性別
DEFINE_COL_SERVICE       = 20        #列：サービス
DEFINE_COL_END           = 22        #列最後
def convertDefine( csv, header, hash, hashNumber )

  hashPersonStr     = Hash.new()
  hashResidentStr   = Hash.new()
  hashProxyStr      = Hash.new()
  hashTuristStr     = Hash.new()
  hashVisitorStr    = Hash.new()
  hashSecretaryStr  = Hash.new()
  hashNPCStr        = Hash.new()
  hashPersonaliry   = Hash.new()
  hashService       = Hash.new()

  #日本語→タグと日本語→値のハッシュを作成
  for row in DEFINE_ROW_START...csv.row_length-1
    col = DEFINE_COL_START
    while col < csv.col_length(row)-1
      engStr = csv.get( row, col )
      japStr = csv.get( row, col+1 )

      if japStr == nil 
        col+=2
        next
      end

      hashNumber.store( japStr, row-DEFINE_ROW_START )

      case col
      when DEFINE_COL_PERSON_TYPE
        hashPersonStr.store( japStr, engStr )
      when DEFINE_COL_RESIDENT_TYPE
        hashResidentStr.store( japStr, engStr )
      when DEFINE_COL_PROXY_TYPE
        hashProxyStr.store( japStr, engStr )
      when DEFINE_COL_TURIST_TYPE
        hashTuristStr.store( japStr, engStr )
      when DEFINE_COL_VISITOR_TYPE
        hashVisitorStr.store( japStr, engStr )
      when DEFINE_COL_SECRETARY_TYPE
        hashSecretaryStr.store( japStr, engStr )
      when DEFINE_COL_NPC_TYPE
        hashNPCStr.store( japStr, engStr )
      when DEFINE_COL_PERSONALITY
        hashPersonaliry.store( japStr, engStr )
      when DEFINE_COL_SERVICE
        hashService.store( japStr, engStr )
      end

      col+=2
    end
  end

  #定義をヘッダに書き込み
  header.write( "//データ定義\n" )
  header.write( "//人物\n" )
  hashPersonStr.each{ |k,v|
    header.write( "#define RESORT_PEOPLE_TYPE_#{v} (#{hashNumber[k]}) //#{k}\n" )
  }
  header.write( "#define RESORT_PEOPLE_TYPE_MAX (#{hashPersonStr.size}) //最大\n" )

  header.write( "//住民\n" )
  hashResidentStr.each{ |k,v|
    header.write( "#define RESORT_RESIDENT_TYPE_#{v} (#{hashNumber[k]}) //#{k}\n" )
  }
  header.write( "#define RESORT_RESIDENT_TYPE_MAX (#{hashResidentStr.size}) //最大\n" )

  header.write( "//代理人\n" )
  hashProxyStr.each{ |k,v|
    header.write( "#define RESORT_PROXY_TYPE_#{v} (#{hashNumber[k]}) //#{k}\n" )
  }
  header.write( "#define RESORT_PROXY_TYPE_MAX (#{hashProxyStr.size}) //最大\n" )

  header.write( "//観光客\n" )
  hashTuristStr.each{ |k,v|
    header.write( "#define RESORT_TURIST_TYPE_#{v} (#{hashNumber[k]}) //#{k}\n" )
  }
  header.write( "#define RESORT_TURIST_TYPE_MAX (#{hashTuristStr.size}) //最大\n" )

  header.write( "//滞在者\n" )
  hashVisitorStr.each{ |k,v|
    header.write( "#define RESORT_VISITOR_TYPE_#{v} (#{hashNumber[k]}) //#{k}\n" )
  }
  header.write( "#define RESORT_VISITOR_TYPE_MAX (#{hashVisitorStr.size}) //最大\n" )

  header.write( "//秘書\n" )
  hashSecretaryStr.each{ |k,v|
    header.write( "#define RESORT_SECRETARY_TYPE_#{v} (#{hashNumber[k]}) //#{k}\n" )
  }
  header.write( "#define RESORT_SECRETARY_TYPE_MAX (#{hashSecretaryStr.size}) //最大\n" )

  header.write( "//NPC\n" )
  hashNPCStr.each{ |k,v|
    header.write( "#define RESORT_NPC_TYPE_#{v} (#{hashNumber[k]}) //#{k}\n" )
  }
  header.write( "#define RESORT_NPC_TYPE_MAX (#{hashNPCStr.size}) //最大\n" )

  header.write( "//性格\n" )
  hashPersonaliry.each{ |k,v|
    header.write( "#define RESORT_PERSONALITY_TYPE_#{v} (#{hashNumber[k]}) //#{k}\n" )
  }
  header.write( "#define RESORT_PERSONALITY_TYPE_MAX (#{hashPersonaliry.size}) //最大\n" )

  header.write( "//サービス\n" )
  hashService.each{ |k,v|
    header.write( "#define RESORT_SERVICE_TYPE_#{v} (#{hashNumber[k]}) //#{k}\n" )
  }
  header.write( "#define RESORT_SERVICE_TYPE_MAX (#{hashService.size}) //最大\n" )

  #定義をruby用ハッシュテーブルに書き込み
  hash.write( "$define_hash = {\n" )
  hashNumber.each{ |k,v|
    hash.write( "\t\"#{k}\"=>#{v},\n" )
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
  csv = CsvLoader.new
  csv.load( ARGV[0] )

  #data作成
  data    = BinaryBuilder.new

  #h作成
  header  = TextBuilder.new
  header.write_header( "#{ARGV[2]}.h", 
                    "#レゾナンスリゾートバイナリの定義情報です",
                    "Toru=Nagihashi" )

  #rb用ハッシュテーブル作成
  hash = TextBuilder.new
  hash.write( "#! ruby -Ks\n" )

  #ハッシュ
  hashNumber = Hash.new()

  #コンバート
  convertDefine( csv, header, hash, hashNumber )

  #rb書き込み
  hash.genelate( "#{ARGV[1]}.rb" )

  #h書き込み
  header.genelate( "#{ARGV[2]}.h" )

end

