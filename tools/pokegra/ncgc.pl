
#====================================================================================
#
#	処理開始
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

		#パレットファイル生成
		$filename = basename( @ARGV[0], 'ncg' );
		$filename =~ s/pfwb/pmwb/g;
		$filename =~ s/c_m\.//g;
		$filename =~ s/c_f\.//g;
		$filename =~ s/(c_)(\w*)(_m)\./_$2/g;
		$filename =~ s/(c_)(\w*)(_f)\./_$2/g;
		$cmd = "rm " . $filename . "_n.NCLR";
		system $cmd;
		$cmd = "rm " . $filename . "_r.NCLR";
		system $cmd;

		#_fファイルが存在しない場合に生成
		$file_name = basename( @ARGV[0], 'ncg' );
		if( ( $file_name =~ m/c_m\./ ) || ( $file_name =~ m/c_\w*_m\./ ) ){
			$file_name =~ s/c_m\./c_f\./g;
		  $file_name =~ s/(c_)(\w*)(_m)\./c_$2_f\./g;
			$file_name = @ARGV[1] . $file_name . "NCBR";
			$size = -s $file_name;
			if( $size eq "" ){
				open( DUMMY, "> " . $file_name );
				close( DUMMY );
			}
		}
	}
	else{
		$file_name = basename( @ARGV[0], '.ncg' );
		$file_name = @ARGV[1] . $file_name . ".NCBR";
		open( DUMMY, "> " . $file_name );
		close( DUMMY );
	}

