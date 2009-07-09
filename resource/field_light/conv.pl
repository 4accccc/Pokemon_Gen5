################################################
#
#	CSV形式のライトデータをBinaryにして出力
#
#	conv.pl	csvfile
#
################################################


#引数をチェック
if( @ARGV < 2 ){
	print( "need 2 param\n" );
	print( "conv.pl	csvfile outputfile\n" );
	exit(1);
}


#ファイル読み込み
open( FILE, $ARGV[0] );
@CSVFILE = <FILE>;
close( FILE );


$FRAME_NUM		= 0;		#フレーム数
@FRAME			= undef;	#フレーム

@LIGHT00_FLAG	= undef;	#ライト使用フラグ
@LIGHT01_FLAG	= undef;	#ライト使用フラグ
@LIGHT02_FLAG	= undef;	#ライト使用フラグ
@LIGHT03_FLAG	= undef;	#ライト使用フラグ
@LIGHT00_X		= undef;	#ライトX
@LIGHT01_X		= undef;	#ライトX
@LIGHT02_X		= undef;	#ライトX
@LIGHT03_X		= undef;	#ライトX
@LIGHT00_Y		= undef;	#ライトY
@LIGHT01_Y		= undef;	#ライトY
@LIGHT02_Y		= undef;	#ライトY
@LIGHT03_Y		= undef;	#ライトY
@LIGHT00_Z		= undef;	#ライトZ
@LIGHT01_Z		= undef;	#ライトZ
@LIGHT02_Z		= undef;	#ライトZ
@LIGHT03_Z		= undef;	#ライトZ
@LIGHT00_R		= undef;	#ライトR
@LIGHT01_R		= undef;	#ライトR
@LIGHT02_R		= undef;	#ライトR
@LIGHT03_R		= undef;	#ライトR
@LIGHT00_G		= undef;	#ライトG
@LIGHT01_G		= undef;	#ライトG
@LIGHT02_G		= undef;	#ライトG
@LIGHT03_G		= undef;	#ライトG
@LIGHT00_B		= undef;	#ライトB
@LIGHT01_B		= undef;	#ライトB
@LIGHT02_B		= undef;	#ライトB
@LIGHT03_B		= undef;	#ライトB

@DIFFUSE_R		= undef;	#ディフューズ
@DIFFUSE_G		= undef;	#ディフューズ
@DIFFUSE_B		= undef;	#ディフューズ
@AMBIENT_R		= undef;	#アンビエント
@AMBIENT_G		= undef;	#アンビエント
@AMBIENT_B		= undef;	#アンビエント
@SPECULAR_R		= undef;	#スペキュラー
@SPECULAR_G		= undef;	#スペキュラー
@SPECULAR_B		= undef;	#スペキュラー
@EMISSION_R		= undef;	#エミッション
@EMISSION_G		= undef;	#エミッション
@EMISSION_B		= undef;	#エミッション
@FOG_R			= undef;	#フォグカラー
@FOG_G			= undef;	#フォグカラー
@FOG_B			= undef;	#フォグカラー
@BG_R			= undef;	#背面カラー
@BG_G			= undef;	#背面カラー
@BG_B			= undef;	#背面カラー


#データ取得
$inputdata = 0;
$datacount = 0;
$i = 0;
foreach $one ( @CSVFILE ){
	@line = split( /,/, $one );
	if( $inputdata == 0 ){
		if( $line[ 0 ] eq "frame" ){
			$inputdata = 1;

			#データ数調査
			while( $line[ $FRAME_NUM+1 ] =~ /[0-9]/ ){
				$FRAME[ $FRAME_NUM ] = $line[ $FRAME_NUM+1 ];
				$FRAME_NUM++;
			}
		}
	}else{

		if( $datacount == 0 ){
			
			for( $i=0; $i<$FRAME_NUM; $i++ ){
				$LIGHT00_FLAG[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 1 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				$LIGHT00_X[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 2 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT00_Y[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 3 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT00_Z[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 4 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT00_R[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 5 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT00_G[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 6 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT00_B[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 7 ){
			
			for( $i=0; $i<$FRAME_NUM; $i++ ){
				$LIGHT01_FLAG[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 8 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				$LIGHT01_X[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 9 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT01_Y[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 10 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT01_Z[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 11 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT01_R[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 12 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT01_G[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 13 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT01_B[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 14 ){
			
			for( $i=0; $i<$FRAME_NUM; $i++ ){
				$LIGHT02_FLAG[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 15 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				$LIGHT02_X[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 16 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT02_Y[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 17 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT02_Z[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 18 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT02_R[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 19 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT02_G[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 20 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT02_B[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 21 ){
			
			for( $i=0; $i<$FRAME_NUM; $i++ ){
				$LIGHT03_FLAG[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 22 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				$LIGHT03_X[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 23 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT03_Y[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 24 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT03_Z[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 25 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT03_R[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 26 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT03_G[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 27 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@LIGHT03_B[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 28 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@DIFFUSE_R[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 29 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@DIFFUSE_G[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 30 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@DIFFUSE_B[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 31 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@AMBIENT_R[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 32 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@AMBIENT_G[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 33 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@AMBIENT_B[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 34 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@SPECULAR_R[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 35 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@SPECULAR_G[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 36 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@SPECULAR_B[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 37 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@EMISSION_R[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 38 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@EMISSION_G[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 39 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@EMISSION_B[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 40 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@FOG_R[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 41 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@FOG_G[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 42 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@FOG_B[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 43 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@BG_R[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 44 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@BG_G[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 45 ){

			for( $i=0; $i<$FRAME_NUM; $i++ ){
				@BG_B[ $i ] = $line[ $i+1 ]
			}
		}
		elsif( $datacount == 46 ){
			#終了
			$inputdata = 0;
		}

		$datacount++;
	}
}


#出力
open( FILE, ">".$ARGV[1] );
binmode( FILE );

for( $i=0; $i<$FRAME_NUM; $i++ ){
	#各フレームごとでまとめる
	print( FILE pack( "I", $FRAME[$i] ) );
	
	print( FILE pack( "C", $LIGHT00_FLAG[$i] ) );
	print( FILE pack( "C", $LIGHT01_FLAG[$i] ) );
	print( FILE pack( "C", $LIGHT02_FLAG[$i] ) );
	print( FILE pack( "C", $LIGHT03_FLAG[$i] ) );

	print( FILE pack( "S", &CONV_GetRgb( $LIGHT00_R[$i], $LIGHT00_G[$i], $LIGHT00_B[$i] ) ) );
	print( FILE pack( "S", &CONV_GetRgb( $LIGHT01_R[$i], $LIGHT01_G[$i], $LIGHT01_B[$i] ) ) );
	print( FILE pack( "S", &CONV_GetRgb( $LIGHT02_R[$i], $LIGHT02_G[$i], $LIGHT02_B[$i] ) ) );
	print( FILE pack( "S", &CONV_GetRgb( $LIGHT03_R[$i], $LIGHT03_G[$i], $LIGHT03_B[$i] ) ) );

	print( FILE pack( "S", $LIGHT00_X[$i]) );
	print( FILE pack( "S", $LIGHT00_Y[$i]) );
	print( FILE pack( "S", $LIGHT00_Z[$i]) );
	print( FILE pack( "S", $LIGHT01_X[$i]) );
	print( FILE pack( "S", $LIGHT01_Y[$i]) );
	print( FILE pack( "S", $LIGHT01_Z[$i]) );
	print( FILE pack( "S", $LIGHT02_X[$i]) );
	print( FILE pack( "S", $LIGHT02_Y[$i]) );
	print( FILE pack( "S", $LIGHT02_Z[$i]) );
	print( FILE pack( "S", $LIGHT03_X[$i]) );
	print( FILE pack( "S", $LIGHT03_Y[$i]) );
	print( FILE pack( "S", $LIGHT03_Z[$i]) );


	print( FILE pack( "S", &CONV_GetRgb( $DIFFUSE_R[$i], $DIFFUSE_G[$i], $DIFFUSE_B[$i] ) ) );
	print( FILE pack( "S", &CONV_GetRgb( $AMBIENT_R[$i], $AMBIENT_G[$i], $AMBIENT_B[$i] ) ) );
	print( FILE pack( "S", &CONV_GetRgb( $SPECULAR_R[$i], $SPECULAR_G[$i], $SPECULAR_B[$i] ) ) );
	print( FILE pack( "S", &CONV_GetRgb( $EMISSION_R[$i], $EMISSION_G[$i], $EMISSION_B[$i] ) ) );
	print( FILE pack( "S", &CONV_GetRgb( $FOG_R[$i], $FOG_G[$i], $FOG_B[$i] ) ) );
	print( FILE pack( "S", &CONV_GetRgb( $BG_R[$i], $BG_G[$i], $BG_B[$i] ) ) );

}

close( FILE );


#正常終了
exit(0);


sub CONV_GetRgb
{
	my( $r, $g, $b ) = @_;
	my( $ans );
	
	$ans =  $r<<0;
	$ans |= $g<<5;
	$ans |= $b<<10;

	return $ans;
}
