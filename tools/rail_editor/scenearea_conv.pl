##-----------------------------------------------------------------------------
#	.saファイルをBinary情報にコンバート
#
#
#	scenearea_conv.pl	.sa fld_scenearea_loader_func.h fld_scenearea.h
#
#
#
#
#
#
##-----------------------------------------------------------------------------


##-----------------------------------------------------------------------------
#		定数
##-----------------------------------------------------------------------------
#各ワークサイズ 
$SA_WORK_SIZ;

#関数ID
$SA_FUNCID_NULL;

#関数ID
@SA_AREACHECK_FUNC_ID = undef;
@SA_UPDATE_FUNC_ID = undef;


#レール情報
@SA_NAME = undef;

#BYTEワークのindexオーバー定数
$BYTEWORK_INDEX_OVER	= 0xffff;

##-----------------------------------------------------------------------------
#	メイン処理
##-----------------------------------------------------------------------------
#引数チェック
if( @ARGV < 3 )
{
	print( "scenearea_conv.pl	.sa fld_scenearea_loader_func.h fld_scenearea.h \n" );
	exit(1);
}



#ローダーの関数IDの読み込み
&LoadSaFuncID( $ARGV[ 1 ] );

#レールシステム定数の読み込み
&loadSaSysData( $ARGV[ 2 ] );

#コンバート
&convData( $ARGV[ 0 ] );

exit(0);


##-----------------------------------------------------------------------------
#	ローダー関数IDの読み込み
##-----------------------------------------------------------------------------
sub LoadSaFuncID
{
	my( $filename ) = @_;
	my( $one, $index, $mode, $input, @data );

	open( FILEIN, $filename );
	@data = <FILEIN>;
	close( FILEIN );
	
	$input = 0;
	foreach $one ( @data )
	{
		#タブとスペース排除
		$one =~ s/\s//g;
		$one =~ s/\t//g;

		
		if( $one =~ /MAX/ )
		{
			# 終了
			$input = 0;
		}
		
		if( $input == 0 )
		{
			#定義開始チェック
			if( $one =~ /AREACHECK_FUNC_ID_START/ )
			{
				$mode = 0;
				$index = 0;
				$input = 1;
			}
			elsif( $one =~ /UPDATE_FUNC_ID_START/ )
			{
				$mode = 1;
				$index = 0;
				$input = 1;
			}
		}
		else
		{
			
			if( $mode == 0 )
			{
				if( $one =~ /([^,]*),/ )
				{
					$SA_AREACHECK_FUNC_ID[ $index ] = $1;
					$index ++;
				}
			}
			elsif( $mode == 1 )
			{
				if( $one =~ /([^,]*),/ )
				{
					$SA_UPDATE_FUNC_ID[ $index ] = $1;
					$index ++;
				}
			}
		}
	}

}

##-----------------------------------------------------------------------------
#	関数ID定数名　->	バイナリ
##-----------------------------------------------------------------------------
sub getCheckAreaFuncIDNameToNo
{
	my( $name ) = @_;
	my( $i );

	for( $i=0; $i<@SA_AREACHECK_FUNC_ID; $i++ )
	{
		if( "".$SA_AREACHECK_FUNC_ID[$i] eq "".$name )
		{
			return $i;
		}
	}

	print( "#getCheckAreaFuncIDNameToNosarch $name not found\n" );
	exit(1);
}

sub getUpdateFuncIDNameToNo
{
	my( $name ) = @_;
	my( $i );

	for( $i=0; $i<@SA_UPDATE_FUNC_ID; $i++ )
	{
		if( "".$SA_UPDATE_FUNC_ID[$i] eq "".$name )
		{
			return $i;
		}
	}

	print( "#getUpdateFuncIDNameToNo sarch $name not found\n" );
	exit(1);
}


##-----------------------------------------------------------------------------
#	レールシステムの定数読み込み
##-----------------------------------------------------------------------------
sub loadSaSysData
{
	my( $filename ) = @_;
	my( $one, $key_index, $input_count, @data );

	open( FILEIN, $filename );
	@data = <FILEIN>;
	close( FILEIN );

	$key_index = 0;
	$input_count = 0;
	foreach $one ( @data )
	{
		#タブとスペース排除
		$one =~ s/\s//g;
		$one =~ s/\t//g;

		if( $one =~ /FLD_SCENEAREA_SCENEPARAM_SIZE\(([0-9]*)\)/ )
		{
			$SA_WORK_SIZ = $1;
      #	print( "SA_WORK_SIZ load = $SA_WORK_SIZ\n" );
			$input_count ++;
		}

		if( $one =~ /FLD_SCENEAREA_FUNC_NULL\(([^)]*)\)/ )
		{
			$SA_FUNCID_NULL = $1;
      #		print( "SA_FUNCID_NULL load = $SA_FUNCID_NULL\n" );
			$input_count ++;
		}
	}

	#読み込み回数を調べる
	if( $input_count != 2 )
	{
		print( "#loadSaSysData load data not 2\n" );
		exit(1);
	}
}

##-----------------------------------------------------------------------------
#	　値に変換
##-----------------------------------------------------------------------------
sub get10Number
{
	my( $number ) = @_;

	if( "".$number eq "0" )
	{
		return 0;
	}

	if( $number =~ s/0x// )
	{
		return hex($number);
	}

	if( $number =~ s/^0// )
	{
		return oct($number);
	}

	return $number;
}

##-----------------------------------------------------------------------------
#	コンバート
##-----------------------------------------------------------------------------
sub convData
{
	my( $filename ) = @_;
	my( $i, $j, $one, @filedata, $outfile, $name_index );

	#情報読み込み
	open( FILEIN, $filename );
	@filedata = <FILEIN>;
	close( FILEIN );
	
	#まず、名前リストを作成
	$name_index = 0;
	for( $i=0; $i<(@filedata-1); $i++ )
	{
		$filedata[$i] =~ s/\t//g;
		$filedata[$i] =~ s/\s//g;
		$one	= $filedata[$i];
		
		if( $one =~ /\+SA$/ )
		{
			$SA_NAME[ $name_index ] = &getPeaceParam( \@filedata, $i+1, "NAME" );
			$name_index ++;
		}
	}
	$filedata[$i] =~ s/\t//g;
	$filedata[$i] =~ s/\s//g;

	
	#情報を数字に置換して、出力
	$outfile = $filename;
	$outfile =~ s/\.sa//;

	#オフセット情報
	open( FILEOUT, ">$outfile.dat" );
	binmode( FILEOUT );

	for( $i=0; $i<@SA_NAME; $i++ )
	{
    #print( $SA_NAME[$i]."\n" );
		#データ
		{
			my( $work_index, $work_number, $one_datasiz, $datasiz );
			$work_index		= 0;
			$datasiz			= 0;

			do
			{
				$work_number = &getPeaceParamByteWorkNumberForName( \@filedata, $SA_NAME[$i], $work_index );
				$one_datasiz = &getPeaceParamByteWorkSizeForName( \@filedata, $SA_NAME[$i], $work_index );

				if( ($work_number != $BYTEWORK_INDEX_OVER) && ($one_datasiz != $BYTEWORK_INDEX_OVER) )
				{
					#出力
					if( $one_datasiz == 1 )
					{
						print( FILEOUT pack( "c", &get10Number($work_number) ) );
					}
					elsif( $one_datasiz == 2 )
					{
						print( FILEOUT pack( "s", &get10Number($work_number) ) );
					}
					elsif( $one_datasiz == 4 )
					{
						print( FILEOUT pack( "i", &get10Number($work_number) ) );
					}
					$datasiz += $one_datasiz;

					#サイズオーバーチェック
					if( $datasiz > $SA_WORK_SIZ )
					{
						print( "camera_work size over  name=".$SA_NAME[$i]."\n" );
						exit( 1 );
					}
					$work_index ++;
				}
			}while( ($work_number != $BYTEWORK_INDEX_OVER) && ($one_datasiz != $BYTEWORK_INDEX_OVER) );

			#足りないワークを出力
			while( $datasiz < $SA_WORK_SIZ )
			{
				print( FILEOUT pack( "c", 0 ) );
				$datasiz ++;
			}
		}

		#各関数ID
		{
			my( $func_id );
			$func_id = &getPeaceParamForName( \@filedata, $SA_NAME[$i], "AREACHECK" );
			print( FILEOUT pack( "S", &getNameIndex( \@SA_AREACHECK_FUNC_ID, $func_id ) ) );

			$func_id = &getPeaceParamForName( \@filedata, $SA_NAME[$i], "UPDATE" );
			print( FILEOUT pack( "S", &getNameIndex( \@SA_UPDATE_FUNC_ID, $func_id ) ) );

			$func_id = &getPeaceParamForName( \@filedata, $SA_NAME[$i], "INSIDE" );
			print( FILEOUT pack( "S", &getNameIndex( \@SA_UPDATE_FUNC_ID, $func_id ) ) );

			$func_id = &getPeaceParamForName( \@filedata, $SA_NAME[$i], "OUTSIDE" );
			print( FILEOUT pack( "S", &getNameIndex( \@SA_UPDATE_FUNC_ID, $func_id ) ) );
		}
	}
	
	close( FILEOUT );
}

##-----------------------------------------------------------------------------
# 名前一致インデックスの取得
##-----------------------------------------------------------------------------
sub getNameIndex
{
	my( $data, $name ) = @_;
	my( $i, $roopnum );

	#NULLチェック
	if( $name =~ /FLD_SCENEAREA_FUNC_NULL/ )
	{
		return &get10Number($SA_FUNCID_NULL);
	}

	$roopnum = @$data;
	for( $i=0; $i<$roopnum; $i++ )
	{
		if( "".$$data[$i] eq "".$name )
		{
			return $i;
		}
	}

	print( "#getNameIndex $name not found\n" );
	exit(1);
}

##-----------------------------------------------------------------------------
# saデータ　パラメータ取得
##-----------------------------------------------------------------------------
sub getPeaceParam
{
	my( $data, $startidx, $key ) = @_;
	my( $i, $one );

	for( $i=$startidx; $i<@$data; $i++ )
	{
		$one = $$data[ $i ];
		$one =~ s/\"//g;
		$one =~ s/\r\n//g;
		$one =~ s/\n//g;

		#もし+が出てきたらPeaceオーバー
		if( $one =~ /\+/ )
		{
			print( "#getPeaceParam peace over key=$key startindex=$startindex\n" );
			exit(1);
		}
		
		if( $one =~ /--$key\:\:(.*)/ )
		{
			return $1;
		}
	}

	print( "#getPeaceParam $startidx , $key not found\n" );
	exit( 1 );
}

sub getPeaceParamForName
{
	my( $data, $name, $key ) = @_;
	my( $i, $one, $last_peace_index );

	$last_peace_index = 0;
	for( $i=0; $i<@$data; $i++ )
	{
		$one = $$data[ $i ];
		$one =~ s/\"//g;
		$one =~ s/\r\n//g;
		$one =~ s/\n//g;

		#もし+が出てきたらlast_peace_index保存
		if( $one =~ /\+/ )
		{
			$last_peace_index = $i;
		}
		
		if( $one =~ /--NAME\:\:$name$/ )
		{
			#last_peace_indexから、$keyの情報を取得する
			return &getPeaceParam( \@$data, $last_peace_index+1, $key );
		}
	}

	print( "#getPeaceParamForName $name not found\n" );
	exit( 1 );
}

sub getPeaceParamForTag
{
	my( $data, $tag, $key ) = @_;
	my( $i, $one );

	$last_peace_index = 0;
	for( $i=0; $i<@$data; $i++ )
	{
		$one = $$data[ $i ];
		$one =~ s/\"//g;
		$one =~ s/\r\n//g;
		$one =~ s/\n//g;

		#タグを見つける
		if( $one =~ /\+$tag$/ )
		{
			#$tagから、$keyの情報を取得する
			return &getPeaceParam( \@$data, $i+1, $key );
		}
	}

	print( "#getPeaceParamForTag $tag not found\n" );
	exit( 1 );

}


#バイトワークの値を取得
sub getPeaceParamByteWorkNumberForIndex
{
	my( $data, $startidx, $index ) = @_;
	my( $i, $one, $index_count );

	$index_count = 0;
	for( $i=$startidx; $i<@$data; $i++ )
	{
		$one = $$data[ $i ];
		$one =~ s/\"//g;
		$one =~ s/\r\n//g;
		$one =~ s/\n//g;

		#もし+が出てきたらPeaceオーバー
		if( $one =~ /\+/ )
		{
			return $BYTEWORK_INDEX_OVER;
		}
		
		if( $one =~ /--[1-4]BYTE\:\:(.*)/ )
		{
			if($index_count == $index)
			{
				return $1;
			}
			$index_count ++;
		}
	}
	return $BYTEWORK_INDEX_OVER;
}

sub getPeaceParamByteWorkNumberForName
{
	my( $data, $name, $index ) = @_;
	my( $i, $one, $last_peace_index );

	$last_peace_index = 0;
	for( $i=0; $i<@$data; $i++ )
	{
		$one = $$data[ $i ];
		$one =~ s/\"//g;
		$one =~ s/\r\n//g;
		$one =~ s/\n//g;

		#もし+が出てきたらPeaceオーバー
		if( $one =~ /\+/ )
		{
			$last_peace_index = $i;
		}

		if( $one =~ /--NAME\:\:$name$/ )
		{
			#last_peace_indexから、$index番目のBYTEパラメータを取得する
			return &getPeaceParamByteWorkNumberForIndex( \@$data, $last_peace_index+1, $index );
		}
		
	}

	print( "#getPeaceParamByteWorkNumberForName $name not found\n" );
	exit( 1 );
}



#バイトワークのBYTEサイズを取得
sub getPeaceParamByteWorkSizeForIndex
{
	my( $data, $startidx, $index ) = @_;
	my( $i, $one, $index_count );

	$index_count = 0;
	for( $i=$startidx; $i<@$data; $i++ )
	{
		$one = $$data[ $i ];
		$one =~ s/\"//g;
		$one =~ s/\r\n//g;
		$one =~ s/\n//g;

		#もし+が出てきたらPeaceオーバー
		if( $one =~ /\+/ )
		{
			return $BYTEWORK_INDEX_OVER;
		}
		
		if( $one =~ /--([1-4])BYTE/ )
		{
			if($index_count == $index)
			{
				return $1;
			}
			$index_count ++;
		}
	}

	return $BYTEWORK_INDEX_OVER;
}

sub getPeaceParamByteWorkSizeForName
{
	my( $data, $name, $index ) = @_;
	my( $i, $one, $last_peace_index );

	$last_peace_index = 0;
	for( $i=0; $i<@$data; $i++ )
	{
		$one = $$data[ $i ];
		$one =~ s/\"//g;
		$one =~ s/\r\n//g;
		$one =~ s/\n//g;

		#もし+が出てきたらPeaceオーバー
		if( $one =~ /\+/ )
		{
			$last_peace_index = $i;
		}

		if( $one =~ /--NAME\:\:$name$/ )
		{
			#last_peace_indexから、$index番目のBYTEパラメータを取得する
			return &getPeaceParamByteWorkSizeForIndex( \@$data, $last_peace_index+1, $index );
		}
		
	}

	print( "#getPeaceParamByteWorkSizeForName $name not found\n" );
	exit( 1 );
}







