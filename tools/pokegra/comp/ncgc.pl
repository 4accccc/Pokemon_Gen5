
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

		$file_name = basename( @ARGV[0], '.ncg' );
        &NCBR_Comp( $file_name );

		#パレットファイル生成
		$filename = basename( @ARGV[0], 'ncg' );
		$filename =~ s/pfwb/pmwb/g;
		$filename =~ s/pbwb/pmwb/g;
		$filename =~ s/[0-9]c_m\./$1/g;
		$filename =~ s/[0-9]c_f\./$1/g;
		$filename =~ s/(c_)(\w*)(_m)\./_$2/g;
		$filename =~ s/(c_)(\w*)(_f)\./_$2/g;
    print $filename;
		$cmd = "rm " . $filename . "_n.NCLR";
		system $cmd;
		$cmd = "rm " . $filename . "_r.NCLR";
		system $cmd;
		$cmd = "rm " . $filename . ".NCLR";
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


#圧縮
sub NCBR_Comp
{
	$r1 = @ARGV[1] . $_[0] . ".NCBR";
	$r2 = @ARGV[1] . $_[0] . ".NCBRs";
    rename $r1, $r2;
    $cmd = "ntrcomp -lex -o " . $r1 . " " . $r2;
    system $cmd;
    $cmd = "rm " . $r2;
    system $cmd;
}
