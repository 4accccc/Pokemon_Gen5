#=======================================================================
# rockpos.rb
# かいりきで汚水わが参照、保存する座標データ位置を作成
# land_list.rb land.csv..
# land.csv テキスト変換済みの地形データ管理表 複数指定可能
#=======================================================================
$KCODE = "SJIS"

#=======================================================================
# 定数
#=======================================================================

#=======================================================================
# 変換
#=======================================================================
fname_csv = ARGV[0]
fname_cdat = ARGV[1]
file_csv = File.open( fname_csv, "r" )
file_cdat = File.open( fname_cdat, "w" )

file_cdat.printf( "//%s\n", fname_cdat )
file_cdat.printf( "//このファイルはコンバータから出力されました\n\n" )
file_cdat.printf( "unsigned short DATA_MMDL_PushRockPosNum[][2] = {\n" )

line = file_csv.gets #データ開始位置まで進める
line = file_csv.gets
line = file_csv.gets

while line = file_csv.gets
  line = line.strip
  str = line.split( "," )
  
  if( str[0] != nil && str[1] != nil && str[2] != nil )
    file_cdat.printf( "  \{%s,%s\},\n", str[1], str[2] )
  end
end

file_cdat.printf( "  {0xffff,0xffff},\n" )
file_cdat.printf( "};" )
file_cdat.close
file_csv.close
