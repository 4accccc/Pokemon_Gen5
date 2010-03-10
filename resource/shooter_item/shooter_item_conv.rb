#!/usr/bin/ruby
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		  shooter_item_conv.rb
#	@brief    シューターアイテムコンバータ
#	@author		Toru=Nagihashi
#	@date		  2010.03.09
#
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#-------------------------------------
# エクセルコンバート
#=====================================
class CsvConvert
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
# アイテムヘッダ読み込み
#=====================================
class ItemHeader
  def initialize
    @define_name    = Array.new  #定義名
    @define_number  = Array.new  #数値
    @define_item    = Array.new  #アイテム名
  end

  #アイテムヘッダを読み込む
  def load( filename )
    File.open(filename, 'r'){|file|
      file.each{|line|
        if line =~ /#define[\s\t]+([^\s\t]+)[\s\t\(]+(\d+)[\s\t\)]+\/\/[\s\t]+(.+).+/
          #正規表現にマッチした部分を取得
          name = Regexp.last_match[1]
          number = Regexp.last_match[2]
          item  = Regexp.last_match[3]
          #puts name 
          #puts number
          #puts item
          @define_name.push( name ) 
          @define_number.push( number ) 
          @define_item.push( item ) 
        end
      }
    }
  end

  #アイテム名をキーにして、定義名と定義している数値を取り出す
  def search( itemname ) 
    index = @define_item.index( itemname )
    if index == nil
      puts itemname
      puts "見つからなかった"
      exit(1)
    end
    return @define_name[index], @define_number[ index ]
  end
end

#-------------------------------------
# ヘッダ作成
#=====================================
class HeaderBuilder
  def initialize
    @buffer       = String.new
    @header_name
  end

  def write_header( header_name, brief, date, author, note )
    @header_name  = header_name.gsub(/\./,'_');
    write( "//============================================================================================\n" )
    write( "/**\n" )
    write( " * @file " + header_name + "\n" )
    if brief != nil 
      write( " * @brief " + brief + "\n" )
    end
    if date != nil 
      write( " * @author " + author + "\n" )
    end
    if author != nil 
      write( " * @date " + date + "\n" )
    end
    if note != nil
      write( " * @note " + note + "\n" )
    end
    write( "*/\n" )
    write( "//============================================================================================\n" )
    write( "#ifndef __" + @header_name.upcase + "__\n" )
    write( "#define __" + @header_name.upcase + "__\n" )
  end

  def write_fotter
    write( "#endif //__" + @header_name.upcase + "__\n" )
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
# メイン
#=====================================
begin
  CREATE_HEADER_NAME  = "shooter_item_data.h"
  CREATE_CDAT_NAME  = "shooter_item_data.cdat"
  CREATE_RB_HASH_NAME  = "shooter_item_hash.rb"
  

  #引数エラー
  if ARGV[0] == nil
    puts "引数1にcsvのファイル名を指定してください" 
    exit(1)
  end
  if ARGV[1] == nil
    puts "引数2にitemsym.hのパスを指定してください" 
    exit(1)
  end
  unless FileTest.exists?(ARGV[0])
    puts "ファイルの場所#{ARGV[0]}が見つかりません" 
    exit(1)
  end
  unless FileTest.exists?(ARGV[1])
    puts "itemsym.hの場所#{ARGV[1]}が見つかりません" 
    exit(1)
  end

  #エクセルコンバータ
  csv_conv  = CsvConvert.new
  csv_conv.load( ARGV[0] )

  #アイテムヘッダ読み込み
  item_header = ItemHeader.new
  item_header.load( ARGV[1] )

  #書き込み内容のためデータサーチ
  #アイテム名→defineと数値→ヘッダへ書き込み
  name_buffer   = Array.new( csv_conv.row_length )
  define_buffer  = Array.new( csv_conv.row_length )
  number_buffer  = Array.new( csv_conv.row_length )
  cost_buffer  = Array.new( csv_conv.row_length )
  for i in 0..csv_conv.row_length-1
    itemname  = csv_conv.get( i, 0 )  #アイテムの名前
    cost  = csv_conv.get( i, 1 )      #コスト
    define , number = item_header.search( itemname )
 
    name_buffer[ i ]    = itemname;
    define_buffer[ i ]  = define;
    number_buffer[ i ]  = number;
    cost_buffer[ i ]    = cost;
  end

  #ヘッダ書き込み
  header_build  = HeaderBuilder.new
  day = Time.now
  header_build.write_header( CREATE_HEADER_NAME, "シューター用アイテム使用許可ビット", day.to_s, "toru=nagihashi", "このファイルは"+__FILE__+"にて自動生成されております" )
  header_build.write( "\n" )

  #内容
  for i in 0..csv_conv.row_length-1
    header_build.write( "#define\tSHOOTER_" + define_buffer[ i ] + "\t\t(" + i.to_s + ")\n" )
  end
  header_build.write( "#define\tSHOOTER_ITEM_MAX\t\t(" + (i+1).to_s + ")\n" )
  header_build.write( "\n" )

  #フッタ書き込み
  header_build.write_fotter

  #ヘッダ作成
  header_build.genelate( CREATE_HEADER_NAME )



  #Cダット書き込み
  cdat_build  = HeaderBuilder.new
  day = Time.now
  cdat_build.write_header( CREATE_CDAT_NAME, "シューター用アイテムからアイテムへの変換テーブル", day.to_s, "toru=nagihashi", "このファイルは"+__FILE__+"にて自動生成されております" )
  cdat_build.write( "\n" )

  #内容
  cdat_build.write( "static const struct {\n" )
  cdat_build.write( "u16 name;\n" )
  cdat_build.write( "u16 cost;\n" )
  cdat_build.write( "} sc_shooter_item_to_item[SHOOTER_ITEM_MAX] = {\n" )
  for i in 0..csv_conv.row_length-1
    cdat_build.write( "\t{#{number_buffer[i]}, #{cost_buffer[i]} }, //#{name_buffer[i]}\n" )
  end
  cdat_build.write( "};\n" )
  cdat_build.write( "\n" )

  #フッタ書き込み
  cdat_build.write_fotter

  #ダット作成
  cdat_build.genelate( CREATE_CDAT_NAME )


  #シューターアイテムハッシュ書き込み
  rbhash_build  = HeaderBuilder.new
  
  rbhash_build.write( "#! ruby -Ks\n" )
  rbhash_build.write( "\n" )
  rbhash_build.write( "$shooter_item_hash = {\n" )
  for i in 0..csv_conv.row_length-1
    rbhash_build.write( "\t\"#{name_buffer[i]}\"=>#{number_buffer[i]},\n" )
  end
  rbhash_build.write( "}\n" )

  #シューターアイテムハッシュ作成
  rbhash_build.genelate( CREATE_RB_HASH_NAME )

end
