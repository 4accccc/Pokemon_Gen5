
=begin
/// 高さデータ取得用
typedef struct {
	fx16	vecN1_x;        //１個目の三角形の法線ベクトル
	fx16	vecN1_y;
	fx16	vecN1_z;

	fx16	vecN2_x;       //２個目の三角形の法線ベクトル
	fx16	vecN2_y;
	fx16	vecN2_z;

	fx32	vecN1_D;       // ax+by+cz+d =0 のD値
	fx32	vecN2_D;

	u32		attr:31;          //アトリビュートビット    0-15がvalue 16-30がflg
	u32		tryangleType:1;   //三角形の形のタイプ  ／ = 0 左:vecN1,右:vecN2  ＼ = 1 右:vecN1,左:vecN2
} NormalVtxSt;

typedef struct{
    u16			width;           // 高さデータの横の個数
    u16			height;          // 高さデータの縦の個数
//	NormalVtxSt* vtxData;     // 高さ
=end


def print_vec( file, str_vec, str_d )
  vec_array = str_vec.unpack("SSS")
  file.printf("%04x,%04x,%04x ... %d\n", vec_array[0], vec_array[1], vec_array[2], str_d.unpack("I")[0])
end

def reader( filename, logfile )

  vec_pat = []
  d_pat = []

  file = File.open(filename ,"rb" )
  w_h = file.read(4).unpack("SS")
  printf( " %s (%d,%d)\n", filename, w_h[0], w_h[1] )
  logfile.printf( "# %s (%d,%d)\n", filename, w_h[0], w_h[1] )

  for i in  (0 ... w_h[0] * w_h[1] ) do
    strVecN1 = file.read(6)
    strVecN2 = file.read(6)
    strValD1 = file.read(4)
    strValD2 = file.read(4)
    strVttr = file.read(4)
    #key_N1 = strVecN1 + strValD1
    #key_N2 = strVecN2 + strValD2
    #pat << key_N1
    #pat << key_N2
    vec_pat << strVecN1
    vec_pat << strVecN2
    d_pat << strValD1
    d_pat << strValD2
    print_vec( logfile, strVecN1, strValD1 )
    print_vec( logfile, strVecN2, strValD2 )
  end

  file.close()
  #return pat
  return [ vec_pat, d_pat ]
end


vec_file = File.open("vec_file.txt","w")
vecTotalPattern = []
dTotalPattern = []
file_count = 0
Dir.glob( "land_res/*.bin" ).each{|file|
  file_count += 1
  pat = reader( file, vec_file )
  vec_pat = pat[0]
  d_pat = pat[1]
  vecTotalPattern += vec_pat
  dTotalPattern += d_pat
}
vec_file.close()
puts "total binary file = #{file_count}"

vec_keys = Hash.new
vecTotalPattern.each{|value|
  if vec_keys.has_key?(value) then
    vec_keys[value] += 1
  else
    vec_keys[value] = 1
  end
}
d_keys = Hash.new
d_reduce_keys = Hash.new
dTotalPattern.each{|value|
  if d_keys.has_key?(value) then
    d_keys[value] += 1
  else
    d_keys[value] = 1
  end
  reduce_value = (value.unpack("I")[0] >> 4 )
  if d_reduce_keys.has_key?(reduce_value) then
    d_reduce_keys[reduce_value] += 1
  else
    d_reduce_keys[reduce_value] = 1
  end
}
puts "vector valiation = #{vec_keys.size}"
puts "D valiation = #{d_keys.size}"
puts "reduced D valiation = #{d_reduce_keys.size}"
File.open("result.txt","w"){|file|
  file.puts "###VECTOR #########################"
  vec_keys.each{|key,value|
    vecs = key.unpack("SSS")
    file.printf("%04x,%04x,%04x ... %d\n", vecs[0],vecs[1],vecs[2], value )
  }
  file.puts "###D #########################"
  d_keys.each{|key,value|
    file.printf( "%08x ... %d\n", key.unpack("I")[0], value )
  }
  file.puts "###D REDUCED #########################"
  d_reduce_keys.each{|key,value|
    file.printf("%08x ... %d\n", key, value )
  }
}

#p keys

