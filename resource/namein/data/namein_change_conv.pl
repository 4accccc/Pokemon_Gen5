#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		namein_change_conv.pl
#	@brief	名前入力画面で文字変換エクセルのコンバータ
#	@author	Toru=Nagihashi
#	@data		2009.10.8
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#=============================================================================
#
#					grobal
#
#=============================================================================
#エクセルコンバータ
$EXCEL_CONV_EXE			=	$ENV{"PROJECT_ROOT"}."/tools/exceltool/ExcelSeetConv.exe";

#エクセルデータ
@NAMEIN_XLS_DATA		= ();		#エクセルデータのデータ本体

#生成するデータ名
$OUTPUTNAME_DATA		= "";		#ファイル名＋拡張子.dat
$TEMPORARY_FILENAME	= "temp.dat"; #エンコード用テンポラリ
$TEMP_BODY_FILENAME	= "body.dat"; #合成用用テンポラリ
$TEMP_HEADER_FILENAME	= "header.dat"; #合成用テンポラリ

#取得したデータ
$DATA_WIDTH					=	0;		#データ幅
$DATA_HEIGHT				=	0;		#データ高さ

@DATA_INPUT					= ();		#変換する文字列
@DATA_CHANGE				= ();		#変換した文字列
@DATA_SHIFT					= ();		#SHIFT時変換した文字列
@DATA_REST					= ();		#変換後に残る文字列
$DATA_LENGTH				= 0;		#データ長

#使わない文字をエラー値として扱う
$DATA_ERROR_VALUE		= "※";

#文字制限
$WORD_LENGTH			= 3;
$DATA_END_VALUE		= "　";

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
	print( "perl namein_change_conv.pl change_***.xls\n" );
	exit(1);
}
#-------------------------------------
#	エクセルデータコンバート
#=====================================
&EXCEL_GetData( $ARGV[0], "data", \@NAMEIN_XLS_DATA );

#-------------------------------------
#	データを取得
#=====================================
$is_row_start	= 0;
foreach $line ( @NAMEIN_XLS_DATA )
{
	$line	=~ s/\r\n//g;
	@word	= split( /,/, $line );

	#データ範囲
	if( $word[0] eq "#start" )
	{
		@TAG_WORD	= @word;
		$is_row_start	= 1;

		next;
	}
	elsif( $word[0] eq "#end_row" )
	{
		$is_row_start	= 0;
	}


	#データ取得
	if( $is_row_start == 1 )
	{
		for( my $i = 0; $i < @TAG_WORD; $i++ ) 
		{
			#タグとデータを取得
			my $tag	= $TAG_WORD[$i];
			my $w		= $word[$i];

			#変換する文字列
			if( $tag eq "#input" )
			{
				&UndefAssert( $w );
				push( @DATA_INPUT, $w );
			}
			#変換される文字列
			if( $tag eq "#change" )
			{
				&UndefAssert( $w );
				push( @DATA_CHANGE, $w );
			}
			#SHIFTを押したときに変換される文字列
			if( $tag eq "#shift" )
			{
				push( @DATA_SHIFT, $w );
			}
			#変換後に残る文字列
			if( $tag eq "#rest" )
			{
				push( @DATA_REST, $w );
			}
			#後尾
			if( $tag eq "#end_col" )
			{
				$DATA_LENGTH++;
			}
		}
	}
}
#-------------------------------------
#	文字数を強制的に$WORD_LENGTHにする。
#=====================================
{
	use Encode;

	my $len	= 0;
	for( my $i = 0; $i < $DATA_LENGTH; $i++ )
	{
		$len	= length(decode('shiftjis', $DATA_INPUT[$i])); 
		while( $len < 3 )
		{
			$DATA_INPUT[$i]	= join '', $DATA_INPUT[$i], $DATA_END_VALUE;
			$len++;
		}
		$len	= length(decode('shiftjis', $DATA_CHANGE[$i])); 
		while( $len < 3 )
		{
			$DATA_CHANGE[$i]	= join '', $DATA_CHANGE[$i], $DATA_END_VALUE;
			$len++;
		}
		$len	= length(decode('shiftjis', $DATA_SHIFT[$i])); 
		while( $len < 3 )
		{
			$DATA_SHIFT[$i]	= join '', $DATA_SHIFT[$i], $DATA_END_VALUE;
			$len++;
		}
		$len	= length(decode('shiftjis', $DATA_REST[$i])); 
		while( $len < 3 )
		{
			$DATA_REST[$i]	= join '', $DATA_REST[$i],$DATA_END_VALUE;
			$len++;
		}
	}
}

#-------------------------------------
#	デバッグプリント
#=====================================
if(1)
{
	print( "DEBUG_PRINT_START\n" );
	print( "データ数".$DATA_LENGTH	."\n" );
	for( my $i = 0; $i < $DATA_LENGTH; $i++ )
	{
		print( "文字=".$DATA_INPUT[$i]."\n" );
		print( "変換=".$DATA_CHANGE[$i]."\n" );
		print( "SHIFT".$DATA_SHIFT[$i]."\n" );
		print( "残り=".$DATA_REST[$i]."\n" );
	}
}

#-------------------------------------
#	アウトプット名
#=====================================
$OUTPUTNAME_DATA	= $ARGV[0];
$OUTPUTNAME_DATA	=~ s/\.xls/\.dat/g;

#-------------------------------------
#	データ化
#=====================================
#-------------------------------------
#	ヘッダ書き込み
#=====================================
open( FILEOUT, ">$TEMP_HEADER_FILENAME" );
binmode( FILEOUT );
print( FILEOUT pack( "S", $DATA_LENGTH ) );
close( FILEOUT ); 
#-------------------------------------
#	ボディ書き込み
#=====================================
open( FILEOUT, ">$TEMP_BODY_FILENAME" );
for( my $i = 0; $i < $DATA_LENGTH; $i++ )
{
	print( FILEOUT $DATA_INPUT[$i] );
	print( FILEOUT $DATA_CHANGE[$i] );
	print( FILEOUT $DATA_SHIFT[$i] );
	print( FILEOUT $DATA_REST[$i] );
}
close( FILEOUT ); 
#-------------------------------------
#	UTF16LEコンバート
#=====================================
&EnocdeUnicode( $TEMP_BODY_FILENAME, $TEMPORARY_FILENAME );
unlink $TEMP_BODY_FILENAME;
rename $TEMPORARY_FILENAME, $TEMP_BODY_FILENAME;

#-------------------------------------
#	データのヘッダ、ボディ合成
#=====================================
open( HEADER, "<$TEMP_HEADER_FILENAME");
open( BODY, "<$TEMP_BODY_FILENAME");
open( MAIN, ">$OUTPUTNAME_DATA");
#ヘッダ書き込み
while( my $data = <HEADER> ) 
{
	print MAIN $data;
}
#ボディ書き込み
while( my $data = <BODY> ) 
{
	print MAIN $data;
}
close( MAIN ); 
close( BODY );
close( HEADER );

unlink $TEMP_HEADER_FILENAME;
unlink $TEMP_BODY_FILENAME;


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
#	@brif ファイルを自動判別しUTF-16にエンコード
#	@param	元のファイル名
#	@param	エンコード後のファイル名
#=====================================
sub EnocdeUnicode 
{
	my( $src, $dst ) = @_;
	my( $data );
	use Encode;
if( 0 )
{
	use Encode qw(from_to);
	use Encode::Guess qw/euc-jp shift-jis/;	#UTF8は自動判別

	open IN, "<$src";
	open OUT, ">$dst";
	my $content = join '', <IN>;

	my $guess = Encode::Guess::guess_encoding($content);
	unless(ref $guess){
		# 判別失敗
		die('Encodeに失敗しました。判別できません guess:' . $guess);
	}else{
		# 判別成功
		$contents = $guess->decode($content);
		$contents = Encode::encode("UTF-16LE", $content);

		print OUT $content;
	}

	close IN;
	close OUT;
}
else
{
	open READ, '<:encoding(shiftjis)', $src;
	open WRITE, '>:encoding(UTF-16LE)', $dst;
	while(my $data = <READ>){
		print WRITE $data;
	}
	close WRITE;
	close READ;
}
}
