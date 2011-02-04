#!/usr/bin/ruby
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		field_goout_effect_converter.rb
#	@brief  室外表現データコンバーター
#	@author	Toru=Nagihashi
#	@data		2011.01.31
#
#	@usage  ruby field_goout_effect_converter.rb data.csv define.csv zoneid.h naix（出力ファイル名）
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "#{ENV["PROJECT_ROOT"]}tools/headerdata.rb"
#-------------------------------------
# 引数チェック
# @retval true  問題なし
# @retval false エラー
#=====================================
def checkArgument
  if ARGV[0] == nil
    puts "引数1にdataのcsvのファイル名を指定してください"
    return false
  end
  if ARGV[1] == nil
    puts "引数２に定義のcsvのファイル名を指定してください"
    return false
  end
  if ARGV[2] == nil
    puts "引数３にzoneid定義のヘッダファイル名を指定してください"
    return false
  end
  if ARGV[3] == nil
    puts "引数４にnaixファイル名を指定してください"
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
# 演出タイプコンバート
#=====================================
TAG_DEFINE_START  = "#define_start"
TAG_DEFINE_END    = "#define_end"

COL_DEFINE_NAME   = 0 #列：定義名
COL_DEFINE_IMD   = 1 #列：imd
COL_DEFINE_ICA   = 2 #列：ica
COL_DEFINE_IMA   = 3 #列：ima
COL_DEFINE_ITA   = 4 #列：ita
COL_DEFINE_ITP   = 5 #列：itp

def convertEffect( csv, cdat, header, effect, combi, naix )
  #拡張子変換
  @HashData = {".imd"=>"_nsbmd",".ica"=>"_nsbca",
          ".ima"=>"_nsbma", ".ita"=>"_nsbta", ".itp"=>"_nsbtp" }
  
  #データチェック用naix
  naixdata = HeaderDataArray.new
  naixdata.load( naix )

  #cdatにヘッダを書きこむ

  cdat.write( "#include \"#{header.getName}\"\n" )
  cdat.write( "#include \"#{naix}\"\n" )

  cdat.write( "//室外表現素材リスト\n" )
  cdat.write( "static const struct {\n" )
  cdat.write( " u16  imd;\n" )
  cdat.write( " u16  ica;\n" )
  cdat.write( " u16  ima;\n" )
  cdat.write( " u16  ita;\n" )
  cdat.write( " u16  itp;\n" )
  cdat.write( " u16  dummy;\n" )
  cdat.write( "} sc_goout_effect_resource[] = {\n" )

  #headerにヘッダを書きこむ
  header.write( "//演出タイプ\n" )

  #先に定義名をコンバート
  is_define = FALSE
  define_cnt  = 0
  for i in 0..csv.row_length-1
    word   = csv.get( i, 0 )

    #タグ検出
    case word
    when TAG_DEFINE_START 
      is_define = TRUE
      define_cnt  = 0
      next
    when TAG_DEFINE_END
      is_define = FALSE
    end

    #定義名範囲
    if is_define == TRUE
      name  = csv.get( i , COL_DEFINE_NAME )

      nsb   = Array.new();
      nsb[0]  = csv.get( i , COL_DEFINE_IMD )
      nsb[1]  = csv.get( i , COL_DEFINE_ICA )
      nsb[2]  = csv.get( i , COL_DEFINE_IMA )
      nsb[3]  = csv.get( i , COL_DEFINE_ITA )
      nsb[4]  = csv.get( i , COL_DEFINE_ITP )

      #コンバート後の名前に変換し
      #存在するファイルかチェック
      

      #cdatに書き込み
      cdat.write( " //GOOUT_EFF_#{name}\n" )
      cdat.write( " {\n" )
      nsb.each do |item|
        if item == nil 
          #素材設定がない
          data  = "GOOUT_EFF_RESOUCE_NONE"
        else
          #素材が設定されている
          if @HashData.key?( File.extname( item ) ) == FALSE 
            puts item
            puts "#{File.extname( item )}のファイルは対応していません\n"
            exit(1)
          end
          data  = "NARC_field_goout_effect_" + item.gsub(/\.\w+$/, @HashData[ File.extname( item ) ])
          begin
            value = naixdata.search( "#{data}", "#{naix}の#{i}行" )
          rescue HeaderSearchError
            exit(1)
          end
        end

        cdat.write( "   #{data},\n" )
      end #each
      cdat.write( " },\n" )
      #定義名ハッシュに登録
      effect.store( name, define_cnt )

      #ヘッダに書き込み
      header.write( "#define GOOUT_EFF_#{name}  (#{define_cnt})\n" )
      define_cnt+=1
    end
  end #for

  #cdatフッタ
  cdat.write( "};\n" )

  #headerフッタ
  header.write( "#define GOOUT_EFF_MAX  (#{define_cnt})\n" )
  header.write( "#define GOOUT_EFF_NULL  (GOOUT_EFF_MAX)\n" )
  header.write( "#define GOOUT_EFF_RESOUCE_NONE  (0xFFFF)\n" )
end

#-------------------------------------
# 組み合わせコンバート
#=====================================
TAG_COMBI_START   = "#combi_start"
TAG_COMBI_END     = "#combi_end"

COL_COMBI_NAME  = 0 #列：定義名
COL_COMBI_00    = 1 #列：組みあせ
COL_COMBI_01    = 2 #列：組み合わせ
COL_COMBI_02    = 3 #列：組み合わせ
COL_COMBI_03    = 4 #列：組み合わせ
def  convertCombination( csv, cdat, header, effect, combi )
  #cdatにヘッダを書きこむ
  cdat.write( "//室外表現素材リスト\n" )
  cdat.write( "static const u16 sc_goout_effect_conmbi[][4] = {\n" )

  #headerにヘッダを書きこむ
  header.write( "//演出組み合わせ\n" )

  #次に組み合わせをコンバート
  is_combi  = FALSE
  combi_cnt = 0
  for i in 0..csv.row_length-1
    word   = csv.get( i, 0 )

    #タグ検出
    case word
    when TAG_COMBI_START
      is_combi  = TRUE
      combi_cnt = 0
      next
    when TAG_COMBI_END
      is_combi  = FALSE
    end

    #組み合わせ範囲
    if is_combi == TRUE
      name  = csv.get( i , COL_COMBI_NAME )

      data   = Array.new();
      data[0]  = csv.get( i , COL_COMBI_00 )
      data[1]  = csv.get( i , COL_COMBI_01 )
      data[2]  = csv.get( i , COL_COMBI_02 )
      data[3]  = csv.get( i , COL_COMBI_03 )

      cdat.write( " //GOOUT_COMBI_#{name}\n" )
      cdat.write( " {\n" )
      data.each do |item|
        if item == nil
          cdat.write( "   GOOUT_EFF_NULL,\n" )
        else
          if effect.key?( item ) 
            cdat.write( "   GOOUT_EFF_#{item},\n" )
          else
            puts "defineのcombiにある#{item}という定義名はみつかりませんでした\n"
            exit(1)
          end
        end
      end
      cdat.write( " },\n" )

      combi.store( name, combi_cnt )
      header.write( "#define GOOUT_COMBI_#{name}  (#{combi_cnt})\n" )
      combi_cnt+=1
    end
  end #for

  #cdatフッタ
  cdat.write( "};\n" )

  #headerフッタ
  header.write( "#define GOOUT_COMBI_MAX  (#{combi_cnt})\n" )
  header.write( "#define GOOUT_COMBI_EFFECT_MAX  (4)\n" )

end #def

#-------------------------------------
# 定数コンバート
#=====================================
TAG_CONST_START   = "#const_start"
TAG_CONST_END     = "#const_end"
COL_CONST_NAME    = 0
COL_CONST_NUMBER  = 1
def convertConst( csv, header )

  #headerにヘッダを書きこむ
  header.write( "//定数\n" )

  #次に定数をコンバート
  is_const  = FALSE
  for i in 0..csv.row_length-1
    word   = csv.get( i, 0 )

    #タグ検出
    case word
    when TAG_CONST_START
      is_const  = TRUE
      next
    when TAG_CONST_END
      is_const  = FALSE
    end

    #組み合わせ範囲
    if is_const == TRUE
      name  = csv.get( i , COL_CONST_NAME )
      num   = csv.get( i , COL_CONST_NUMBER )
      header.write( "#define FIELD_GOOUT_#{name}  (#{num})\n" )
    end
  end #for
end

#-------------------------------------
# データコンバートメイン
#=====================================
EFF_SEAZON_NUM  = 4
EFF_SEAZON_IN   = 3

ROW_START       = 2   #行開始位置
COL_START       = 0   #列開始位置
COL_ZONEID      = COL_START   #列：ZONEID
COL_EFF_START   = 1   #列：エフェクト開始
COL_EFF_END     = EFF_SEAZON_NUM * EFF_SEAZON_IN + COL_EFF_START  #列：エフェクト終了

def convertData( csv, data, zone, effect, combi )

  #データチェック用naix
  zonedata = HeaderDataArray.new
  zonedata.load( zone )

  #先に定義名をコンバート
  for i in ROW_START..csv.row_length-1
    #ゾーンな
    value   = csv.get( i, COL_ZONEID )
    begin
      zoneid = zonedata.search( "ZONE_ID_#{value}", "dataの#{i}列0行" )
    rescue HeaderSearchError
      exit(1)
    end

    #組み合わせ名
    effdata = Array.new
    for j in COL_EFF_START..COL_EFF_END-1
      value = csv.get( i, j )
      if combi.key?( value ) == FALSE 
        puts "dataの#{i}列#{j}行にある組み合わせ名が見つかりませんでした"
        exit(1)
      end
      effdata[j-COL_EFF_START]  = combi[ value ]
    end

    #dataへの書き込み
    data.write_u32( zoneid )
    effdata.each do |item|
      data.write_u16( item )
    end

  end #for
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

  def write_header( filename, brief )
    @header_name  = filename 
    write( "//============================================================================================\n" )
    write( "/**\n" )
    write( " * @file " + filename + "\n" )
    if brief != nil 
      write( " * @brief " + brief + "\n" )
    end
    write( " * @author Toru=Nagihashi\n" )
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

  def getName()
    return @header_name
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
  defCsv  = CsvLoader.new
  defCsv.load( ARGV[1] )

  #data作成
  data    = BinaryBuilder.new

  #h作成
  header  = TextBuilder.new
  header.write_header( "#{ARGV[4]}.h", 
                    "#{ARGV[4]}を使用するための定義情報です" )
  #Cdat作成
  cdat  = TextBuilder.new
  cdat.write_header( "#{ARGV[4]}.cdat",
                    "#{ARGV[4]}のリソース情報です" )

  #コンバート
  effect  = Hash.new
  combi   = Hash.new
  convertEffect( defCsv, cdat, header, effect, combi, ARGV[3]  )
  convertCombination( defCsv, cdat, header, effect, combi )
  convertConst( defCsv, header )
  convertData( dataCsv, data, ARGV[2], effect, combi )

  #dat書き込み
  data.genelate( "#{ARGV[4]}.dat" )

  #Cdat書き込み 
  cdat.genelate( "#{ARGV[4]}.cdat" )

  #h書き込み
  header.genelate( "#{ARGV[4]}.h" )

end

