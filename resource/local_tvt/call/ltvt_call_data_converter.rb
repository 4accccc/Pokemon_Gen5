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
COL_PERSON  = 1   #列：人物
COL_STATE   = 2   #列：状態定義
COL_ZONEID  = 3   #列：ゾーンID
COL_PLACE   = 4   #列：場所
COL_CALLFLAG= 5   #列：電話フラグ
COL_GREET_MSGID = 6   #列：メッセージID
COL_TALK_MSGID  = 7   #列：メッセージID
COL_BYE_MSGID   = 8   #列：メッセージID
COL_SP_MSGID   = 9   #列：メッセージID
COL_END     = 10   #列最後
def convertLocalTvtCallData( csv, data, header, msg, zone )

  @HashPerson = {"DOCTOR"=>0, "RIVAL"=>1, "CHELEN"=>2, "BELL"=>3, "MOTHER"=>4, "SECRET"=>5 }
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
    greet_msgid = csv.get( i, COL_GREET_MSGID )
    talk_msgid  = csv.get( i, COL_TALK_MSGID )
    bye_msgid   = csv.get( i, COL_BYE_MSGID )
    sp_msgid   = csv.get( i, COL_SP_MSGID )
    index   = cnt[person]
    cnt[person] += 1

    data.write_u16(person)
    data.write_u16(index)
    data.write_u16(place)
    if place == 0 #フィールドなのでゾーンIDを検索しいれる
      begin
      data.write_u16( zonedata.search( "ZONE_ID_#{zoneid}", "#{ARGV[0]}" ) )
      rescue HeaderSearchError
        exit(1)
      end
    else
      data.write_u16(0)
    end
    data.write_u16(call)
    begin
      value = msgdata.search( "#{greet_msgid}", "#{ARGV[0]}の#{i}行#{COL_GREET_MSGID}列" )
      data.write_u16( value )
    rescue HeaderSearchError
        exit(1)
    end
    begin
      value = msgdata.search( "#{talk_msgid}", "#{ARGV[0]}の#{i}行#{COL_TALK_MSGID}列" )
      data.write_u16( value )
    rescue HeaderSearchError
      exit(1)
    end
    begin
      value = msgdata.search( "#{bye_msgid}", "#{ARGV[0]}の#{i}行#{COL_BYE_MSGID}列" )
      data.write_u16( value )
    rescue HeaderSearchError
      exit(1)
    end
    begin
      if sp_msgid == nil && person != @HashPerson["RIVAL"]
      value = 0
      else
      value = msgdata.search( "#{sp_msgid}", "#{ARGV[0]}の#{i}行#{COL_SP_MSGID}列" )
      end
      data.write_u16( value )
    rescue HeaderSearchError
      exit(1)
    end
    data.write_u16(0) #4バイトアライメントにするためのダミー
  end

  #ヘッダの作成
  header.write( "//データ定義\n" )
  header.write( "#define LOCAL_TVT_CALLDATA_MAX (#{i})\n" )
  header.write( "//人物定義\n" )
  @HashPerson.each{ |k,v|
    header.write( "#define LOCAL_TVT_PERSON_#{k} (#{v})\n" )
  }
  header.write( "#define LOCAL_TVT_PERSON_MAX (#{@HashPerson.size})\n" )
  header.write( "//場所定義\n" )
  @HashPlace.each{ |k,v|
    header.write( "#define LOCAL_TVT_PLACE_#{k} (#{v})\n" )
  }
  header.write( "#define LOCAL_TVT_PLACE_MAX (#{@HashPlace.size})\n" )
  header.write( "//状態定義\n" )
  cnt = [0,0,0,0,0,0,0,0,0,0,0,]
  for i in ROW_START..csv.row_length-1
    person  = csv.get( i, COL_PERSON )
    state   = csv.get( i, COL_STATE )
    index   = @HashPerson[ person ]
    header.write( "#define #{person}_STATE_#{state} (#{cnt[index]})\n" )
    cnt[index]+=1
  end
  header.write( "//各状態最大数\n" )
  @HashPerson.each{ |k,v|
    header.write( "#define #{k}_STATE_MAX (#{cnt[v]})\n" )
  }

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

