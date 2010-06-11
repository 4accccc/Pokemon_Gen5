#==========================================================
#	ミュージカル・アイテムデータ変換Ruby
#	
#===========================================================

$KCODE = "SJIS"

#============================
#	define
#============================

#============================
#	proto
#============================

#============================
#	main
#============================

dataFileName = ARGV[0]
outFileName = ARGV[1]
if( dataFileName == nil )
	printf( "data file is not found!!\n" )
	exit 1
end
if( outFileName == nil )
	printf( "output file is not found!!\n" )
	exit 1
end

printf("datafile   [%s]\n",dataFileName);
printf("outputfile [%s]\n",outFileName);

dataFile = File.open( dataFileName, "r" );
outFile = File.open( outFileName, "wb" );


outFile.write( "//----------------------------------------------\n" );
outFile.write( "//このファイルはform_conv.rbから出力されました。\n" );
outFile.write( "//フィールドで存在しないフォルムチェック\n" );
outFile.write( "//----------------------------------------------\n" );
outFile.write( "\n" );
outFile.write( "static const u16 fieldNgFormArr[] = \n" );
outFile.write( "{\n" );

#ファイルの解釈開始

line = dataFile.gets #先頭行はラベルなので抜かす

no = 1
#パラメタコンバート
while line = dataFile.gets
	
	str = line.split( "\t" )
	if( str[2].to_i != 0 )
  	printf("[%s]",str[0])
  	printf("[%s]\n",str[1])
    outFile.write( "\t" + str[0] + ", //" + str[1] + "\n" );
	end
=begin
	printf("[%s]",str[0])
	printf("[%s]",str[1])
	printf("[%s]",str[2])
	printf("\n")
=end
end

#ファイルの解釈終了
outFile.write( "};\n" );

dataFile.close
outFile.close
