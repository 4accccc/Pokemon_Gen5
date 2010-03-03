#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		townmap_data_conv.pl
#	@brief	タウンマップのエクセルデータコンバータ
#	@author	Toru=Nagihashi
#	@data		2009.07.17
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#=============================================================================
#
#					grobal
#
#=============================================================================
#エクセルコンバータ
$EXCEL_CONV_EXE			=	$ENV{"PROJECT_ROOT"}."/tools/exceltool/ExcelSeetConv.exe";

#エクセルデータ
@TOWNMAP_XLS_HEADER	= ();		#タウンマップエクセルデータヘッダ
@TOWNMAP_XLS_DATA		= ();		#タウンマップエクセルデータのデータ本体

#生成するデータ名
$OUTPUTNAME_DATA		= "townmap_data.dat";
$OUTPUTNAME_HEADER	= "townmap_data.h";

#定義名取得用ファイルネーム
$ZONEID_FILENAME		=	"../../fldmapdata/zonetable/zone_id.h";
$GMM_DIR						= "../../message/dst/";	#コンバート後のmsg_xxx.hのdefineを参照するため
$GUIDGMM_FILENAME		=	"";
$PLACEGMM_FILENAME	= "";
$SYSFLAG_FILENAME		= "../../fldmapdata/flagwork/flag_define.h";

#定義名取得のためのバッファ
@ZONEID_BUFF				= ();		#ゾーンID用バッファ
@GUIDGMM_BUFF				= ();		#ガイド文字ID
@PLACEGMM_BUFF			=	();		#名所文字ID
@TYPE_BUFF					= ();		#タイプ名	これだけheaderから取り出す
@ZONE_SEARCH				= ();		#自分の場所を調べる方法
@SYSFLAG_BUFF				= ();		#システムフラグ

#取得したデータ
@DATA_ZONEID				=	();		#ゾーンID
@DATA_ZONESEARCH		=	();		#探索方法
@DATA_POS_X					=	();		#座標X
@DATA_POS_Y					=	();		#座標Y
@DATA_CURSOR_X			=	();		#カーソルX
@DATA_CURSOR_Y			=	();		#カーソルY
@DATA_HIT_S_X				=	();		#当たり開始X
@DATA_HIT_S_Y				=	();		#当たり開始Y
@DATA_HIT_E_X				=	();		#当たり終了X
@DATA_HIT_E_Y				=	();		#当たり終了Y
@DATA_HIT_W					=	();		#当たり幅
@DATA_TYPE					=	();		#種類
@DATA_SKY_FLAG			= ();		#空を飛ぶ
@DATA_WARP_X				= ();		#到着X
@DATA_WARP_Y				= ();		#到着Y
@DATA_ARRIVE_FLAG		= ();		#到着フラグ
@DATA_GUIDE					= ();		#ガイドID
@DATA_PLACE1				= ();		#名所１ID
@DATA_PLACE2				= ();		#名所２ID
@DATA_PLACE3				= ();		#名所３ID
@DATA_PLACE4				= ();		#名所４ID
@DATA_PLACE5				= ();		#名所５ID
@DATA_PLACE6				= ();		#名所６ID
@DATA_ZKN_ANM				=	();		#図鑑分布アニメ番号
@DATA_ZKN_POS_X			=	();		#図鑑分布配置座標X
@DATA_ZKN_POS_Y			=	();		#図鑑分布配置座標Y

#データ総数
$DATA_LENGTH				= 0;

#エラーデータ
$DATA_ERROR_VALUE		= 0xFFFF;

#=============================================================================
#
#					main
#
#=============================================================================
#-------------------------------------
#	エラー
#=====================================
if( @ARGV < 1 )
{
	print( "USAGE\n" );
	print( "perl townmap_data_conv.pl townmapdata.xls\n" );
	exit(1);
}
#-------------------------------------
#	エクセルデータコンバート
#=====================================
&EXCEL_GetData( $ARGV[0], "header", \@TOWNMAP_XLS_HEADER );
&EXCEL_GetData( $ARGV[0], "data", \@TOWNMAP_XLS_DATA );

#-------------------------------------
#	ヘッダ情報を取得
#=====================================
$line_cnt	= 0;
$is_file_name_start	= 0;
$is_type_start	= 0;
$zone_search_start	= 0;
foreach $line ( @TOWNMAP_XLS_HEADER )
{

	$line	=~ s/\r\n//g;
	@word	= split( /,/, $line );

	#データ範囲
	if( $word[0] eq "#filename_start" )
	{
		$line_cnt	= 0;
		$is_file_name_start	= 1;
	}
	elsif( $word[0] eq "#filename_end" )
	{
		$is_file_name_start	= 0;
	}
	elsif( $word[0] eq "#type_start" )
	{
		$line_cnt	= 0;
		$is_type_start	= 1;
	}
	elsif( $word[0] eq "#type_end" )
	{
		$is_type_start	= 0;
	}
	elsif( $word[0] eq "#zone_search_start" )
	{
		$line_cnt	= 0;
		$zone_search_start	= 1;
	}
	elsif( $word[0] eq "#zone_search_end" )
	{
		$zone_search_start	= 0;
	}

	#ファイル名データ取得
	if( $is_file_name_start == 1 )
	{
		if( $line_cnt == 1 )
		{
			&UndefAssert( $word[1] );

			#ガイドGMMファイル
			my $name	= "msg_" . $word[1];
			$name	=~ s/gmm/h/g;
			$GUIDGMM_FILENAME		= $GMM_DIR . $name;
			#print $GUIDGMM_FILENAME."\n";
		}
		elsif( $line_cnt == 2 )
		{
			&UndefAssert( $word[1] );

			#名所GMMファイル
			my $name	= "msg_" . $word[1];
			$name	=~ s/gmm/h/g;
			$PLACEGMM_FILENAME	= $GMM_DIR . $name;
			#print $PLACEGMM_FILENAME."\n";
		}
	}
	#種類データ取得
	elsif( $is_type_start == 1 )
	{
		if( $line_cnt >= 1 ) 
		{
			#print "$word[1]"."\n";
			push( @TYPE_BUFF, $word[1] );
		}
	}
	#種類データ取得
	elsif( $zone_search_start == 1 )
	{
		if( $line_cnt >= 1 ) 
		{
			#print "$word[1]"."\n";
			push( @ZONE_SEARCH, $word[1] );
		}
	}

	#行数カウント
	$line_cnt++;
}

#-------------------------------------
#	定義名のためのデータ読み込み
#=====================================
&FILE_GetData( $ZONEID_FILENAME, \@ZONEID_BUFF );
&FILE_GetData( $GUIDGMM_FILENAME, \@GUIDGMM_BUFF );
&FILE_GetData( $PLACEGMM_FILENAME, \@PLACEGMM_BUFF );
&FILE_GetData( $SYSFLAG_FILENAME, \@SYSFLAG_BUFF );

#-------------------------------------
#	データを取得
#=====================================
$is_start	= 0;
foreach $line ( @TOWNMAP_XLS_DATA )
{
	$line	=~ s/\r\n//g;
	@word	= split( /,/, $line );

	#データ範囲
	if( $word[0] eq "#file_start" )
	{
		@TAG_WORD	= @word;
		$is_start	= 1;
		next;
	}
	elsif( $word[0] eq "#file_end" )
	{
		$is_start	= 0;
	}

	#データ取得
	if( $is_start == 1 )
	{
		for( my $i = 0; $i < @TAG_WORD; $i++ ) 
		{
			#タグとデータを取得
			my $tag	= $TAG_WORD[$i];
			my $w		= $word[$i];

			#ゾーンID
			if( $tag eq "#zone" )
			{
				&UndefAssert( $w );
				push( @DATA_ZONEID, &GetTypeNumber( $w, \@ZONEID_BUFF ) );
			}
			#自分の位置探索
			if( $tag eq "#zone_search" )
			{
				&UndefAssert( $w );
				$val	= &GetArrayNumber( $w, \@ZONE_SEARCH );
				push( @DATA_ZONESEARCH, $val );
			}
			#座標X
			elsif( $tag eq "#pos_x" )
			{
				&UndefAssert( $w );
				push( @DATA_POS_X, $w );
			}
			#座標Y
			elsif( $tag eq "#pos_y" )
			{
				&UndefAssert( $w );
				push( @DATA_POS_Y, $w );
			}
			#カーソルX
			elsif( $tag eq "#cursor_x" )
			{
				&UndefAssert( $w );
				push( @DATA_CURSOR_X, $w );
			}
			#カーソルY
			elsif( $tag eq "#cursor_y" )
			{
				&UndefAssert( $w );
				push( @DATA_CURSOR_Y, $w );
			}
			#当たり開始X
			elsif( $tag eq "#hit_s_x" )
			{
				&UndefAssert( $w );
				push( @DATA_HIT_S_X, $w );
			}
			#当たり開始Y
			elsif( $tag eq "#hit_s_y" )
			{
				&UndefAssert( $w );
				push( @DATA_HIT_S_Y, $w );
			}
			#当たり終了X
			elsif( $tag eq "#hit_e_x" )
			{
				&UndefAssert( $w );
				push( @DATA_HIT_E_X, $w );
			}
			#当たり終了Y
			elsif( $tag eq "#hit_e_y" )
			{
				&UndefAssert( $w );
				push( @DATA_HIT_E_Y, $w );
			}
			#当たり幅
			elsif( $tag eq "#hit_w" )
			{
				&UndefAssert( $w );
				push( @DATA_HIT_W, $w );
			}
			#種類
			elsif( $tag eq "#type" )
			{
				my $val;
				if( $w eq "" )
				{
					$val	= $DATA_ERROR_VALUE;
				}
				else
				{
					$val	= &GetArrayNumber( $w, \@TYPE_BUFF );
				}
				push( @DATA_TYPE, $val );
			}
			#空を飛ぶ
			elsif( $tag eq "#sky_flag" )
			{
				push( @DATA_SKY_FLAG, $w );
			}
			#着地X
			elsif( $tag eq "#warp_x" )
			{
				push( @DATA_WARP_X, $w );
			}
			#着地Y
			elsif( $tag eq "#warp_y" )
			{
				push( @DATA_WARP_Y, $w );
			}
			#到着フラグ
			elsif( $tag eq "#arrive_flag" )
			{
				my $val;
				if( $w eq "" )
				{
					$val	= $DATA_ERROR_VALUE;
				}
				else
				{
					$val	= &GetFlagNumber( $w );
				}
				push( @DATA_ARRIVE_FLAG, $val );
			}
			#ガイド文字
			elsif( $tag eq "#guide" )
			{
				my $val;
				if( $w eq "" )
				{
					$val	= $DATA_ERROR_VALUE;
				}
				else
				{
					$val	= &GetTypeNumber( $w, \@GUIDGMM_BUFF );
				}
				push( @DATA_GUIDE, $val );
			}
			#名所１
			elsif( $tag eq "#place1" )
			{
				my $val;
				if( $w eq "" )
				{
					$val	= $DATA_ERROR_VALUE;
				}
				else
				{
					$val	= &GetTypeNumber( $w, \@PLACEGMM_BUFF );
				}
				push( @DATA_PLACE1, $val );
			}
			#名所２
			elsif( $tag eq "#place2" )
			{
				my $val;
				if( $w eq "" )
				{
					$val	= $DATA_ERROR_VALUE;
				}
				else
				{
					$val	= &GetTypeNumber( $w, \@PLACEGMM_BUFF );
				}
				push( @DATA_PLACE2, $val );
			}
			#名所３
			elsif( $tag eq "#place3" )
			{
				my $val;
				if( $w eq "" )
				{
					$val	= $DATA_ERROR_VALUE;
				}
				else
				{
					$val	= &GetTypeNumber( $w, \@PLACEGMM_BUFF );
				}
				push( @DATA_PLACE3, $val );
			}
			#名所４
			elsif( $tag eq "#place4" )
			{
				my $val;
				if( $w eq "" )
				{
					$val	= $DATA_ERROR_VALUE;
				}
				else
				{
					$val	= &GetTypeNumber( $w, \@PLACEGMM_BUFF );
				}
				push( @DATA_PLACE4, $val );
			}
			#名所５
			elsif( $tag eq "#place5" )
			{
				my $val;
				if( $w eq "" )
				{
					$val	= $DATA_ERROR_VALUE;
				}
				else
				{
					$val	= &GetTypeNumber( $w, \@PLACEGMM_BUFF );
				}
				push( @DATA_PLACE5, $val );
			}
			#名所６
			elsif( $tag eq "#place6" )
			{
				my $val;
				if( $w eq "" )
				{
					$val	= $DATA_ERROR_VALUE;
				}
				else
				{
					$val	= &GetTypeNumber( $w, \@PLACEGMM_BUFF );
				}
				push( @DATA_PLACE6, $val );
			}
			#図鑑分布アニメ番号
			if( $tag eq "#zkn_anm" )
			{
				&UndefAssert( $w );
				push( @DATA_ZKN_ANM, $val );
			}
			#図鑑分布配置座標X
			elsif( $tag eq "#zkn_pos_x" )
			{
				&UndefAssert( $w );
				push( @DATA_ZKN_POS_X, $w );
			}
			#図鑑分布配置座標Y
			elsif( $tag eq "#zkn_pos_y" )
			{
				&UndefAssert( $w );
				push( @DATA_ZKN_POS_Y, $w );
			}
			#終了
			elsif( $tag eq "#end" )
			{
				$DATA_LENGTH++;
			}
		}
	}
}
#-------------------------------------
#	当たり判定情報に場所の位置を足す
#=====================================
for( my $i = 0; $i < $DATA_LENGTH; $i++ )
{
	$DATA_HIT_S_X[$i]	+= $DATA_POS_X[$i];
	$DATA_HIT_S_Y[$i]	+= $DATA_POS_Y[$i];
	$DATA_HIT_E_X[$i]	+= $DATA_POS_X[$i];
	$DATA_HIT_E_Y[$i]	+= $DATA_POS_Y[$i];
}

#-------------------------------------
#	デバッグプリント
#=====================================
if(0)
{
	print( "DEBUG_PRINT_START\n" );
	for( my $i = 0; $i < $DATA_LENGTH; $i++ )
	{
		print( "DATA=".$i."\n" );
		print( "ZONE=".$DATA_ZONEID[$i]."\n" );
		print( "SEAR=".$DATA_ZONESEARCH[$i]."\n" );
		print( "posX=".$DATA_POS_X[$i]."\n" );
		print( "posY=".$DATA_POS_Y[$i]."\n" );
		print( "curX=".$DATA_CURSOR_X[$i]."\n" );
		print( "curY=".$DATA_CURSOR_Y[$i]."\n" );
		print( "hisX=".$DATA_HIT_S_X[$i]."\n" );
		print( "hisY=".$DATA_HIT_S_Y[$i]."\n" );
		print( "hieX=".$DATA_HIT_E_X[$i]."\n" );
		print( "hieY=".$DATA_HIT_E_Y[$i]."\n" );
		print( "hitW=".$DATA_HIT_W[$i]."\n" );
		print( "type=".$DATA_TYPE[$i]."\n" );
		print( "skyf=".$DATA_SKY_FLAG[$i]."\n" );
		print( "warX=".$DATA_WARP_X[$i]."\n" );
		print( "warY=".$DATA_WARP_Y[$i]."\n" );
		print( "arrf=".$DATA_ARRIVE_FLAG[$i]."\n" );
		print( "guid=".$DATA_GUIDE[$i]."\n" );
		print( "pla1=".$DATA_PLACE1[$i]."\n" );
		print( "pla2=".$DATA_PLACE2[$i]."\n" );
		print( "pla3=".$DATA_PLACE3[$i]."\n" );
		print( "pla4=".$DATA_PLACE4[$i]."\n" );
		print( "pla5=".$DATA_PLACE5[$i]."\n" );
		print( "pla6=".$DATA_PLACE6[$i]."\n" );
   	print( "zknAnm=".$DATA_ZKN_ANM[$i]."\n" );
		print( "zknPosX=".$DATA_ZKN_POS_X[$i]."\n" );
		print( "zknPosY=".$DATA_ZKN_POS_Y[$i]."\n" );
    print( "\n" );
	}
}

#-------------------------------------
#	ヘッダ作成
#=====================================
open( FILEOUT, ">$OUTPUTNAME_HEADER" );
print( FILEOUT "#pragma once\n" );
print( FILEOUT "//このファイルはtownmap_data_convによって自動生成されています\n\n" );
print( FILEOUT "//データ総数\n" );
print( FILEOUT "#define TOWNMAP_DATA_MAX\t($DATA_LENGTH)\n\n" );
print( FILEOUT "//データエラー値\n" );
print( FILEOUT "#define TOWNMAP_DATA_ERROR\t($DATA_ERROR_VALUE)\n\n" );
print( FILEOUT "//場所のタイプ\n" );
$cnt	= 0;
foreach $type ( @TYPE_BUFF )
{
	print( FILEOUT "#define TOWNMAP_PLACETYPE_$type\t($cnt)\n" );
	$cnt++;
}
$cnt	= 0;
foreach $search ( @ZONE_SEARCH )
{
	print( FILEOUT "#define TOWNMAP_ZONESEARCH_$search\t($cnt)\n" );
	$cnt++;
}
print( FILEOUT "#define TOWNMAP_PLACETYPE_MAX\t($cnt)\n\n" );
close( FILEOUT ); 

#-------------------------------------
#	データ化
#=====================================
open( FILEOUT, ">$OUTPUTNAME_DATA" );
binmode( FILEOUT );
for( my $i = 0; $i < $DATA_LENGTH; $i++ )
{
	print( FILEOUT pack( "S", $DATA_ZONEID[$i] ) );
	print( FILEOUT pack( "S", $DATA_ZONESEARCH[$i] ) );
	print( FILEOUT pack( "S", $DATA_POS_X[$i] ) );
	print( FILEOUT pack( "S", $DATA_POS_Y[$i] ) );
	print( FILEOUT pack( "S", $DATA_CURSOR_X[$i] ) );
	print( FILEOUT pack( "S", $DATA_CURSOR_Y[$i] ) );
	print( FILEOUT pack( "S", $DATA_HIT_S_X[$i] ) );
	print( FILEOUT pack( "S", $DATA_HIT_S_Y[$i] ) );
	print( FILEOUT pack( "S", $DATA_HIT_E_X[$i] ) );
	print( FILEOUT pack( "S", $DATA_HIT_E_Y[$i] ) );
	print( FILEOUT pack( "S", $DATA_HIT_W[$i] ) );
	print( FILEOUT pack( "S", $DATA_TYPE[$i] ) );
	print( FILEOUT pack( "S", $DATA_SKY_FLAG[$i] ) );
	print( FILEOUT pack( "S", $DATA_WARP_X[$i] ) );
	print( FILEOUT pack( "S", $DATA_WARP_Y[$i] ) );
	print( FILEOUT pack( "S", $DATA_ARRIVE_FLAG[$i] ) );
	print( FILEOUT pack( "S", $DATA_GUIDE[$i] ) );
	print( FILEOUT pack( "S", $DATA_PLACE1[$i] ) );
	print( FILEOUT pack( "S", $DATA_PLACE2[$i] ) );
	print( FILEOUT pack( "S", $DATA_PLACE3[$i] ) );
	print( FILEOUT pack( "S", $DATA_PLACE4[$i] ) );
	print( FILEOUT pack( "S", $DATA_PLACE5[$i] ) );
	print( FILEOUT pack( "S", $DATA_PLACE6[$i] ) );
	print( FILEOUT pack( "S", $DATA_ZKN_ANM[$i] ) );
	print( FILEOUT pack( "S", $DATA_ZKN_POS_X[$i] ) );
	print( FILEOUT pack( "S", $DATA_ZKN_POS_Y[$i] ) );
}
close( FILEOUT ); 

#-------------------------------------
#	正常終了
#=====================================
exit(0);


#=============================================================================
#
#					sub routine
#
#============================================================================
#-------------------------------------
#	@brief	エクセルデータをカンマ区切りで読み込み
#	@param	ファイル名スカラー
#	@param	シート名スカラー
#	@param	格納するのバッファ（配列のリファレンス\@buff）
#=====================================
sub EXCEL_GetData
{
	my( $filename, $sheetname, $buff ) = @_;
	my( $EXCEL_DEFDATA_FILE_NAME );

	#基本情報シートをコンバート
	$EXCEL_DEFDATA_FILE_NAME = "def_data.txt";
	system( $EXCEL_CONV_EXE.' -o '.$EXCEL_DEFDATA_FILE_NAME.' -n '. $sheetname.' -s csv '.$filename );

	open( EXCEL_DEF_FILEIN, $EXCEL_DEFDATA_FILE_NAME );
	@$buff = <EXCEL_DEF_FILEIN>;
	close( EXCEL_DEF_FILEIN );

	system( 'rm '.$EXCEL_DEFDATA_FILE_NAME );
}
#-------------------------------------
#	@brief	ファイル読み込み
#	@param	ファイル名スカラー
#	@param	格納するのバッファ（配列のリファレンス\@buff）
#=====================================
sub FILE_GetData
{
	my( $filename, $buff ) = @_;

	#基本情報シートをコンバート
	open( FILEIN, $filename );
	@$buff = <FILEIN>;
	close( FILEIN );
}

#-------------------------------------
#	@brief	定義の数値を取得
#	@param	定義名スカラー
#	@param	定義名が入ったファイルのバッファ（リファレンス\@buff）
#	@retval	数値
#=====================================
sub GetTypeNumber
{
	my( $name, $buff ) = @_;

	foreach $data ( @$buff )
	{
		#print( " $name == $data \n" );
		if( $data =~ /$name\s*\(\s*([0-9]*)\)/ )
		{
			#print "ok\n";
			return $1;
		}
	}

	print( "$name not find\n" );
	exit(1);
}
#-------------------------------------
#	@brief	フラグの定義の数値を取得
#	@param	定義名スカラー
#	@retval	数値
#=====================================
sub GetFlagNumber
{
	my( $name ) = @_;

	foreach $data ( @SYSFLAG_BUFF )
	{
		#print( " $name == $data \n" );
		if( $data =~ /#define $name\s*([0-9]*)/ )
		{
			print( " $name == $data \n" );
			print "ok $1 \n";
			return $1;
		}
	}

	print( "$name not find\n" );
	exit(1);

	return $DATA_ERROR_VALUE;
}
#-------------------------------------
#	@brief	配列の名前と一致したインデックスを返す
#	@param	名前
#	@param	配列（リファレンス\@buff）
#	@retval	数値
#=====================================
sub GetArrayNumber
{
	my( $name, $buff ) = @_;
	my( $cnt );

	$cnt	= 0;
	foreach $data ( @$buff )
	{
		if( $data =~ /$name/ )
		{
			return $cnt;
		}
		$cnt++;
	}

	print( "$name not find\n" );
	exit(1);
}

#-------------------------------------
#	@brief	未設定チェック
#	@param	値
#=====================================
sub UndefAssert
{
	my( $val )	= @_;
	if( $val eq "" )
	{
		print ( "未設定です\n" );
		exit(1);
	}
}
