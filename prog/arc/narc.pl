#====================================================================================
#
#	アーカイブテーブル＆定数定義　生成コンバータ
#
#====================================================================================

use Switch;

	$argc = @ARGV;

	if( $argc < 4 ){
		print "error:perl perl_file read_file pm_version src_dir dest_dir\n";
		exit(1);
	}

	open( ARC_F, @ARGV[0] ) or die "[@ARGV[0]]", $!;
	@data = <ARC_F>;
	close( ARC_F );

	open( ARC_DEF, "> arc_def.h");
	open( ARC_FILE, "> arc_file.h");
	open( ARC_TREE, "> file_tree.inc");
	
	print ARC_DEF "#ifndef __ARC_DEF_H__\n";
	print ARC_DEF "#define __ARC_DEF_H__\n\n";
	print ARC_DEF "enum{\n";
	print ARC_FILE "static	const char *ArchiveFileTable[]={\n";
	print ARC_TREE "GAME_FILE_TREE=";

	$data_num	= @data;
	$num_1		= 0;
	$num_10		= 0;
	$num_100	= 0;
	$count		= 0;
	$write_count= 0;

	&MakeDir;

	while($data_num != 0){
		@arc_data		=	split(/ |\t|\r|\n/,@data[$count]);
		$arc_data_num	=	@arc_data;
		for( $i = 0 ; $i < $arc_data_num ; $i++ ){
			my $flag = 0;
			if( length @arc_data[$i] ){
				switch(substr(@arc_data[$i],0,1)){
					case '$' {
						$version = @arc_data[$i];
						$version =~ s/\$//g;
						$find = index( $version, @ARGV[1] );
						if( $find >= 0 && length $version == length @ARGV[1] ){
							&FileWrite($i+1);
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

	if( $write_count > 0 ){
		print ARC_TREE	" \\\n";
	}

	print ARC_DEF	"\t$enum,\n";
	print ARC_TREE	"\t$tree";
	print ARC_FILE	"\t\"$tree\",\t\t//$file\n";

	$num_1++;
	my $flag = FALSE;
	if( $num_1 > 9 ){
		$num_1 = 0;
		$num_10++;
		$flag = TRUE;
		if( $num_10 > 9 ){
			$num_10 = 0;
			$num_100 ++;
			if( $num_100 > 9 ){
				print "Error:File Max Over\n";
				exit(1);
			}
		}
	}
	if( $flag == TRUE ){
		&MakeDir;
	}

	my $src_file = @ARGV[2];
	$src_file .= $file;
	my $dst_file = @ARGV[3];
	$dst_file .= $tree;
	my @cmd = ( 'cp', $src_file, $dst_file );

	system @cmd;
	if( $? >> 8 != 0 ){
		exit(1);
	}

	$write_count++;
}

#===========================================================
#
#	ディレクトリ作成サブルーチン
#
#===========================================================
sub MakeDir{
	my $dir = "@ARGV[3]a";
	if( -d $dir){}
	else{
		mkdir ( $dir, 0777) || die "mkdir failed : $!";
	}
	$dir .= "/0";
	if( -d $dir){}
	else{
		mkdir ( $dir, 0777) || die "mkdir failed : $!";
	}
	$dir .= "/0";
	if( -d $dir){}
	else{
		mkdir ( $dir, 0777) || die "mkdir failed : $!";
	}
	$dir = "@ARGV[3]a/$num_100";
	if( -d $dir){}
	else{
		mkdir ( $dir, 0777) || die "mkdir failed : $!";
	}
	$dir .= "/$num_10";
	if( -d $dir){}
	else{
		mkdir ( $dir, 0777) || die "mkdir failed : $!";
	}
}

