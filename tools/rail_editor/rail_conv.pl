##-----------------------------------------------------------------------------
#	.railファイルをBinary情報にコンバート
#
#
#	rail_conv.pl	.rail field_rail_loader_func.h field_rail.h
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
$RAIL_CAMERA_SET_WORK_SIZ;
$RAIL_LINEPOS_SET_WORK_SIZ;

#KEY
@RAIL_KEY = undef;

#レールテーブルNULL
$RAIL_TBL_NULL;

#NAMEBUF
$RAIL_NAMEBUF_SIZ;


#関数ID
@RAIL_CAMERA_FUNC_ID = undef;
@RAIL_LINEPOS_FUNC_ID = undef;
@RAIL_LINEDIST_FUNC_ID = undef;


#レール情報
@POINT_NAME = undef;
@LINE_NAME	= undef;
@LINEPOS_NAME = undef;
@CAMERA_NAME = undef;

#CAMERA_SET LINEPOS_SET	BYTEワークのindexオーバー定数
$BYTEWORK_INDEX_OVER	= 0xffff;

##-----------------------------------------------------------------------------
#	メイン処理
##-----------------------------------------------------------------------------
#引数チェック
if( @ARGV < 3 )
{
	print( "rail_conv.pl	.rail field_rail_loader_func.h field_rail.h \n" );
	exit(1);
}



#ローダーの関数IDの読み込み
&LoadRailFuncID( $ARGV[ 1 ] );

#レールシステム定数の読み込み
&loadRailSysData( $ARGV[ 2 ] );

#コンバート
&convData( $ARGV[ 0 ] );

exit(0);


##-----------------------------------------------------------------------------
#	ローダー関数IDの読み込み
##-----------------------------------------------------------------------------
sub LoadRailFuncID
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
			if( $one =~ /CAMERA_FUNC_ID_START/ )
			{
				$mode = 0;
				$index = 0;
				$input = 1;
			}
			elsif( $one =~ /LINEPOS_FUNC_ID_START/ )
			{
				$mode = 1;
				$index = 0;
				$input = 1;
			}
			elsif( $one =~ /LINEDIST_FUNC_ID_START/ )
			{
				$mode = 2;
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
					$RAIL_CAMERA_FUNC_ID[ $index ] = $1;
					$index ++;
				}
			}
			elsif( $mode == 1 )
			{
				if( $one =~ /([^,]*),/ )
				{
					$RAIL_LINEPOS_FUNC_ID[ $index ] = $1;
					$index ++;
				}
			}
			elsif( $mode == 2 )
			{
				if( $one =~ /([^,]*),/ )
				{
					$RAIL_LINEDIST_FUNC_ID[ $index ] = $1;
					$index ++;
				}
			}
		}
	}

}

##-----------------------------------------------------------------------------
#	関数ID定数名　->	バイナリ
##-----------------------------------------------------------------------------
sub getCameraFuncIDNameToNo
{
	my( $name ) = @_;
	my( $i );

	for( $i=0; $i<@RAIL_CAMERA_FUNC_ID; $i++ )
	{
		if( $RAIL_CAMERA_FUNC_ID[$i] =~ /$name/ )
		{
			return $i;
		}
	}

	print( "#getCameraFuncIDNameToNo sarch $name not found\n" );
	exit(1);
}

sub getLinePosFuncIDNameToNo
{
	my( $name ) = @_;
	my( $i );

	for( $i=0; $i<@RAIL_LINEPOS_FUNC_ID; $i++ )
	{
		if( $RAIL_LINEPOS_FUNC_ID[$i] =~ /$name/ )
		{
			return $i;
		}
	}

	print( "#getLinePosFuncIDNameToNo sarch $name not found\n" );
	exit(1);
}

sub getLineDistFuncIDNameToNo
{
	my( $name ) = @_;
	my( $i );

	for( $i=0; $i<@RAIL_LINEDIST_FUNC_ID; $i++ )
	{
		if( $RAIL_LINEDIST_FUNC_ID[$i] =~ /$name/ )
		{
			return $i;
		}
	}

	print( "#getLineDistFuncIDNameToNo sarch $name not found\n" );
	exit(1);
}


##-----------------------------------------------------------------------------
#	レールシステムの定数読み込み
##-----------------------------------------------------------------------------
sub loadRailSysData
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

		if( $one =~ /RAIL_CAMERA_SET_WORK_SIZ\(([0-9]*)\)/ )
		{
			$RAIL_CAMERA_SET_WORK_SIZ = $1;
      #print( "RAIL_CAMERA_SET_WORK_SIZ load = $RAIL_CAMERA_SET_WORK_SIZ\n" );
			$input_count ++;
		}

		if( $one =~ /RAIL_LINEPOS_SET_WORK_SIZ\(([0-9]*)\)/ )
		{
			$RAIL_LINEPOS_SET_WORK_SIZ = $1;
      #print( "RAIL_LINEPOS_SET_WORK_SIZ load = $RAIL_LINEPOS_SET_WORK_SIZ\n" );
			$input_count ++;
		}

		if( $one =~ /RAIL_TBL_NULL\(([^\)]*)\)/ )
		{
			$RAIL_TBL_NULL = $1;
      #print( "RAIL_TBL_NULL load = $RAIL_TBL_NULL\n" );
			$input_count ++;
		}

		if( $one =~ /RAIL_NAME_BUF_SIZ\(([0-9]*)\)/ )
		{
			$RAIL_NAMEBUF_SIZ = $1;
      #print( "RAIL_NAME_BUF_SIZ load = $RAIL_NAMEBUF_SIZ\n" );
			$input_count ++;
		}

		#キーの定数
		if( $one =~ /RAIL_KEY_[^,]*,/ )
		{
			if( $one =~ /MAX/ )
			{
			}
			else
			{
				#定数を格納
				$one =~ /(RAIL_KEY_[^,=]*)[,=]/;
				$RAIL_KEY[ $key_index ] = $1;
        #print( $RAIL_KEY[ $key_index ]." load\n" );
				$key_index++;
				$input_count ++;
			}
		}
	}

	#読み込み回数を調べる
	if( $input_count != 9 )
	{
		print( "#loadRailSysData load data not 9 --- $input_count\n" );
		exit(1);
	}
}

##-----------------------------------------------------------------------------
#	キーの定義名 ->　値に変換
##-----------------------------------------------------------------------------
sub getRAIL_KeyNameToNo
{
	my( $keyname ) = @_;
	my( $i );

	for( $i=0; $i<@RAIL_KEY; $i++ )
	{
		if( $RAIL_KEY[$i] =~ /$keyname/ )
		{
			return $i;
		}
	}

	print( "#getRAIL_KeyNameToNo $keyname not found\n" );
	exit(1);
}

##-----------------------------------------------------------------------------
#	　値に変換
##-----------------------------------------------------------------------------
sub get10Number
{
	my( $number ) = @_;

	if( $number eq "0" )
	{
		return 0;
	}

	if( $number =~ s/0x// )
	{
		return hex($number);
	}

	if( $number =~ s/^[-]*0// )
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
	my( $i, $j, $one, @filedata, $outfile, $output_siz );
	my( $point_index, $line_index, $linepos_index, $camera_index );

	#情報読み込み
	open( FILEIN, $filename );
	@filedata = <FILEIN>;
	close( FILEIN );
	
	#まず、ポイント、ライン、座標、カメラの各名前リストを作成
	$point_index	= 0;
	$line_index		= 0;
	$linepos_index= 0;
	$camera_index	= 0;
	for( $i=0; $i<(@filedata-1); $i++ )
	{
		$filedata[$i] =~ s/\t//g;
		$filedata[$i] =~ s/\s//g;
		$one	= $filedata[$i];
		
		if( $one =~ /\+POINT$/ )
		{
			$POINT_NAME[ $point_index ] = &getPeaceParam( \@filedata, $i+1, "NAME" );
			$point_index ++;
		}
		elsif( $one =~ /\+LINE$/ )
		{
			$LINE_NAME[ $line_index ] = &getPeaceParam( \@filedata, $i+1, "NAME" );
			$line_index ++;
		}
		elsif( $one =~ /\+LINEPOS$/ )
		{
			$LINEPOS_NAME[ $linepos_index ] = &getPeaceParam( \@filedata, $i+1, "NAME" );
			$linepos_index ++;
		}
		elsif( $one =~ /\+CAMERA$/ )
		{
			$CAMERA_NAME[ $camera_index ] = &getPeaceParam( \@filedata, $i+1, "NAME" );
			$camera_index ++;	
		}
	}
	$filedata[$i] =~ s/\t//g;
	$filedata[$i] =~ s/\s//g;

	
	#情報を数字に置換して、出力
	$outfile = $filename;
	$outfile =~ s/\.rail//;

	#オフセット情報
	open( FILEOUT_OFS, ">$outfile.ofs" );
	binmode( FILEOUT_OFS );

	print( FILEOUT_OFS pack( "i", &get10Number(&getPeaceParamForTag( \@filedata, "ALLDATA", "OFSUNIT")) ) );
	
	close( FILEOUT_OFS );


	#ポイント情報
	{
		my( $line, $key, $camera_set, $width_ofs,  $len );
		open( FILEOUT_POINT, ">$outfile.point" );
		binmode( FILEOUT_POINT );

		for( $i=0; $i<@POINT_NAME; $i++ )
		{
			#リンク情報の整合性チェック
			&isLinkPoint( \@filedata, $POINT_NAME[$i] );
			
			#LINES
			$line = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "LINES00");
			print( FILEOUT_POINT pack( "I", &getNameIndex( \@LINE_NAME, $line ) ) );
			$line = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "LINES01");
			print( FILEOUT_POINT pack( "I", &getNameIndex( \@LINE_NAME, $line ) ) );
			$line = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "LINES02");
			print( FILEOUT_POINT pack( "I", &getNameIndex( \@LINE_NAME, $line ) ) );
			$line = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "LINES03");
			print( FILEOUT_POINT pack( "I", &getNameIndex( \@LINE_NAME, $line ) ) );

			#KEYS
			$key = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "KEYS00");
			print( FILEOUT_POINT pack( "I", &getRAIL_KeyNameToNo( $key ) ) );
			$key = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "KEYS01");
			print( FILEOUT_POINT pack( "I", &getRAIL_KeyNameToNo( $key ) ) );
			$key = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "KEYS02");
			print( FILEOUT_POINT pack( "I", &getRAIL_KeyNameToNo( $key ) ) );
			$key = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "KEYS03");
			print( FILEOUT_POINT pack( "I", &getRAIL_KeyNameToNo( $key ) ) );

			#WIDTH_OFS
			$width_ofs = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "WIDTH_OFS00");
			print( FILEOUT_POINT pack( "I", $width_ofs ) );
			$width_ofs = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "WIDTH_OFS01");
			print( FILEOUT_POINT pack( "I", $width_ofs ) );
			$width_ofs = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "WIDTH_OFS02");
			print( FILEOUT_POINT pack( "I", $width_ofs ) );
			$width_ofs = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "WIDTH_OFS03");
			print( FILEOUT_POINT pack( "I", $width_ofs ) );


			#POS
			print( FILEOUT_POINT pack( "i", &get10Number(&getPeaceParamForName( \@filedata, $POINT_NAME[$i], "VECX")) ) );
			print( FILEOUT_POINT pack( "i", &get10Number(&getPeaceParamForName( \@filedata, $POINT_NAME[$i], "VECY")) ) );
			print( FILEOUT_POINT pack( "i", &get10Number(&getPeaceParamForName( \@filedata, $POINT_NAME[$i], "VECZ")) ) );

			#CAMERA_SET
			$camera_set = &getPeaceParamForName( \@filedata, $POINT_NAME[$i], "CAMERA_SET");
			print( FILEOUT_POINT pack( "I", &getNameIndex( \@CAMERA_NAME, $camera_set ) ) );

			#NAME
			$len = length($POINT_NAME[$i]);
      #print( $POINT_NAME[$i]."len = $len\n" );
			if( $len >= $RAIL_NAMEBUF_SIZ )
			{
				print( "point_name size over ".$POINT_NAME[$i]." limit=$RAIL_NAMEBUF_SIZ\n" );
				exit(1);
			}
			$len = $RAIL_NAMEBUF_SIZ;
			print( FILEOUT_POINT pack( "A$len", $POINT_NAME[$i] ) );
		}
		
		close( FILEOUT_POINT );
	}
	print( "output $outfile.point\n" );


	#ライン情報
	{
		my( $point, $key, $linepos, $camera_set, $len, $line_grid_max );
		open( FILEOUT_LINE, ">$outfile.line" );
		binmode( FILEOUT_LINE );

		for( $i=0; $i<@LINE_NAME; $i++ )
		{
			#リンク情報の整合性チェック
			&isLinkLine( \@filedata, $LINE_NAME[$i] );
		

			#POINT
			$point = &getPeaceParamForName( \@filedata, $LINE_NAME[$i], "POINT_S");
			print( FILEOUT_LINE pack( "I", &getNameIndex( \@POINT_NAME, $point ) ) );
			$point = &getPeaceParamForName( \@filedata, $LINE_NAME[$i], "POINT_E");
			print( FILEOUT_LINE pack( "I", &getNameIndex( \@POINT_NAME, $point ) ) );

			#KEY
			$key = &getPeaceParamForName( \@filedata, $LINE_NAME[$i], "KEY");
			print( FILEOUT_LINE pack( "I", &getRAIL_KeyNameToNo( $key ) ) );

			#LINEPOS
			$linepos = &getPeaceParamForName( \@filedata, $LINE_NAME[$i], "LINEPOS_SET");
			print( FILEOUT_LINE pack( "I", &getNameIndex( \@LINEPOS_NAME, $linepos ) ) );

			#CAMERA_SET
			$camera_set = &getPeaceParamForName( \@filedata, $LINE_NAME[$i], "CAMERA_SET");
			print( FILEOUT_LINE pack( "I", &getNameIndex( \@CAMERA_NAME, $camera_set ) ) );

			#LINE_GRID_MAX
			$line_grid_max = &getPeaceParamForName( \@filedata, $LINE_NAME[$i], "LINE_GRID_MAX");
			print( FILEOUT_LINE pack( "I", &get10Number( $line_grid_max ) ) );

			#NAME
			$len = length($LINE_NAME[$i]);
			if( $len > $RAIL_NAMEBUF_SIZ )
			{
				print( "line_name size over ".$LINE_NAME[$i]." limit=$RAIL_NAMEBUF_SIZ\n" );
				exit(1);
			}
			print( FILEOUT_LINE pack( "a$RAIL_NAMEBUF_SIZ", $LINE_NAME[$i] ) );
		}
		
		close( FILEOUT_LINE );
	}
	print( "output $outfile.line\n" );


	#ラインヘッダー情報
	{
		open( FILEOUT_LINE, ">$outfile.h" );

    print( FILEOUT_LINE "#pragma once\n" );
    print( FILEOUT_LINE "// out put rail_conv.pl\n" );
    print( FILEOUT_LINE "// rail line name\n" );
    

		for( $i=0; $i<@LINE_NAME; $i++ )
    {
			print( FILEOUT_LINE "#define ".$LINE_NAME[$i]." ($i)\n" );
    }
  }
	print( "output $outfile.h\n" );

	#ラインPOS情報
	{
		my( $funcindex, $distfuncindex, $work_index, $datasiz, $one_datasiz, $work_number );
		open( FILEOUT_LINEPOS, ">$outfile.linepos" );
		binmode( FILEOUT_LINEPOS );

		for( $i=0; $i<@LINEPOS_NAME; $i++ )
		{

			#FUNCINDEX
			$funcindex = &getPeaceParamForName( \@filedata, $LINEPOS_NAME[$i], "FUNCINDEX");
			print( FILEOUT_LINEPOS pack( "I", &getLinePosFuncIDNameToNo( $funcindex ) ) );

			#FUNCDISTINDEX
      #$distfuncindex = &getPeaceParamForName( \@filedata, $LINEPOS_NAME[$i], "FUNCDISTINDEX");
      #print( FILEOUT_LINEPOS pack( "I", &getLineDistFuncIDNameToNo( $distfuncindex ) ) );

			#ワークデータの出力
			$datasiz = 0;
			$work_index = 0;
			do
			{
				$work_number = &getPeaceParamByteWorkNumberForName( \@filedata, $LINEPOS_NAME[$i], $work_index );
				$one_datasiz = &getPeaceParamByteWorkSizeForName( \@filedata, $LINEPOS_NAME[$i], $work_index );
				if( ($work_number != $BYTEWORK_INDEX_OVER) && ($one_datasiz != $BYTEWORK_INDEX_OVER) )
				{
					#出力
					if( $one_datasiz == 1 )
					{
						print( FILEOUT_LINEPOS pack( "c", &get10Number($work_number) ) );
					}
					elsif( $one_datasiz == 2 )
					{
						print( FILEOUT_LINEPOS pack( "s", &get10Number($work_number) ) );
					}
					elsif( $one_datasiz == 4 )
					{
						print( FILEOUT_LINEPOS pack( "i", &get10Number($work_number) ) );
					}
					$datasiz += $one_datasiz;

					#サイズオーバーチェック
					if( $datasiz > $RAIL_LINEPOS_SET_WORK_SIZ )
					{
						print( "linepos_work size over  name=".$LINEPOS_NAME[$i]."\n" );
						exit( 1 );
					}

					$work_index ++;
				}
			}while( ($work_number != $BYTEWORK_INDEX_OVER) && ($one_datasiz != $BYTEWORK_INDEX_OVER) );

			#足りないワークを出力
			while( $datasiz < $RAIL_LINEPOS_SET_WORK_SIZ )
			{
				print( FILEOUT_LINEPOS pack( "c", 0 ) );
				$datasiz ++;
			}
		}

		close( FILEOUT_LINEPOS );
	}
	print( "output $outfile.linepos\n" );

	#カメラ情報
	{
		my( $funcindex, $work_index, $datasiz, $one_datasiz, $work_number );
		open( FILEOUT_CAMERA, ">$outfile.camera" );
		binmode( FILEOUT_CAMERA );

		for( $i=0; $i<@CAMERA_NAME; $i++ )
		{

			#FUNCINDEX
			$funcindex = &getPeaceParamForName( \@filedata, $CAMERA_NAME[$i], "FUNCINDEX");
			print( FILEOUT_CAMERA pack( "I", &getCameraFuncIDNameToNo( $funcindex ) ) );

			#ワークデータの出力
			$datasiz = 0;
			$work_index = 0;

			do
			{
				$work_number = &getPeaceParamByteWorkNumberForName( \@filedata, $CAMERA_NAME[$i], $work_index );
				$one_datasiz = &getPeaceParamByteWorkSizeForName( \@filedata, $CAMERA_NAME[$i], $work_index );
				if( ($work_number != $BYTEWORK_INDEX_OVER) && ($one_datasiz != $BYTEWORK_INDEX_OVER) )
				{
					#出力
					if( $one_datasiz == 1 )
					{
						print( FILEOUT_CAMERA pack( "c", &get10Number($work_number) ) );
					}
					elsif( $one_datasiz == 2 )
					{
						print( FILEOUT_CAMERA pack( "s", &get10Number($work_number) ) );
					}
					elsif( $one_datasiz == 4 )
					{
						print( FILEOUT_CAMERA pack( "i", &get10Number($work_number) ) );
					}
					$datasiz += $one_datasiz;

					#サイズオーバーチェック
					if( $datasiz > $RAIL_CAMERA_SET_WORK_SIZ )
					{
						print( "camera_work size over  name=".$CAMERA_NAME[$i]."\n" );
						exit( 1 );
					}
					$work_index ++;
				}
			}while( ($work_number != $BYTEWORK_INDEX_OVER) && ($one_datasiz != $BYTEWORK_INDEX_OVER) );

			#足りないワークを出力
			while( $datasiz < $RAIL_CAMERA_SET_WORK_SIZ )
			{
				print( FILEOUT_CAMERA pack( "c", 0 ) );
				$datasiz ++;
			}
		}

		close( FILEOUT_CAMERA );
	}
	print( "output $outfile.camera\n" );
}

##-----------------------------------------------------------------------------
# 名前一致インデックスの取得
##-----------------------------------------------------------------------------
sub getNameIndex
{
	my( $data, $name ) = @_;
	my( $i, $roopnum );

	#NULLチェック
	if( $name =~ /RAIL_TBL_NULL/ )
	{
		return &get10Number($RAIL_TBL_NULL);
	}

	$roopnum = @$data;
	for( $i=0; $i<$roopnum; $i++ )
	{
		if( $$data[$i] =~ /$name/ )
		{
			return $i;
		}
	}

	print( "#getNameIndex $name not found\n" );
	exit(1);
}

##-----------------------------------------------------------------------------
# railデータ　パラメータ取得
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
			print( "#getPeaceParam peace over key=$key startindex=$startidx\n" );
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
		if( $one =~ /\+$tag/ )
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

		if( $one =~ /--NAME\:\:$name/ )
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




##-----------------------------------------------------------------------------
#		リンク情報確認　ポイント
##-----------------------------------------------------------------------------
sub isLinkPoint
{
	my( $data, $point ) = @_;
	my( $line, $key, $line_point_s, $line_point_e, $i );

	for( $i=0; $i<4; $i++ )
	{
		$line						= &getPeaceParamForName( \@$data, $point, "LINES0".$i );
		$key						= &getPeaceParamForName( \@$data, $point, "KEYS0".$i );
		if( $line =~ /RAIL_TBL_NULL/ )
		{
			if( !($key =~ /RAIL_KEY_NULL/) )
			{
				print( "point[$point] line-key error $line to $key\n" );
				exit(1);
			}
		}
		else
		{
			$line_point_s		= &getPeaceParamForName( \@$data, $line, "POINT_S" );
			$line_point_e		= &getPeaceParamForName( \@$data, $line, "POINT_E" );

			#接続ラインに自分の情報があるかチェック
			if( !($line_point_s eq $point) && !($line_point_e eq $point) )
			{
				print( "point[$point] line link error $line に $point と接続している情報はありません。\n" );
				exit(1);
			}
			#キーが割り振られているかチェック
			if( $key =~ /RAIL_KEY_NULL/ )
			{
				print( "point[$point] line-key error $line to $key\n" );
				exit(1);
			}
		}
	}
}


##-----------------------------------------------------------------------------
#		リンク情報確認　ライン
##-----------------------------------------------------------------------------
sub isLinkLine
{
	my( $data, $line ) = @_;
	my( $point_s, $point_e, $i, $point_line, $link_ok_s, $link_ok_e );
	my( $camera_set );

	$point_s = &getPeaceParamForName( \@$data, $line, "POINT_S" );
	$point_e = &getPeaceParamForName( \@$data, $line, "POINT_E" );
	
	#ラインの開始と終了のポイントに自分の情報があるか。
	$link_ok_s = 0;
	$link_ok_e = 0;
	for( $i=0; $i<4; $i++ )
	{
		$point_line	= &getPeaceParamForName( \@$data, $point_s, "LINES0".$i );
		if( $point_line eq $line )
		{
			$link_ok_s = 1;
		}

		$point_line	= &getPeaceParamForName( \@$data, $point_e, "LINES0".$i );
		if( $point_line eq $line )
		{
			$link_ok_e = 1;
		}
	}
	if(($link_ok_s != 1) || ($link_ok_e != 1))
	{
		print( "line[$line] linkerr $point_s or $point_e に $line がありません。\n" );
#		exit(1);	#強制ではない
	}

	#オフセットangleカメラ　両サイドポイントのカメラがFixAngleCameraである必要がある
	$camera_set = &getPeaceParamForName( \@$data, $line, "CAMERA_SET" );
	$camera_set = &getPeaceParamForName( \@$data, $camera_set, "FUNCINDEX" );
	if( $camera_set =~ /FIELD_RAIL_LOADER_CAMERA_FUNC_OFSANGLE/ )
	{
		$camera_set = &getPeaceParamForName( \@$data, $point_s, "CAMERA_SET" );
		$camera_set = &getPeaceParamForName( \@$data, $camera_set, "FUNCINDEX" );
		if( !($camera_set =~ /FIELD_RAIL_LOADER_CAMERA_FUNC_FIXANGLE/) )
		{
			print( "line[$line] camera_set err $point_s が FIELD_RAIL_LOADER_CAMERA_FUNC_FIXANGLE を使用していません。\n" );
#			exit(1);
		}
		$camera_set = &getPeaceParamForName( \@$data, $point_e, "CAMERA_SET" );
		$camera_set = &getPeaceParamForName( \@$data, $camera_set, "FUNCINDEX" );
		if( !($camera_set =~ /FIELD_RAIL_LOADER_CAMERA_FUNC_FIXANGLE/) )
		{
			print( "line[$line] camera_set err $point_s が FIELD_RAIL_LOADER_CAMERA_FUNC_FIXANGLE を使用していません。\n" );
#			exit(1);
		}
	}
}






