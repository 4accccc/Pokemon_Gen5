
#=================================================
#マップエディターから出力されたbinaryファイル
#=================================================
class ATTR_DATA

  #1個目の三角形の法線ベクトル
	attr_reader:vecN1       

  #2個目の三角形の法線ベクトル
	attr_reader:vecN2       

  ## ax+by+cz+d =0 のD値
	attr_reader:vecN1_D       
	attr_reader:vecN2_D

  attr_reader:attr_trtype

  ##三角形の形のタイプ  ／ = 0 左:vecN1,右:vecN2  ＼ = 1 右:vecN1,左:vecN2
	attr_reader:tryangleType  
	
  # アトリビュートID,bit
  attr_reader :attr_id, :attr_bit

  def initialize file

    @vecN1 = file.read(6).unpack("SSS");

    @vecN2 = file.read(6).unpack("SSS");

    @vecN1_D = file.read(4).unpack("I")[0];
    @vecN2_D = file.read(4).unpack("I")[0];

    @attr_trtype = file.read(4).unpack("I")[0];
    @attr = @attr_trtype & 0x7fffffff
    @tryangleType = (@attr_trtype >> 31) & 0x00000001
    
    ##アトリビュートビット    0-15がvalue 16-30がflg
    
    # ビット配列を作成
    @attr_bit = Array.new
    (@attr >> 16).to_s(2).split(//).each{|i|
      @attr_bit << i.to_i
    }

    # アトリビュートIDを取得
    @attr_id = (@attr & 0xFFFF)
    
  end
end


# アトリビュートパーサー
class ATTR_PURSER
	
  attr_reader :map_w, :map_d ,:attr_tbl
  
  #読み込み
  def initialize file_name

    # ファイル読み込み
    file = File.open( file_name , "rb" )

    # ヘッダーデータ読み込み
    header = file.read(4).unpack("SS")

    # MAP幅
    @map_w = header[0]
    # MAP奥行き
    @map_d = header[1]
    # データサイズ
    @data_size = @map_w * @map_d 
    # アトリビュートデータテーブル
    @attr_tbl = Array.new

    #読み込みループ
    for i in  (0 ... @data_size) do
      @attr_tbl << ATTR_DATA.new( file )
    end
    file.close
  end

end

#data = ATTR_PURSER.new( "../land_res/m_buill01_00_00.bin" )
#puts data.map_w
#puts data.attr_tbl[0].attr_id
#print data.attr_tbl[0].attr_bit[1]


