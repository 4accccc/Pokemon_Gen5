
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

	$cmd = "g2dcvtr @ARGV[0] -bg -cr/0_0_32_16 -o/@ARGV[1]";
	system $cmd;
	if( $? >> 8 != 0 ){
		$filename = basename( @ARGV[0] );
		$filename = basename( $filename, '.nmc' );
		$cmd = "rm " . $filename . ".NMCR";
		system $cmd;
        &NANR_Comp;
		exit(1);
	}
 	$filename = basename( @ARGV[0] );
	$filename = basename( $filename, '.nmc' );
	$cmd = "rm " . $filename . ".NCGR";
	system $cmd;

    &NANR_Comp;

sub NANR_Comp
{
	#リネーム＞圧縮＞削除
	$filename = basename( @ARGV[0] );
	$filename = basename( $filename, '.nmc' );
    $filename = @ARGV[1] . $filename;
    $r1 = $filename . ".NANR";
    $r2 = $filename . ".NANRs";
    rename $r1, $r2;
	$cmd = "ntrcomp -lex -o " . $r1 . " " . $r2;
	system $cmd;
	$cmd = "rm " . $r2;
	system $cmd;
}
