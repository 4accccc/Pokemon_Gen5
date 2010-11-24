#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#   @file		  mkdef.rb
#   @brief    動作モデルリストから、モデルタイプハッシュを作成する
#   @author		Masayuki Onoue
#   @data		2010.11.19
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$KCODE = "SJIS"
load "rbdefine.rb"

file_listcsv = File.open( ARGV[0] , "r" );
mkdef        = File.open( ARGV[1] , "w" );

header =<<EOS
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#   @file		  %s
#   @brief    このファイルはコンバーターから出力されたファイルです。
#   @author		Masayuki Onoue
#
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[

$anm_type = {
EOS
mkdef.printf( header , ARGV[1] );

file_listcsv.each_with_index do |line,index|
 
  # ==========================
  # 読み飛ばし処理
  if index <= 0
    next
  end

  # 読み込み終了
  if( line =~ /#END/ )
    break
  end

  # 分離
  str = line.split( "," )

  mkdef.printf("\"%s\"=>\"%s\",\n",str[RBDEF_NUM_BBD_CODE],str[RBDEF_NUM_BBD_ANMTYPE]);

end

mkdef.puts "}"

file_listcsv.close
mkdef.close
