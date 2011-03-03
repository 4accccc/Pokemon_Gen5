#!/usr/bin/ruby
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		conv_tool.rb
#	@brief  コンバータに使うツール類
#	@author	Toru=Nagihashi
#	@data		2011.03.03
#
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
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
