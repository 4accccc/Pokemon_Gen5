#################################################################
#
#	フォグテーブル作成
#		conv.pl .fog
#
#
#################################################################


#引数チェック
if( @ARGV != 1 )
{
	print( "conv.pl .fog\n" );
	exit(1);
}


#読み込み
open( FILEIN, $ARGV[0] );
@filedata = <FILEIN>;
close( FILEIN );


#情報を出力
$outfile = $ARGV[0];
$outfile =~ s/\.fog//;
open( FILEOUT, ">".$outfile.".dat" );
binmode( FILEOUT );
print( FILEOUT pack("i", &getParam(\@filedata, "OFFSET") ) );
print( FILEOUT pack("i", &getParam(\@filedata, "SLOPE") ) );

close( FILEOUT );

exit(0);


sub getParam
{
	my( $data, $key ) = @_;
	my( $i, $onedata, $tmp );
	
	for( $i=0; $i<@$data; $i++ )
	{
		$onedata = $$data[$i];
		$onedata =~ s/\s//g;
		$onedata =~ s/\t//g;

		if( $onedata =~ /$key\:\:(.*)/ )
		{
			$tmp = $1;
			if( $tmp =~ /0x/ )	#16進数チェック
			{
				$tmp = oct($tmp);
			}
			return $tmp;
		}
	}

	print( "$key　が　ありません。" );
	exit(1);
}
