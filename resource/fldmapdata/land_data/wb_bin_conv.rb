#
#   高さ情報　削減のためのコンバーター
# 
#  アトリビュート高さ情報　*.binを
#   法線、D値テーブルとテーブルインデックスの情報の2つに分割する。
#
#　起動時にモードを渡すことで、処理が変わります。
#
# .binから頂点テーブルを生成
#   ruby wb_bin_conv.rb TBL .bin 
#
# すべての.tblをマージし.cdat作成
#   ruby wb_bin_conv.rb CDAT outfile
#
# .alltblを使用し、インデックス形式の高さ情報を出力
#   ruby wb_bin_conv.rb BIN .bin
#
# .alltblを使用し、インデックス形式の高さ情報を出力 立体交差
#   ruby wb_bin_conv.rb CROSSBIN .bin _ex.bin
#

class OutPutError < Exception; end
class ConvertError < Exception; end
class FormatError < Exception; end

#============================================================================
# デバック出力
#============================================================================
def debug_puts str
  #puts str
end



#============================================================================
# 定数部
#============================================================================

#データフラグ
WB_NORMALVTX_TRYANGLE_ONE=0         ## 三角形１つのデータ
WB_NORMALVTX_TRYANGLE_TWO=1         ## 三角形２つのデータ
WB_NORMALVTX_TRYANGLE_TWO_INDEX=2   ## 三角形２つのデータのインデックス（サイズはWB_NORMALVTXST_TR1と一緒）
WB_NORMALVTX_TRYANGLE_DATA_NONE=3   ## 三角形２つのデータのインデックス（サイズはWB_NORMALVTXST_TR1と一緒）

#フォルダ名
TMPFOLDER = "bintmp"
OUTFOLDER = "binout"


#============================================================================
# クラス宣言
#============================================================================

#=================================================
#法線・D値　テーブル情報
#=================================================
class NORMAL_D_TBL
  attr:header
  attr:normalbuff
  attr:dbuff

  def initialize file
    if file == nil then
      @header = Array.new
      @header[0] = 0
      @header[1] = 0

      @normalbuff = Array.new
      @dbuff = Array.new
    else
      loadFile( file )
    end
  end

  def loadFile file
    @header  = file.read(4).unpack( "SS" )

    debug_puts("tbl normal=#{@header[0]} d=#{@header[1]}")

    @normalbuff = Array.new
    for i in (0 ... @header[0]) do
      vec = file.read( 6 ).unpack("SSS")
      @normalbuff << vec
      debug_puts ("load #{vec.length}")
      debug_puts ("x#{vec[0]} y#{vec[1]} z#{vec[2]}")
    end

    size = @header[1] * 4
    @dbuff = file.read( size ).unpack( "I#{@header[1]}" )
  end

  def setNormal x, y, z

    #なければ入れる
    result = isNormalIn( x, y, z )
    if result[0] == false then
      #登録
      @normalbuff << [x, y, z]
      debug_puts ("add #{[x, y, z].length}")
      debug_puts ("x#{x} y#{y} z#{z}")

      @header[0] += 1
    end
  end

  def setD d
    result = isDIn( d )
    if result[0] == false then
      #登録
      @dbuff << d
      @header[1] += 1
    end
  end


  #インデックスの取得
  def getNormalIndex x, y, z

    result = isNormalIn( x, y, z )
    if result[0] == false then
      puts( "alltbl に #{x} #{y} #{z}の法線がない" )
      raise ConvertError
    end
    return result[1]
  end

  def getDIndex d

    result = isDIn( d )
    if result[0] == false then
      puts( "alltbl に #{d}の平面の位置がない" )
      raise ConvertError
    end

    return result[1]
  end

  #binary出力
  def outputBinary
    output = ""

    output = ""
    #まず、それぞれのサイズ
    output += [@header[0]].pack("S")
    output += [@header[1]].pack("S")


    #個々の情報
    @normalbuff.each{|item|
      vec = item
      debug_puts ("x #{vec[0]} y #{vec[1]} z#{vec[2]}")
      output += [vec[0]].pack("S")
      output += [vec[1]].pack("S")
      output += [vec[2]].pack("S")
    }
    @dbuff.each{|item|
      output += [item].pack("I")
    }
    return output
  end


  def dump
    puts "NormalTbl"
    roop_num = @header[0]
    for i in (0 ... roop_num) do
      vec = @normalbuff[i]
      puts( "  #{vec[0]}, #{vec[1]}, #{vec[2]}," )
    end

    puts "D_Tbl"
    @dbuff.each{|item|
      puts( "  #{item}," )
    }
  end

  def outputCDAT output
    
    output.printf( "#pragma once\n" )
    output.printf( "// このファイルはresource/fldmapdata/land_data/wb_bin_conv.rbから出力されました。\n" )
    output.printf( "\n" )
    output.printf( "\n" )
    output.printf( "#define WB_NORMAL_TBL_MAX ( #{@header[0]} )\n" )
    output.printf( "#define WB_PLANE_D_TBL_MAX ( #{@header[1]} )\n" )
    output.printf( "\n" )
    output.printf( "\n" )
    output.printf( "// 法線テーブル\n" )
    output.printf( "const u16 WB_NORMAL_TBL[] = {\n" )
    
    roop_num = @header[0]
    for i in (0 ... roop_num) do
      vec = @normalbuff[i]
      output.printf( "  #{vec[0]}, #{vec[1]}, #{vec[2]},\n" )
    end
    
    output.printf( "};\n" )
    output.printf( "\n" )
    output.printf( "\n" )
    output.printf( "// 平面の位置Dテーブル\n" )
    output.printf( "const u32 WB_PLANE_D_TBL[] = {\n" )
    @dbuff.each{|item|
      output.printf( "  #{item},\n" )
    }
    output.printf( "};\n" )

    
  end


  #重複法線のチェック
  def isNormalIn x, y, z

    tryangle_count = 0
    @normalbuff.each{|item|
      if item == [x, y, z] then
        return [true, tryangle_count]
      end
      
      #三角形の数を数える
      tryangle_count += 1
    }
    return [false, tryangle_count]
  end

  #重複D値のチェック
  def isDIn d

    d_count = 0
    @dbuff.each{|item|
      if item == d then
        return [true, d_count]
      end
      
      #数を数える
      d_count += 1
    }
    return [false, d_count]
  end
end


#=================================================
#インデックス型の法線　D値　テーブル情報
#=================================================
class WB_BINFILE
  attr:format #WB_NORMALVTX_TRYANGLE_ONEなど
  attr:vecN1  #三角形１の法線
  attr:vecN2  #三角形２の法線
  attr:D1     #三角形１のD値
  attr:D2     #三角形２のD値
  attr:attr_trtype
	attr:attr          ##アトリビュートビット    0-15がvalue 16-30がflg
	attr:tryangleType  ##三角形の形のタイプ  ／ = 0 左:vecN1,右:vecN2  ＼ = 1 右:vecN1,左:vecN2
  
  def initialize file
    if file == nil then
      @format = WB_NORMALVTX_TRYANGLE_DATA_NONE
      @vecN1 = 0
      @vecN2 = 0
      @D1 = 0
      @D2 = 0
      @attr_trtype = 0
      @attr = 0
      @tryangleType = 0
    else
      tmp = file.read(2).unpack("S")[0]
      @format = tmp & 0x3
      @vecN1  = (tmp>>2) & 0x3fff

      if (@format == WB_NORMALVTX_TRYANGLE_ONE) || (@format == WB_NORMALVTX_TRYANGLE_TWO_INDEX) then
        @D1 = file.read(2).unpack("S")[0]
        @attr_trtype = file.read(4).unpack("I")[0]
        @attr = @attr_trtype & 0x7fffffff
        @tryangleType = (@attr_trtype >> 31) & 0x00000001
      elsif @format == WB_NORMALVTX_TRYANGLE_TWO then
        
        @vecN2 = file.read(2).unpack("S")[0]
        @D1 = file.read(2).unpack("S")[0]
        @D2 = file.read(2).unpack("S")[0]
        @attr_trtype = file.read(4).unpack("I")[0]
        @attr = @attr_trtype & 0x7fffffff
        @tryangleType = (@attr_trtype >> 31) & 0x00000001
        
      else
      end
    end
  end



  #データ設定
  def setNormalFormat normal_index, d_index, attr_trtype
    @format = WB_NORMALVTX_TRYANGLE_ONE
    @vecN1  = normal_index
    @D1     = d_index
    @attr_trtype = attr_trtype
    @attr = @attr_trtype & 0x7fffffff
    @tryangleType = (@attr_trtype >> 31) & 0x00000001
  end

  def setTwoFormat n1_index, n2_index, d1_index, d2_index
    @format = WB_NORMALVTX_TRYANGLE_TWO
    @vecN1  = n1_index
    @vecN2  = n2_index
    @D1     = d1_index
    @D2     = d2_index
  end
  
  def setTwoIndexFormat dataindex, attr_trtype
    @format = WB_NORMALVTX_TRYANGLE_TWO_INDEX
    @D1     = dataindex
    @attr_trtype = attr_trtype
    @attr = @attr_trtype & 0x7fffffff
    @tryangleType = (@attr_trtype >> 31) & 0x00000001
  end


  def dump tag

    puts "tag#{tag}"
    puts "format #{@format}"
    puts "vecN1 #{@vecN1}"
    puts "vecN2 #{@vecN2}"
    puts "D1 #{@D1}"
    puts "D2 #{@D2}"
    puts "attr #{@attr}"
    puts "tryangleType #{@tryangleType}"
    
  end

  #binary出力
  def outputBinary
    output = ""
    
    #ベクトルは、ｘのテーブルインデックスにする
    head  = @format | (@vecN1 << 2)
    output += [head].pack("S")
    
    if (@format == WB_NORMALVTX_TRYANGLE_ONE) || (@format == WB_NORMALVTX_TRYANGLE_TWO_INDEX) then
      
      output += [@D1].pack("S")
      output += [@attr_trtype].pack("I")
    elsif @format == WB_NORMALVTX_TRYANGLE_TWO then
      output += [@vecN2].pack("S")
      output += [@D1].pack("S")
      output += [@D2].pack("S")
    else

      puts( "WB_FORMAT_BINARY output err" )
      raise OutPutError
    end

    return output
  end
  
end

#=================================================
#マップエディターから出力されたbinaryファイル
#=================================================
class MAPEDITOR_BINFILE

	attr:vecN1       ##１個目の三角形の法線ベクトル

	attr:vecN2       ##２個目の三角形の法線ベクトル

	attr:vecN1_D       ## ax+by+cz+d =0 のD値
	attr:vecN2_D

  attr:attr_trtype
	attr:attr          ##アトリビュートビット    0-15がvalue 16-30がflg
	attr:tryangleType  ##三角形の形のタイプ  ／ = 0 左:vecN1,右:vecN2  ＼ = 1 右:vecN1,左:vecN2

  attr:tryangle_one  #1つの三角形で大丈夫か？
  

  def initialize file

    @vecN1 = file.read(6).unpack("SSS");

    @vecN2 = file.read(6).unpack("SSS");

    @vecN1_D = file.read(4).unpack("I")[0];
    @vecN2_D = file.read(4).unpack("I")[0];

    @attr_trtype = file.read(4).unpack("I")[0];
    @attr = @attr_trtype & 0x7fffffff
    @tryangleType = (@attr_trtype >> 31) & 0x00000001


    @tryangle_one = false
    if @vecN1 == @vecN2 then
       
      if @vecN1_D == @vecN2_D then
        @tryangle_one = true 
       end
       
     end
  end

  def dump tag
    puts "tag (#{tag})"
    puts "vecN1_x #{@vecN1[0]}"
    puts "vecN1_y #{@vecN1[1]}"
    puts "vecN1_z #{@vecN1[2]}"

    puts "vecN2_x #{@vecN2[0]}"
    puts "vecN2_y #{@vecN2[1]}"
    puts "vecN2_z #{@vecN2[2]}"

    puts "vecN1_D #{@vecN1_D}"
    puts "vecN2_D #{@vecN2_D}"

    puts "attr #{@attr}"
    puts "tryangleType #{@tryangleType}"
  end

  #法線取得
  #戻り値
  #index:0 ベクトルの個数
  #index:1～ x,y,z x2,y2,z2 ...
  def getNormal
    if @tryangle_one then
      return [ 1, @vecN1[0], @vecN1[1], @vecN1[2] ]
    end
    
    return [ 2, @vecN1[0], @vecN1[1], @vecN1[2], @vecN2[0], @vecN2[1], @vecN2[2] ]
  end

  #D値取得
  #戻り値
  #index:0 D値の個数
  #index:1～ d0, d1, d2 ...
  def getD
    if @tryangle_one then
      return [ 1, @vecN1_D ]
    end
    
    return [ 2, @vecN1_D, @vecN2_D ]
  end
  
end

#=================================================
#マップエディターから出力されたbinary読み込みクラス
#=================================================
class MAPEDITOR_BINFILE_LOADER

  attr:header
  attr:dats

  attr:binaryTbl

  attr:wbformat_tbl
  
  #読み込み
  def initialize file

    @dats = Array.new
    @wbformat_tbl = Array.new

    @binaryTbl = NORMAL_D_TBL.new(nil)
    
    @header = file.read(4).unpack("SS")

    #読み込みループ
    debug_puts "#{@header[0]} #{@header[1]}"
    loop_num = @header[0] * @header[1]
    for i in  (0 ... loop_num) do
      @dats << MAPEDITOR_BINFILE.new( file )
    end
    
  end

  def addBinFile file
    header = file.read(4).unpack("SS")

    #読み込みループ
    debug_puts "#{header[0]} #{header[1]}"
    loop_num = header[0] * header[1]

    if header[0] != @header[0] then
      puts( "cross bin ブロックサイズ異常 x=#{header[0]} z#{header[1]}" )
      raise FormatError
    end
    if header[1] != @header[1] then
      puts( "cross bin ブロックサイズ異常 x=#{header[0]} z#{header[1]}" )
      raise FormatError
    end
    
    @header[1] += header[1] #Z方向にデータを足す
    for i in  (0 ... loop_num) do
      @dats << MAPEDITOR_BINFILE.new( file )
    end
  end

  #全法線　平面Dテーブル読み込み
  def loadBinaryTbl file
    @binaryTbl.loadFile file
  end

  #データ数
  def getArrayNum
    return @dats.size
  end

  #ダンプAll
  def dump

    puts "BIN Dump"
    count = 0
    @dats.each{|obj|
      obj.dump(count)
      count += 1
    }

    puts "TBL Dump"
    @binaryTbl.dump

    puts "WBBIN Dump"
    count = 0
    @wbformat_tbl.each{|obj|
      obj.dump(count)
      count += 1
    }
  end

  #テーブル生成
  def makeTbl

    @dats.each{|obj|
      normal_tbl = obj.getNormal()
      d_tbl = obj.getD()

      #マージ登録
      #法線
      i = 0
      for i in (0 ... normal_tbl[0]) do
        @binaryTbl.setNormal( normal_tbl[ 1+(i*3)+0 ], normal_tbl[ 1+(i*3)+1 ], normal_tbl[ 1+(i*3)+2 ] )
      end

      #D値
      i = 0
      for i in (0 ... d_tbl[0]) do
        @binaryTbl.setD( d_tbl[1+i] )
      end
    }
  end

  #テーブル出力
  def outputTbl
    debug_puts "output Tbl"
    return @binaryTbl.outputBinary
  end

  #データをインデックス形式に変更
  def convertWbFormat alltblfile
    debug_puts("convert")

    #全法線、D値テーブル
    loadBinaryTbl( alltblfile )

    @wbformat_tbl.clear()

    twoArray = Array.new

    #コンバート
    @dats.each{|obj|

      wbformat_bin = WB_BINFILE.new(nil)

      if obj.tryangle_one == true then

        n1_index = @binaryTbl.getNormalIndex( obj.vecN1[0], obj.vecN1[1], obj.vecN1[2] )
        d1_index = @binaryTbl.getDIndex( obj.vecN1_D )
        
        wbformat_bin.setNormalFormat( n1_index, d1_index, obj.attr_trtype )
      else

        wbformat_bin.setTwoIndexFormat( twoArray.length, obj.attr_trtype )
        
        #別バッファに２三角形データを保存
        twodata = WB_BINFILE.new(nil)
        n1_index = @binaryTbl.getNormalIndex( obj.vecN1[0], obj.vecN1[1], obj.vecN1[2] )
        d1_index = @binaryTbl.getDIndex( obj.vecN1_D )
        n2_index = @binaryTbl.getNormalIndex( obj.vecN2[0], obj.vecN2[1], obj.vecN2[2] )
        d2_index = @binaryTbl.getDIndex( obj.vecN2_D )
        twodata.setTwoFormat( n1_index, n2_index, d1_index, d2_index )
        twoArray  << twodata
      end
      
      #本データに保存
      @wbformat_tbl << wbformat_bin
    }
    
    #２三角形情報を終端に登録
    twoArray.each{|obj|
      
      @wbformat_tbl << obj
    }
    
  end

  #wbformatの情報を出力
  def outputWbFormatBinary
    output = ""

    #ヘッダー
    output += [@header[0]].pack("S")
    output += [@header[1]].pack("S")

    @wbformat_tbl.each{|obj|
      output += obj.outputBinary()
    }

    return output
  end
end


#============================================================================
# TBL生成処理
#============================================================================
def make_tbl( filename )
  begin

    output = ""

    file = File.open( filename, "rb" );

    #情報の読み込み
    loader = MAPEDITOR_BINFILE_LOADER.new( file )

    #ダンプ
    #loader.dump
    loader.makeTbl()

    output += loader.outputTbl()

    debug_puts "ofile"
    #出力
    ofilename = "#{TMPFOLDER}/" + File.basename(filename, ".*") + ".tbl"
    outputfile  = File.open(ofilename, "wb")
    outputfile.write output
    

  rescue => errors
    p errors
    #例外時
    exit(1)
  else
  end

end


#============================================================================
# 全TBLのマージを行いcdatを生成
#============================================================================
def make_AllTbl outfile

  allTbl = NORMAL_D_TBL.new(nil)
  
  #tmpないの全 Tblをマージ
  Dir.glob( "#{TMPFOLDER}/*.tbl" ).each{|filename|

    puts "merge #{filename}..."

    #情報読み込み
    file = File.open( filename, "rb" )
    tblTmp = NORMAL_D_TBL.new( file )
    file.close

    #マージ
    roop_num = tblTmp.header[0]
    for i in (0 ... roop_num) do
      vec = tblTmp.normalbuff[i]
      #法線を設定
      allTbl.setNormal( vec[0], vec[1], vec[2] )
    end

    for i in (0 ... tblTmp.header[1]) do
      #Dの値を設定
      allTbl.setD( tblTmp.dbuff[i] )
    end

    normal_num = allTbl.normalbuff.length
    d_num = allTbl.dbuff.length
    puts "normal num #{normal_num} d num #{d_num}"
  }

  #全データのCDATを出力
  outputfile = File.open( outfile, "w" )
  allTbl.outputCDAT( outputfile )
  outputfile.close()

  #binaryを出力
  output = ""
  output += allTbl.outputBinary
  outputfile  = File.open("#{TMPFOLDER}/normal_d_bin.alltbl", "wb")
  outputfile.write output
  outputfile.close()
  
end


#============================================================================
# インデックス形式のファイルを出力
#============================================================================
def make_indexBinary inputfile
  begin

    output = ""


    file = File.open( inputfile, "rb" );

    alltbl = File.open( "#{TMPFOLDER}/normal_d_bin.alltbl", "rb" )

    #情報の読み込み
    loader = MAPEDITOR_BINFILE_LOADER.new( file )

    #wbFORMATにコンバート
    loader.convertWbFormat( alltbl )

    #ダンプ
    #loader.dump

    output += loader.outputWbFormatBinary()

    debug_puts "ofile"
    #出力
    ofilename = "#{OUTFOLDER}/" + File.basename(inputfile, ".*") + ".wbbin"
    outputfile  = File.open(ofilename, "wb")
    outputfile.write output
    

  rescue => errors
    p errors
    #例外時
    exit(1)
  else
  end
end

#============================================================================
# インデックス形式のファイルを出力
#============================================================================
def make_indexCrossBinary inputfile
  begin

    output = ""

    file = File.open( inputfile[0], "rb" );
    file2 = File.open( inputfile[1], "rb" );

    alltbl = File.open( "#{TMPFOLDER}/normal_d_bin.alltbl", "rb" )

    #情報の読み込み
    loader = MAPEDITOR_BINFILE_LOADER.new( file )
    loader.addBinFile( file2 )

    #wbFORMATにコンバート
    loader.convertWbFormat( alltbl )

    output += loader.outputWbFormatBinary()

    debug_puts "ofile"
    #出力
    ofilename = "#{OUTFOLDER}/" + File.basename(inputfile[0], ".*") + ".wbbin"
    outputfile  = File.open(ofilename, "wb")
    outputfile.write output
    

  rescue => errors
    p errors
    #例外時
    exit(1)
  else
  end
end

#============================================================================
# メイン処理
#============================================================================

if ARGV[0] == "TBL" then
  make_tbl( ARGV[1] )
elsif ARGV[0] == "CDAT" then
  make_AllTbl( ARGV[1] )
elsif ARGV[0] == "BIN" then
  make_indexBinary( ARGV[1] )
else
  make_indexCrossBinary( [ARGV[1], ARGV[2]] )
end

exit(0)
