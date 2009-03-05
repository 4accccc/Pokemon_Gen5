########################################################
#
#
#		ディレクトリ内にあるimdのノードを列挙します。
#
#
########################################################
@FILELIST	= undef;			#処理するファイルのリスト



#imdファイルのリストを作成
@FILELIST = glob( "*.imd" );

foreach $one ( @FILELIST ){
	&out_put( $one );
}

exit(0);


#NODEのリストを作成する
sub out_put(){
	my( $filename ) = @_;
	my( @infile, @nodearry, $fileoutname, $filename_cut, $i );

	#ファイル読み込み
	open( FILEIN, $filename );
	@infile = <FILEIN>;
	close( FILEIN );

	foreach $data ( @infile ){
		if( $data =~ /node index=\"([0-9]*)\" name=\"([^\"]*)\"/ ){
			$nodearry[ $1 ] = $2;
		}
	}

	#ファイルに書き出す
	$fileoutname = $filename;
	$fileoutname =~ s/imd/h/g;
	$filename_cut = $filename;
	$filename_cut =~ s/.imd//;
	open( FILEOUT, ">$fileoutname" );
	print( FILEOUT "// $filenameのノードデータ\n" );
	print( FILEOUT "#ifndef NODE_IDX_$filename_cut\n" );
	print( FILEOUT "#define NODE_IDX_$filename_cut\n\n\n" );

	for( $i=0; $i<@nodearry; $i++ ){
		print( FILEOUT "#define ".$nodearry[$i]."		($i)\n" );
	}

	print( FILEOUT "#define ".$filename_cut."_NODEIDXMAX		($i)\n" );

	print( FILEOUT "\n\n#endif // NODE_IDX_$filename_cut\n" );
	close( FILEOUT );
}
