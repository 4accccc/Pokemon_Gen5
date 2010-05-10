#=================================================
#外部から指定されたアトリビュートが使用されているかを調べます
#指定アトリビュートは、fbd.txtに１６進数でリスト化して記述してください
#検索結果がresult.txtに出力されます。
#
#進入可能な水のアトリビュートをwater.txtに列挙してください。
#水は必ず進入不可のビットで囲まれていますがこの漏れをチェックできます
#=================================================

#=================================================
#マップエディターから出力されたbinary読み込みクラス
#=================================================
class MAPEDITOR_BINFILE_LOADER

  attr:header
  attr:dats
  attr:attr_tbl

  #読み込み
  def initialize file, water_tbl
    @dats = Array.new
    @header = file.read(4).unpack("SS")
    @attr_tbl = Array.new
    @WaterTbl = water_tbl

    #読み込みループ
    loop_num = @header[0] * @header[1]
    for i in  (0 ... loop_num) do
      @dats << MAPEDITOR_BINFILE.new( file )
    end
  end

  def MakeAttrTbl
    @dats.each{ |grid|
      @attr_tbl << grid.GetAttr
    }
  end

  def DumpAttr
    @attr_tbl.each { |i|
      int = i.to_i
      p "%#x" % int
    }
  end

  def GetAttrTbl
    return @attr_tbl
  end

  def Check( result_file, fbd )
    @attr_tbl.each_with_index{ |t, idx|
      low = t & 0xFFFF
      rc = fbd.include?(low)
      if rc == true then
        printf("finded 0x%x\n",low)
        result_file.printf("X=%d Z=%d finded 0x%x\n",idx % @header[0] , idx / @header[0], low)
      end
    }
  end

  def CheckWater( result_file )
    @attr_tbl.each_with_index{ |t, idx|
      #水チェック
      if IsWater( idx ) == true then
        u = idx - @header[0]
        l = idx - 1
        r = idx + 1
        d = idx + @header[0]

        pass = 0;
      
        if u >= 0 then
          if IsWater( u ) == true || IsHit(u)==1 then
            pass+=1
          end
        else
          pass+=1
        end

        if (l>=0) && ( (l/@header[1]) == (idx/@header[1]) ) then
          if IsWater( l ) == true || IsHit(l)==1 then
            pass+=1
          end
        else
          pass+=1
        end

        if d < @header[0]*@header[1] then
          if IsWater( d ) == true || IsHit(d)==1 then
            pass+=1
          end
        else
          pass+=1
        end

        if (r < @header[0]*@header[1]) && ( (r/@header[1]) == (idx/@header[1]) ) then
          if IsWater( r ) == true || IsHit(r)==1 then
            pass+=1
          end
        else
          pass+=1
        end

        if pass != 4 then
          result_file.printf("X=%d Z=%d WaterError\n",idx % @header[0] , idx / @header[0])
        end
      end
    }
  end

  def IsWater( idx )
    atr = @attr_tbl[idx]
    low = atr & 0xFFFF
    rc = @WaterTbl.include?(low)
    return rc 
  end

  def IsHit( idx )
    atr = @attr_tbl[idx]
    hit = (atr >> 16) & 0x1
    return hit
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

  def initialize file

    @vecN1 = file.read(6).unpack("SSS");

    @vecN2 = file.read(6).unpack("SSS");

    @vecN1_D = file.read(4).unpack("I")[0];
    @vecN2_D = file.read(4).unpack("I")[0];

    @attr_trtype = file.read(4).unpack("I")[0];
    @attr = @attr_trtype & 0x7fffffff
    @tryangleType = (@attr_trtype >> 31) & 0x00000001

  end

  def GetAttr
    #puts "attr #{@attr}"
    return @attr
  end
end

#使用禁止アトリビュート一覧作成
fbd = Array.new
fbd_file = File.open( "fbd.txt", "r" );
while line = fbd_file.gets
	num = line.chomp("\n").chomp("\r")
  fbd << Integer(num)
end

#水アトリビュート一覧作成
water = Array.new
water_file = File.open( "water.txt", "r" );
while line = water_file.gets
	num = line.chomp("\n").chomp("\r")
  water << Integer(num)
end

result_file = File.open( ARGV[1], "w" );

#ディレクトリ内binファイルリストアップ
bin_ary = Dir.glob("../land_res/*.bin")
bin_ary.each{ | i |
  printf("%s check\n",i)
  result_file.printf( "%s check\n",i )
  file = File.open( i, "rb" );
  bin_loader = MAPEDITOR_BINFILE_LOADER.new( file, water )
  bin_loader.MakeAttrTbl

  if ARGV[0] == 0 then
    bin_loader.Check(result_file, fbd)
  else
    bin_loader.CheckWater(result_file)
  end
  p "clear"
}

#file = File.open( "../land_res/map24_23.bin", "rb" );
#bin_loader = MAPEDITOR_BINFILE_LOADER.new( file )
#bin_loader.MakeAttrTbl

#tbl = bin_loader.GetAttrTbl
#tbl.each{ |t|
#  low = t & 0xFFFF
#  rc = fbd.include?(low)
#  if rc == true then
#    printf("finded %x\n",low)
#    exit
#  end
#}
