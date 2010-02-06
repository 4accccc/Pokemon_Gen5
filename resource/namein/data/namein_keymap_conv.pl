#!/usr/bin/perl
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#
#	@file		namein_keymap_conv.pl
#	@brief	名前入力画面でキー配列エクセルのコンバータ
#	@author	Toru=Nagihashi
#	@data		2009.10.8
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#=============================================================================
#
#					grobal
#
#=============================================================================
use utf8;

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

@DATA_STR						= ();		#文字列
$DATA_LENGTH				= 0;		#文字列長

#エラーデータ
$DATA_ERROR_VALUE		= "※";	#使わない文字をエラー値として扱う

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
	print( "perl namein_keymap_conv.pl keymap_***.xls\n" );
	exit(1);
}
#-------------------------------------
#	エクセルデータコンバート
#=====================================
&EXCEL_GetDataUnicode( $ARGV[0], "data", \@NAMEIN_XLS_DATA );

#-------------------------------------
#	データを取得
#=====================================
$is_row_start	= 0;
$is_col_start	= 0;
$row_cnt	= 0;
$col_cnt	= 0;
foreach $line ( @NAMEIN_XLS_DATA )
{
	$line	=~ s/\r\n//g;
	@word	= split( /\t/, $line );

  #print "\n";
  #foreach $data ( @word )
  #{
  #  print Encode::encode("shiftjis", $data ); # UTF8フラグを落す
  #}

	#データ範囲
	if( $word[0] eq "#start" )
	{
		@TAG_WORD	= @word;
		$is_row_start	= 1;

		#データ幅を取得
		for( $col_cnt = 0; $TAG_WORD[ $col_cnt ] ne "#end_col"; $col_cnt++ ){}
		$DATA_WIDTH		= $col_cnt - 1;	##startはデータ長に含まれないので-1

		next;
	}
	elsif( $word[0] eq "#end_row" )
	{
		$is_row_start	= 0;

		#データ高さを取得
		$DATA_HEIGHT	= $row_cnt;
	}

	#データ取得
	if( $is_row_start == 1 )
	{
		$row_cnt++;
		for( my $i = 0; $i < @TAG_WORD; $i++ ) 
		{
			#タグとデータを取得
			my $tag	= $TAG_WORD[$i];
			my $w		= $word[$i];

			#行開始
			if( $tag eq "#start" )
			{
				$is_col_start	= 1;
				next;
			}
			#行終了
			elsif( $tag eq "#end_col" )
			{
				$is_col_start	= 0;
			}

			#行開始～行終了まで内部のバッファを受け取る
			if( $is_col_start )
			{
				&UndefAssert( $w );
	
				#使用しない値だったら、エラー値
				if( $w eq "NONE" )
				{
					push( @DATA_STR, $DATA_ERROR_VALUE );
				}
				else
				{
					push( @DATA_STR, $w );
				}
				$DATA_LENGTH++;
			}
		}
	}
}

#-------------------------------------
#	デバッグプリント
#=====================================
if(1)
{
	print( "DEBUG_PRINT_START\n" );
	print( "data w".$DATA_WIDTH	."\n" );
	print( "data h".$DATA_HEIGHT	."\n" );
	print( "data L".$DATA_LENGTH	."\n" );
	for( my $i = 0; $i < $DATA_LENGTH; $i++ )
	{
		print( "STR=" );
    print Encode::encode("shiftjis", $DATA_STR[$i] ); # UTF8フラグを落す 
    print "\n";
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
print( FILEOUT pack( "S", $DATA_WIDTH ) );
print( FILEOUT pack( "S", $DATA_HEIGHT ) );
close( FILEOUT ); 
#-------------------------------------
#	ボディ書き込み
#=====================================
open( FILEOUT, ">$TEMP_BODY_FILENAME" );
for( my $i = 0; $i < $DATA_LENGTH; $i++ )
{
	print( FILEOUT Encode::encode("UTF16LE",$DATA_STR[$i]) );
}
close( FILEOUT ); 
#-------------------------------------
#	UTF16LEコンバート
#=====================================
#&EnocdeUnicode( $TEMP_BODY_FILENAME, $TEMPORARY_FILENAME );
#unlink $TEMP_BODY_FILENAME;
#rename $TEMPORARY_FILENAME, $TEMP_BODY_FILENAME;

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
#	@brief	エクセルデータをユニコードトして読み込み
#	@param	ファイル名スカラー
#	@param	シート名スカラー
#	@param	格納するのバッファ（配列のリファレンス\@buff）
#=====================================
sub EXCEL_GetDataUnicode
{
	my( $filename, $sheetname, $buff ) = @_;
	my( $EXCEL_DEFDATA_FILE_NAME );

	#基本情報シートをコンバート
	$EXCEL_DEFDATA_FILE_NAME = "def_data.txt";
	system( $EXCEL_CONV_EXE.' -o '.$EXCEL_DEFDATA_FILE_NAME.' -n '. $sheetname.' -s unicode '.$filename );

	open( EXCEL_DEF_FILEIN, "<:encoding(UTF-16LE)", $EXCEL_DEFDATA_FILE_NAME );
	@$buff = <EXCEL_DEF_FILEIN>;
	close( EXCEL_DEF_FILEIN );

  #unicodeのファイルを読み込んできたので戦闘にBOMがついているので削除
#  $line	=~ s/\r\n//g;
#  $$buff[0];
  substr($$buff[0], 0, 1) = "";
  print $$buff[0];

#  foreach $data ( @$buff )
#	{
  #   print Encode::encode("shiftjis", $data ); # UTF8フラグを落す
  #}

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
