#====================================================================================
#
#	NARCタイムスタンプ比較用ファイルツリー生成
#
#====================================================================================

use Switch;

	$argc = @ARGV;

	if( $argc < 3 ){
		print "error:perl perl_file read_file version debug\n";
		exit(1);
	}

	open( ARC_F, @ARGV[0] ) or die "[@ARGV[0]]", $!;
	@data = <ARC_F>;
	close( ARC_F );

	open( ARC_TREE, "> narc_tree.inc");
	
	print ARC_TREE "NARC_TREE= \\\n";

	$data_num	= @data;
	$write_count= 0;

	while($data_num != 0){
		@arc_data		=	split(/ +|\t|\r|\n/,@data[$count]);
		$arc_data_num	=	@arc_data;
		for( $i = 0 ; $i < $arc_data_num ; $i++ ){
			my $flag = 0;
			if( length @arc_data[$i] ){
				switch(substr(@arc_data[$i],0,1)){
					case '$' {
						$version = @arc_data[$i];
						$version =~ s/\$//g;
            if( $version eq "DEBUG_YES" && @ARGV[2] eq "yes" )
            {
  						&FileWrite($i+1);
            }
            elsif( $version eq "DEBUG_NO" && @ARGV[2] eq "no" )
            {
  						&FileWrite($i+1);
            }
            else
            {
  						$find = index( $version, @ARGV[1] );
  						if( $find >= 0 && length $version == length @ARGV[1] ){
  							&FileWrite($i+1);
  						}
            }
						$flag = 1;
					}
					case '#' {
						$flag = 1;
					}
					else     {
						&FileWrite($i);
						$flag = 1;
					}
				}
				if( $flag == 1 ){
					last;
				}
			}
		}
		$count++;
		$data_num--;
	}

	close( ARC_TREE );

#===========================================================
#
#	ファイル書き出しサブルーチン
#
#	引数：@_[0]　ラベルが格納された配列の添え字
#
#===========================================================
sub FileWrite{
	my $index = @_[0];
	my $file_num = $index + 1;
	while( length @arc_data[$file_num] == 0 ){
		$file_num++;
		if( $file_num >= $arc_data_num ){
			print "Error:Not Found FileName\n";
			exit(1);
		}
	}
	my $file = @arc_data[$file_num];
	$file =~ s/"//g;
	if( substr( $file, 0, 1 ) eq '#' ){
		print "Error:Not Found FileName $file\n";
		exit(1);
	}

	if( $write_count > 0 ){
		print ARC_TREE	" \\\n";
	}

	print ARC_TREE	"\t$file";

	$write_count++;
}

