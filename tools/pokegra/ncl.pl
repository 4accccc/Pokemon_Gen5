
#====================================================================================
#
#	処理開始
#
#====================================================================================
use File::Basename;

use constant NCL_HEAD_SIZE	=>	4 + 2 + 2 + 4 + 2 + 2 + 4 + 4 + 4 + 4;

	@NCLRHead = ( 0x52, 0x4c, 0x43, 0x4e, 0xff, 0xfe, 0x00, 0x01,
				  0x48, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01, 0x00,
				  0x54, 0x54, 0x4c, 0x50, 0x38, 0x00, 0x00, 0x00,
				  0x04, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00,
				  0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00 );

	$argc = @ARGV;

	if( $argc < 2 ){
		print "error:perl perl_file read_file write_dir\n";
		die;
	}

	&NCLRMake;

#====================================================================================
#
#	NCLRMake
#
#====================================================================================
sub NCLRMake
{
	$file_name = basename( @ARGV[0], '.ncl' );
	$file_name = @ARGV[1] . $file_name . ".NCLR";
	open( NCL, @ARGV[0] );
	open( NCLR, "> " . $file_name );
	binmode( NCL );
	binmode( NCLR );

	for( $i = 0 ; $i < @NCLRHead ; $i++ ){
		$write = pack "C", @NCLRHead[ $i ];
		print NCLR $write;
	}

	#ヘッダーデータを読み込み
	read NCL, $header, NCL_HEAD_SIZE; 

	read NCL, $data, 0x20; 
	print NCLR $data;

	close( NCL );
	close( NCLR );

	#レアファイルが無かったらノーマルをコピーする
	$rare_name = @ARGV[0];
	$rare_name =~ s/_n/_r/g;
	$size = -s $rare_name;

	if( $size eq "" )
	{
		$file_name = basename( @ARGV[0], '.ncl' );
		$file_name = @ARGV[1] . $file_name . ".NCLR";
		$rare_name = $file_name;
		$rare_name =~ s/_n/_r/g;
		open( NCLR_N, $file_name );
		open( NCLR_R, "> " . $rare_name );
		binmode( NCLR_N );
		binmode( NCLR_R );
		read NCLR_N, $data, 72;
		print NCLR_R $data;
		close( NCLR_N );
		close( NCLR_R );
	}
}
