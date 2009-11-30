#=======================================================================
# d06stone.rb
# 電気洞窟に配置する浮遊石一覧cdatファイル作成
#=======================================================================
$KCODE = "SJIS"

#=======================================================================
# 定数
#=======================================================================

#=======================================================================
# エラー終了
#=======================================================================
def error_end( f0, f1, f2, del )
  f0.close
  f1.close
  f2.close
  File.delete( del )
end

#=======================================================================
# 変換
#=======================================================================
fpath_cdat = ARGV[0]
fpath_csv0 = ARGV[1]
fpath_csv1 = ARGV[2]
file_cdat = File.open( fpath_cdat, "w" )
file_csv0 = File.open( fpath_csv0, "r" )
file_csv1 = File.open( fpath_csv1, "r" )

file_cdat.printf( "//%s\n", fpath_cdat )
file_cdat.printf( "//D06浮遊石配置\n" )
file_cdat.printf( "//このファイルはコンバータから出力されました\n\n" )

#浮遊石大
buf = Array.new()
buf << "#define BIGSTONE_MAX (99)\n"
buf << "static const s16 data_d06BigStonePos[BIGSTONE_MAX][3] = {\n"

i = 0
line = file_csv0.gets #データ開始位置まで進める

while line = file_csv0.gets
  line = line.strip
  str = line.split( "," )
  
  if( str[1] != nil && str[1] != "" &&
      str[2] != nil && str[2] != "" &&
      str[3] != nil && str[3] != "" )
      buf << sprintf( "\t{%s,%s,%s},\n", str[1], str[2], str[3] )
  else
    break
  end
  
  i = i + 1
end

buf <<= "};\n"
buf[0] = sprintf( "#define BIGSTONE_MAX (%d)\n", i )

if( i == 0 )
  printf( "ERROR:D06 BIG STONE NOTHING\n" )
  error_end( file_cdat, file_csv0, file_csv1, fpath_cdat )
  exit 1
end

file_cdat.printf( "%s", buf );

#浮遊石小
buf = Array.new()
buf << "#define MINISTONE_MAX (99)\n"
buf << "static const s16 data_d06MiniStonePos[MINISTONE_MAX][3] = {\n"

i = 0
line = file_csv1.gets #データ開始位置まで進める

while line = file_csv1.gets
  line = line.strip
  str = line.split( "," )
  
  if( str[1] != nil && str[1] != "" &&
      str[2] != nil && str[2] != "" &&
      str[3] != nil && str[3] != "" )
      buf << sprintf( "\t{%s,%s,%s},\n", str[1], str[2], str[3] )
  else
    break
  end
  
  i = i + 1
end

buf <<= "};\n"
buf[0] = sprintf( "#define MINISTONE_MAX (%d)\n", i )

if( i == 0 )
  printf( "ERROR:D06 MINI STONE NOTHING\n" )
  error_end( file_cdat, file_csv0, file_csv1, fpath_cdat )
  exit 1
end

file_cdat.printf( "%s", buf );

file_cdat.close
file_csv0.close
file_csv1.close
