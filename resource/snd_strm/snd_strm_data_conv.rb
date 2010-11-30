#!/usr/bin/ruby
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#   @file		snd_strm_data_conv.rb
#   @brief  サウンドストリーミングデータコンバータ
#   @author	Toru=Nagihashi
#   @data		2010.11.05
#
#   @usage  ruby snd_strm_data_conv.rb snd_strm_data.csv(入力ファイル名) snd_strm_data.cdat（出力ファイルcdat名）arc_list.txt（出力ファイルlist名）conv.bat(主力ファイルバッチ名) snd_strm_debug.cdat(デバッグ用出力ファイルcdat名)
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
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
    puts "引数２にcdatの出力ファイル名を指定してください"
    return false
  end
  if ARGV[2] == nil
    puts "引数３にアーカイブリストの出力ファイル名を指定してください"
    return false
  end
  if ARGV[3] == nil
    puts "引数４にバッチファイルの出力ファイル名を指定してください"
    return false
  end
  if ARGV[4] == nil
    puts "引数５にデバッグ用出力ファイル名を指定してください"
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
# コンバートメイン
#=====================================
ROW_START = 6 #行開始位置
COL_START = 0 #列開始位置
COL_NAME = 0    #列１：ファイル名
COL_PCM = 1     #列３：PCM
COL_HERZS = 2   #列２：ヘルツ
COL_END = 2     #列最後
def convertSndStrmData( csv, cDat, list, bat, debCDat )


  #データ用構造体
  cDat.write( "static const struct {\n"+
              "  u32 dataID;\n"+
              "  u16 hz;\n"+
              "  u16 pcm;\n"+
              "} sc_snd_strm_data[] = \n"+
              "{\n" )

  debCDat.write( "static const u16 * sc_snd_strm_debug_name[] = {\n" )

  #１行ずつ見ていく
  for i in ROW_START..csv.row_length-1

    name  = csv.get( i, COL_NAME )
    herzs = csv.get( i, COL_HERZS )
    pcm   = csv.get( i, COL_PCM )

    #名前をdatidの文字列に変える
    datName = "NARC_snd_strm_" + name.gsub(/\.wav/,'') + "_strm"

    #ヘルツを数値から定数に変換
    @HashHerzs  = {"8khz"=>0, "11khz"=>1, "22khz"=>2, "32khz"=>3 }

    #PCMを数値から定数に変換
    @HashPcm  = {"PCM8"=>0, "PCM16"=>1}

    #書き込み
    cDat.write( " {\n"+
               "    #{datName},\n"+
               "    #{@HashHerzs[herzs]},\n"+
               "    #{@HashPcm[pcm]},\n"+
               " },\n"
              )

    debCDat.write("    L\"#{name}\",\n" )

    #アーカイブリストへ追加
    list.write( "\"#{name.gsub(/\.wav/,'')}.strm\"\n" );

    #バッチファイルへ追加
    if @HashPcm[pcm] == 0
      bat.write( "strmconv -c #{name}\n" );
    end
    if @HashPcm[pcm] == 1
      bat.write( "strmconv -s #{name}\n" );
    end
  end


  #データ用構造体
  cDat.write( "};\n" )
  debCDat.write( "};\n" )


  #総数
  cDat.write( "#define SND_STRM_MAX NELEMS(sc_snd_strm_data )\n" )

end


#=============================================================================
#
#   クラス
#
#=============================================================================
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
# ｃdat作成
#=====================================
class CdatBuilder
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
  csv = CsvConvert.new
  csv.load( ARGV[0] )

  #cdat作成
  cDat  = CdatBuilder.new
  cDat.write_header( ARGV[1], 
                    "サウンドストリームのヘルツやPCM情報です",
                    "Toru=Nagihashi" )
  #デバッグ用cdat作成
  debCDat  = CdatBuilder.new
  debCDat.write_header( ARGV[4], 
                    "サウンドストリームのデバッグ情報です",
                    "Toru=Nagihashi" )

  #アーカイブリスト作成
  list  = CdatBuilder.new #Cdatじゃないけど代用できるので
  #バッチファイル作成
  bat  = CdatBuilder.new #Cdatじゃないけど代用できるので

  #コンバート
  convertSndStrmData( csv, cDat, list, bat, debCDat )

  #デバッグ用cdat書き込み
  debCDat.genelate( ARGV[4] )
  #cdat書き込み
  cDat.genelate( ARGV[1] )
  #リスト書き込み
  list.genelate( ARGV[2] )
  #バッチファイル書き込み
  bat.genelate( ARGV[3] )

end
