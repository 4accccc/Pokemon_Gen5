#======================================================================
#	char2bmp
# キャラクターデータをBMPデータに変換する
#======================================================================

lib_path1 = File.dirname(__FILE__).gsub(/\\/,"/") + "/../ncg2imd/ncg_parser"
lib_path2 = File.dirname(__FILE__).gsub(/\\/,"/") + "/../ncg2imd/ncl_parser"
require lib_path1
require lib_path2

#========================================
# ファイル情報ヘッダ
module FileHeader
  def initialize
    super # これがないと初期化されない(オーバーライド)
    @bfType       = 'BM'  # ビットマップであることを示す
    @bfSize       = 0     # ファイルサイズ
    @bfReserved1  = 0     # 予約領域(常に0)
    @bfReserved2  = 0     # 予約領域(常に0)
    @bfOffBits    = 54     # ファイル先頭からデータまでのオフセット(0でもいい)
  end
end

#========================================
# 情報ヘッダ
module InfoHeader
  def initialize
    super # これがないと初期化されない
    @biSize           = 40  # 構造体のバイト数 -> sizeof(BITMAPCOREHEADER)
    @biWidth          = 0   # 画像の幅
    @biHeight         = 0   # 画像の高さ
    @biPlanes         = 1   # 1固定
    @biBitCount       = 24   # 16色ビットマップ
    @biCompression    = 0   # 0で無圧縮
    @biSizeImage      = 0   # 画像データ部のサイズ
    @biXPelsPerMeter  = 0   # 横方向の解像度
    @biYPelsPerMeter  = 0   # 縦方向の解像度
    @biClrUsed        = 0  # 格納されているパレット数(0でbiBitCountで示す最大数)
    @biClrImportant   = 0   # 0で全部の色が必要を示す
  end
end

#========================================
# ビットマップクラス
class Bitmap

  # モジュールを取り込む
  include FileHeader
  include InfoHeader

  # データファイル
  def setdata( out_file , ncg_file , ncl_file )

    # 色情報(16色のみ対応の設計)
    @RGBQUAD = Array.new(16)

    # NCPパーサー
    ncg_parser = NCG_STRUCT.new()
    ncg_parser.Pars(ncg_file)

    # NCLパーサー
    ncl_parser = NCL_STRUCT.new(1)
    ncl_parser.Pars(ncl_file)

    @ncg_size_w = ncg_parser.GetWidth
    @ncg_size_h = ncg_parser.GetHeight
    
    # サイズ計算
    @biWidth = @ncg_size_w*8
    @biHeight = @ncg_size_h*8

    # キャラクターデータの取得
    @charadata = ncg_parser.GetCharData

    # パレットデータの取得
    @paldata = ncl_parser.GetPaletteData

    # 出力ファイル名
    puts out_file
    @out_file_name = out_file

    # データサイズ計算
    #@bfSize = 100000
  end

  # ファイル情報書き込む
  def writefileinfo
    # ファイル情報ヘッダ
    @out.write [@bfType].pack("a*") # 2バイト
    @out.write [@bfSize].pack("I")  # 4バイト
    @out.write [@bfReserved1].pack("S") # 2バイト
    @out.write [@bfReserved2].pack("S")
    @out.write [@bfOffBits].pack("I")
  end

  # データ情報を書き込む
  def writeinfo
    # 情報ヘッダ
    @out.write [@biSize].pack("I")
    @out.write [@biWidth].pack("i")
    @out.write [@biHeight].pack("i")
    @out.write [@biPlanes].pack("S")
    @out.write [@biBitCount].pack("S")
    @out.write [@biCompression].pack("I")
    @out.write [@biSizeImage].pack("I")
    @out.write [@biXPelsPerMeter].pack("i")
    @out.write [@biYPelsPerMeter].pack("i")
    @out.write [@biClrUsed].pack("I")
    @out.write [@biClrImportant].pack("I")
  end

  # 色情報を変換する
  def colerconv( c )
    return (c.to_f/31*255).round 
  end
 
  # パレットデータをコンバートする
  def convpalette

    @rawpal = Array.new
    0.upto(15){|y|
      0.upto(15){|x|
        palpos = x+y*16
        r = (@paldata[palpos] & 0x1f)            # 赤
        g = (@paldata[palpos] & 0x3E0)  >> 5   # 緑
        b = (@paldata[palpos] & 0xfc00) >> 10  # 青

        r = colerconv(r)
        g = colerconv(g)
        b = colerconv(b)

        @rawpal << [[b].pack("C"),[g].pack("C"),[r].pack("C")]
      }
    }

  end

  # キャラクターデータのコンバート
  def convertcharadata
    @bmp_array = Array.new
    # 1キャラ8ドット
    (@ncg_size_h*8-1).downto(0){|y|
      # 横キャラ数＊４（1バイトずつ処理する)
      (@ncg_size_w*4).times{|x|

        # 今どのキャラクターにいるか
        c_x = x/4
        c_y = y/8

        # キャラクターのスタート地点の計算
        # ※ Y列＊128バイト＋X行*32
        c_start = c_x*32 + c_y*(@ncg_size_w*32)

        # キャラクタ内でどの座標にいるのか計算
        c_pos = x%4+(y%8)*4

        # 上下ビットと下位ビットを交換する
        byte = @charadata[c_start+c_pos]
        @bmp_array << @rawpal[(byte & 0xf)]
        @bmp_array << @rawpal[((byte & 0xf0) >> 4)]
      }
    }
    
  end

  # キャラクターデータの書き込み
  def writechar
    @out.write @bmp_array
  end

  # 書き込む
  def write

    @out = open( @out_file_name ,"wb" )
    @out.binmode

    # ファイル情報書き込み
    writefileinfo
    # データ情報書き込み
    writeinfo
    # キャラクタ情報書き込み
    writechar

    @out.close

  end

  # コンバート
  def dataconvert
    # パレットデータのコンバート
    convpalette
    # キャラクターデータのコンバート
    convertcharadata
  end

  # キャラクターデータの切り取り
  def cutchar( x , y ,w , h)
    tmp_array = Marshal.load(Marshal.dump(@bmp_array))
    @bmp_array.clear
    tmp_array.size.times{|i|
      pos_x = i % @biWidth
      pos_y = i / @biWidth

      if pos_x >= x and pos_x <= x+w then
        if pos_y >= y and pos_y <= y+h then
          @bmp_array << tmp_array[i]
        end
      end
    }
  end

  # データの切り取り
  def cutdata( x, y, w, h )
  
    #puts w*h
    
    cutchar(x,y,w-1,h-1)
    #puts @bmp_array.size
  
    # サイズ計算
    @biWidth  = w
    @biHeight = h
    
    #puts w
  end
end

