##-----------------------------------------------------------------------------
#
#
#		タマ投げデータコンバータ
#
#
##-----------------------------------------------------------------------------

##-----------------------------------------------------------------------------
#	ファイルポインタ
##-----------------------------------------------------------------------------
@FILE_IN	= undef;

##-----------------------------------------------------------------------------
#	データ参照定数
##-----------------------------------------------------------------------------
$FILE_DATA_NUM	= 15;		#データ総数
$FILE_START_IDX	= 3;		#データ開始行数
$FILE_TYPE_ST	= 1;		#データタイプ開始列数
$FILE_TYPE_NUM	= 6;		#データタイプ数




#まずはエクセルコンバート
#コンバート後のファイルを読み込み
system( "../../pokemon_pl/convert/exceltool/ExcelSeetConv.exe -o bucket_data.txt -p 0 -s csv bucket_data.xls" );
open( FILEIN, "bucket_data.txt" );
@FILE_IN = <FILEIN>;
close( FILEIN );


#書き出しようのファイルをオープン
#バイナリモードに設定
open( FIELOUT, ">bucket_data.bin" );
binmode( FIELOUT );


#データの書き出し
for( $i=0; $i<$FILE_DATA_NUM; $i++ ){
	@file_data = split( /,/, $FILE_IN[ $i+$FILE_START_IDX ] );
	for( $j=0; $j<$FILE_TYPE_NUM; $j++ ){

		if( $file_data[ $j+$FILE_TYPE_ST ] eq "MAX" ){
			print( FIELOUT pack("I", 100000 ) );
		}else{
			print( FIELOUT pack("I", $file_data[ $j+$FILE_TYPE_ST ] ) );
		}
	}
}

#終了
close( FIELOUT );

#生成物を破棄
system( "rm bucket_data.txt" );

exit(0);
