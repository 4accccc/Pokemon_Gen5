#!/usr/bin/ruby
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		ltvt_call_data_converter.rb
#	@brief  ローカルライブキャスター呼び出しモードデータコンバーター
#	@author	Toru=Nagihashi
#	@data		2011.01.18
#
#	@usage  ruby ltvt_call_data_converter.rb local_tvt_call_data.csv typedef.h msg_h zoneid.h（出力ファイル名）
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
    puts "引数２にtype定義のヘッダファイル名を指定してください"
    return false
  end
  if ARGV[2] == nil
    puts "引数３にgmmラベルのヘッダファイル名を指定してください"
    return false
  end
  if ARGV[3] == nil
    puts "引数4にゾーンIDヘッダファイル名を指定してください"
    return false
  end
  if ARGV[4] == nil
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
# コンバートメイン
#=====================================
ROW_START   = 1   #行開始位置
COL_START   = 0   #列開始位置
COL_PERSON  = 1   #列1：人物
COL_STATE   = 2   #列2：状態定義
COL_ZONEID  = 3   #列3：ゾーンID
COL_PLACE   = 4   #列4：場所
COL_CALLFLAG= 5   #列4：電話フラグ
COL_MSGID   = 6   #列4：メッセージID
COL_TALK    = 7   #列4：会話タイプ
COL_END     = 8   #列最後
def convertLocalTvtCallData( csv, data, header, msg, zone )

  @HashPerson = {"DOCTOR"=>0, "RIVAL"=>1, "CHELEN"=>2, "BELL"=>3, "MOTHER"=>4 }
  @HashPlace  = {"FIELD"=>0, "ROOM"=>1, "DUNGEON"=>2 }
  @HashSelect = {"YES"=>1, "NO"=>0}

  #データの作成
  msgdata = HeaderDataArray.new
  msgdata.load( msg )

  zonedata = HeaderDataArray.new
  zonedata.load( zone )

  cnt = [0,0,0,0,0,0,0,0,0,0,0,]
  for i in ROW_START..csv.row_length-1
    person  = @HashPerson[ csv.get( i, COL_PERSON ) ]
    state   = csv.get( i, COL_STATE )
    place   = @HashPlace[ csv.get( i, COL_PLACE ) ]
    zoneid  = csv.get( i, COL_ZONEID )
    call    = @HashSelect[ csv.get( i, COL_CALLFLAG ) ]
    msgid   = csv.get( i, COL_MSGID )
    talk    = csv.get( i, COL_TALK )
    index   = cnt[person]
    cnt[person] += 1

    data.write_u16(person)
    data.write_u16(index)
    data.write_u16(place)
    if place == 0 #フィールドなのでゾーンIDを検索しいれる
      data.write_u16( zonedata.search( zoneid, "" ) )
    else
      data.write_u16(0)
    end
    data.write_u16(call)
    data.write_u16( msgdata.search( msgid, "" ) )
    data.write_u16(talk)
    data.write_u16(0) #4バイトアライメント用ダミー
  end

  #ヘッダの作成
  header.write( "//人物定義\n" )
  @HashPerson.each{ |k,v|
    header.write( "#define LOCAL_TVT_PERSON_#{k} (#{v})\n" )
  }
  header.write( "//場所定義\n" )
  @HashPlace.each{ |k,v|
    header.write( "#define LOCAL_TVT_PLACE_#{k} (#{v})\n" )
  }
  header.write( "//状態定義\n" )
  cnt = [0,0,0,0,0,0,0,0,0,0,0,]
  for i in ROW_START..csv.row_length-1
    person  = csv.get( i, COL_PERSON )
    state   = csv.get( i, COL_STATE )
    index   = @HashPerson[ person ]
    header.write( "#define #{person}_STATE_#{state} (#{cnt[index]})\n" )
    cnt[index]+=1
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
    write( " * @date #{Time.now} \n" )
    write( " * @note #{File.basename(__FILE__)}から自動生成されています。\n" )
    write( "*/\n" )
    write( "//============================================================================================\n" )
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
    exit(0)
  end

  #csv読込み
  csv = CsvLoader.new
  csv.load( ARGV[0] )

  #data作成
  data    = BinaryBuilder.new

  #h作成
  header  = TextBuilder.new
  header.write_header( "#{ARGV[4]}.h", 
                    "#{ARGV[4]}を使用するための定義情報です",
                    "Toru=Nagihashi" )
  #コンバート
  convertLocalTvtCallData( csv, data, header, ARGV[2], ARGV[3] )

  #date書き込み
  data.genelate( "#{ARGV[4]}.dat" )

  #h書き込み
  header.genelate( "#{ARGV[4]}.h" )

end

