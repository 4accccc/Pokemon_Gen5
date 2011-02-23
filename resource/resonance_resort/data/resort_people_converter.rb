#!/usr/bin/ruby
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
    puts "引数1にcsvのファイル名を指定してください"
    return false
  end
  if ARGV[2] == nil
    puts "引数３にgmmラベルのヘッダファイル名を指定してください"
    return false
  end
  if ARGV[3] == nil
    puts "引数4にスクリプトヘッダファイル名を指定してください"
    return false
  end
  if ARGV[4] == nil
    puts "引数5に出力ファイル名を指定してください"
    return false
  end
  if ARGV[5] == nil
    puts "引数5に出力ヘッダファイル名を指定してください"
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
  unless FileTest.exists?(ARGV[3])
    puts "#{ARGV[3]}が見つかりません"
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
# 人物定義コンバート
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
DEFINE_COL_END           = 20        #列最後
def convertDefine( csv, header, hashNumber )

  hashPersonStr     = Hash.new()
  hashResidentStr   = Hash.new()
  hashProxyStr      = Hash.new()
  hashTuristStr     = Hash.new()
  hashVisitorStr    = Hash.new()
  hashSecretaryStr  = Hash.new()
  hashNPCStr        = Hash.new()
  hashPersonaliry   = Hash.new()

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
end

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

  for row in DEFINE_ROW_START...csv.row_length-1

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
#   クラス
#
#=============================================================================
#-------------------------------------
# エクセルローダー
#=====================================
class CsvLoader
  require "csv"

  def initialize
    @buffer

    @row_length  = 0          #行
    @col_length = Array.new   #列
  end

  def load( filename )
    #長さ取得
    CSV.open( filename, 'r' ) do |row|
      @row_length  += 1
      @col_length.push( row.size )
    end

    #二次元配列
    @buffer = Array.new(@row_length)
    @buffer.each_index{ |i|
      @buffer[i] = Array.new(@col_length[i])
    }

    #データ取得
    i = 0
    j = 0
    CSV.open( filename, 'r' ) do |row|
      row.each do |cell|
        @buffer[j][i] = cell
        i += 1
      end
      j += 1
      i = 0
    end
  end

  def get( row, col )
    return @buffer[row][col]
  end

  def row_length
    return @row_length
  end

  def col_length( row )
    return @col_length[ row ]
  end
end

#-------------------------------------
# テキストファイル作成
#=====================================
class TextBuilder
  def initialize
    @buffer       = String.new
    @header_name
  end

  def write_header( filename, brief, author )
    @header_name  = filename.gsub(/\./,'_');
    write( "//============================================================================================\n" )
    write( "/**\n" )
    write( " * @file " + filename + "\n" )
    if brief != nil 
      write( " * @brief " + brief + "\n" )
    end
    if author != nil 
      write( " * @author " + author + "\n" )
    end
    write( " * @note #{File.basename(__FILE__)}から自動生成されています。\n" )
    write( "*/\n" )
    write( "//============================================================================================\n" )
    write( "#pragma once\n" )
    #write( "#ifndef __" + @header_name.upcase + "__\n" )
    #write( "#define __" + @header_name.upcase + "__\n" )
  end

  def write_fotter
    #write( "#endif //__" + @header_name.upcase + "__\n" )
  end

  def write( str )
    @buffer << str
  end

  def genelate( filename )
    newfile = File.new( filename, "w" )
    @buffer.each{ |line|
      newfile.puts( line )
    }
    newfile.close
  end
end

#-------------------------------------
# バイナリファイル作成
#=====================================
class BinaryBuilder
  def initialize
    @buffer = [];
  end

  def write_u8( data )
     @buffer  << [data.to_i].pack('C')
  end

  def write_u16( data )
    @buffer   << [data.to_i].pack('S')
  end

  def write_u32( data )
    @buffer   << [data.to_i].pack('I')
  end

  def genelate( filename )
    newfile = File.new( filename, "wb" )
    @buffer.join
    newfile.write( @buffer )
    newfile.close
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
  dataCsv = CsvLoader.new
  dataCsv.load( ARGV[0] )

  defineCsv = CsvLoader.new
  defineCsv.load( ARGV[1] )

  #data作成
  data    = BinaryBuilder.new

  #h作成
  header  = TextBuilder.new
  header.write_header( "#{ARGV[5]}.h", 
                    "#{ARGV[4]}を使用するための定義情報です",
                    "Toru=Nagihashi" )

  #ハッシュ
  hashNumber = Hash.new()
  #コンバート
  convertDefine( defineCsv, header, hashNumber )
  convertPeople( dataCsv, data, hashNumber, ARGV[2], ARGV[3] )

  #date書き込み
  data.genelate( "#{ARGV[4]}.dat" )

  #h書き込み
  header.genelate( "#{ARGV[5]}.h" )

end

