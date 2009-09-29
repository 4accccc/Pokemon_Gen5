#====================================================================================
#
#	アーカイブ定数定義　生成コンバータ
#
#====================================================================================

use Switch;
use	File::Basename;

use constant ARC_LIST	=> 0;
use constant VERSION	=> 1;
use constant DEBUG	  => 2;
use constant MB_FLAG	=> 3;

	$argc = @ARGV;

	if( $argc < 4 ){
		print "error:perl perl_file read_file pm_version pm_debug multi_boot_flag\n";
		print " read_file:\n  読み込むリストファイル（通常は、arc_tool.lst）\n";
		print " pm_version:\n  バージョンフラグ（ポケモンなどで２バージョンある時に作成バージョンを指定）\n";
		print " multi_boot_flag:\n  yesを指定することで、マルチブート子機対応になる\n";
		exit(1);
	}

	open( ARC_F, @ARGV[ARC_LIST] ) or die "[@ARGV[ARC_LIST]]", $!;
	@data = <ARC_F>;
	close( ARC_F );

	open( ARC_DEF, "> arc_def.h");
	open( ARC_FILE, "> arc_file.h");
	
	print ARC_DEF "#ifndef __ARC_DEF_H__\n";
	print ARC_DEF "#define __ARC_DEF_H__\n\n";
	print ARC_DEF "enum{\n";

#マルチブート対応テーブル作成
	if( @ARGV[MB_FLAG] =~ m/^yes/i ){
#		print ARC_FILE "static	const u8 *ArchiveFileTable[]={\n";
		open( ARC_EXTERN, "> arc_extern.h");
		print ARC_FILE "#include \"arc_extern.h\"\n";
	}

	print ARC_FILE "static	const char *ArchiveFileTable[]={\n";

	$data_num	= @data;
	$num_1		= 0;
	$num_10		= 0;
	$num_100	= 0;
	$count		= 0;

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
            if( $version eq "DEBUG_YES" && @ARGV[DEBUG] eq "yes" )
            {
  						&FileWrite($i+1);
            }
            elsif( $version eq "DEBUG_NO" && @ARGV[DEBUG] eq "no" )
            {
  						&FileWrite($i+1);
            }
            else
            {
  						$find = index( $version, @ARGV[VERSION] );
  						if( $find >= 0 && length $version == length @ARGV[VERSION] ){
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

	print ARC_DEF "\tARCID_TABLE_MAX\n};\n";
	print ARC_DEF "\n#endif __ARC_DEF_H__\n";
	print ARC_FILE "};\n";
	close( ARC_DEF );
	close( ARC_FILE );
	if( @ARGV[MB_FLAG] =~ m/^yes/i ){
		close( ARC_EXTERN );
	}

#===========================================================
#
#	ファイル書き出しサブルーチン
#
#	引数：@_[0]　ラベルが格納された配列の添え字
#
#===========================================================
sub FileWrite{
	my $index = @_[0];
	my $enum = @arc_data[$index];
	my $tree = "a/$num_100/$num_10/$num_1";
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

	print ARC_DEF	"\t$enum,\n";

#マルチブート対応テーブル作成
	if( @ARGV[MB_FLAG] =~ m/^yes/i ){
		my @extlist = ('\.narc');
		( $name, $dir, $ext ) = fileparse( $file, @extlist );
		print ARC_FILE	"\t\&_start_$name,\t&_end_$name,\n";
		print ARC_EXTERN "extern\tchar\t_start_$name;\n";
		print ARC_EXTERN "extern\tchar\t_end_$name;\n";
	}
	else{
		print ARC_FILE	"\t\"$tree\",\t\t//$file\n";
	}

	$num_1++;
	if( $num_1 > 9 ){
		$num_1 = 0;
		$num_10++;
		if( $num_10 > 9 ){
			$num_10 = 0;
			$num_100 ++;
			if( $num_100 > 9 ){
				print "Error:File Max Over\n";
				exit(1);
			}
		}
	}
}

