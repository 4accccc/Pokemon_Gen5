
#====================================================================================
#
#	èàóùäJén
#
#====================================================================================
use File::Basename;

	$argc = @ARGV;

	if( $argc < 2 ){
		print "error:perl perl_file read_file write_dir\n";
		die;
	}

	$file_name = @ARGV[0];

	$size = -s $file_name;

	if( $size ){
		$cmd = "g2dcvtr @ARGV[0] -bmp -bg -cr/0_0_32_16 -o/@ARGV[1]";
		system $cmd;
	}
	else{
		$file_name = basename( @ARGV[0], '.ncg' );
		$file_name = @ARGV[1] . $file_name . ".NCBR";
		open( DUMMY, "> " . $file_name );
		close( DUMMY );
	}

