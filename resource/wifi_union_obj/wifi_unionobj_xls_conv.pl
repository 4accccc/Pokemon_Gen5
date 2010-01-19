#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		wifi_unionobj_xls_conv.pl
#	@brief	ユニオンOBJのエクセルデータコンバータ
#	@author	Toru=Nagihashi
#	@data		2010.01.19
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#=============================================================================
#
#					grobal
#
#=============================================================================
#エクセルコンバータ
$EXCEL_CONV_EXE			=	$ENV{"PROJECT_ROOT"}."/tools/exceltool/ExcelSeetConv.exe";

#エクセルデータ
@XLS_DATA		= ();		#エクセルデータのデータ本体

#生成するデータ名
$OUTPUTNAME_DATA		= "wifi_unionobj_plt.cdat";

#取得したデータ
@DATA_PLT   				=	();		#パレットID

#データ総数
$DATA_LENGTH				= 0;

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
	print( "perl wifi_unionobj_xls_conv.pl wifi_unionobj_plt.xls\n" );
	exit(1);
}
#-------------------------------------
#	エクセルデータコンバート
#=====================================
&EXCEL_GetData( $ARGV[0], "data", \@XLS_DATA );

#-------------------------------------
#	データを取得
#=====================================
$is_start	= 0;
foreach $line ( @XLS_DATA )
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

			#パレット番号
			if( $tag eq "#plt_num" )
			{
				&UndefAssert( $w );
        #&NumberAssert( $w ); 
				push( @DATA_PLT, $w );
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
#	デバッグプリント
#=====================================
if(0)
{
	print( "DEBUG_PRINT_START\n" );
	for( my $i = 0; $i < $DATA_LENGTH; $i++ )
	{
		print( "DATA=".$i."\n" );
		print( "ZONE=".$DATA_PLT[$i]."\n" );
		print( "\n" );
	}
}

#-------------------------------------
#	ヘッダ作成
#=====================================
open( FILEOUT, ">$OUTPUTNAME_DATA" );
print( FILEOUT "#pragma once\n" );
print( FILEOUT "//このファイルはwifi_unionobj_xls_conv.plによって自動生成されています\n" );
print( FILEOUT "//UNIONOBJは１６キャラクター分を８パレットでまかなっているので\n" );
print( FILEOUT "//どのキャラクターがどのパレットを参照しているかを調べるためのテーブルです\n" );
print( FILEOUT "//テーブルは見た目番号順に並んでいます(my_statusのtrainer_view)\n" );
$cnt	= 0;
print( FILEOUT "static const u8 sc_wifi_unionobj_plt[] =\n" );
print( FILEOUT "{\n" );

foreach $plt ( @DATA_PLT )
{
	print( FILEOUT "\t$plt,\n" );
	$cnt++;
}
print( FILEOUT "};\n" );

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

#-------------------------------------
#	@brief	数値チェック
#	@param	値
#=====================================
sub NumberAssert
{
	my( $val )	= @_;
	if ($val =~ /[0-9]+/)
	{
		print ( "数値を入れてください\n" );
		exit(1);
	}
}
