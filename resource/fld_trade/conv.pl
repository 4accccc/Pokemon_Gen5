#
#			交換ポケモンデータ　コンバーター
#			データが13個固定
#

#$FILE_NUM	= 12
@FLD_TRADE_FILE = undef;	#データファル
@MONSNO_H_FILE	= undef;	#モンスターナンバーデファイン
@TOKUSYU_H_FILE	= undef;	#特殊ナンバーデファイン
@ITEMSYM_H_FILE = undef;	#アイテムナンバーデファイン
@PMVER_H_FILE = undef;		#国コードナンバーデファイン
@OUTPUT0_FILE = undef;		#書き出しデータ
@OUTPUT1_FILE = undef;		#書き出しデータ
@OUTPUT2_FILE = undef;		#書き出しデータ
@OUTPUT3_FILE = undef;		#書き出しデータ
@OUTPUT4_FILE = undef;		#書き出しデータ
@OUTPUT5_FILE = undef;		#書き出しデータ
@OUTPUT6_FILE = undef;		#書き出しデータ
@OUTPUT7_FILE = undef;		#書き出しデータ
@OUTPUT7_FILE = undef;		#書き出しデータ
@OUTPUT8_FILE = undef;		#書き出しデータ
@OUTPUT9_FILE = undef;		#書き出しデータ
@OUTPUT10_FILE = undef;		#書き出しデータ
@OUTPUT11_FILE = undef;		#書き出しデータ
@OUTPUT12_FILE = undef;		#書き出しデータ

@INPUT_INDEX=( 1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37,);

#############################################################
#
#	メイン動さ
#
#############################################################

#ファイル読み込み
&file_open();

$count = 0;
foreach $one ( @FLD_TRADE_FILE ){

	#0項目はダミーデータ
	if( $count > 0 ){
		@line = split( "\t", $one );

		if( $count == 1 ){		#モンスターナンバー
			$OUTPUT0_FILE[0] = &get_monsno($line[$INPUT_INDEX[0]] );
			$OUTPUT1_FILE[0] = &get_monsno($line[$INPUT_INDEX[1]] );
			$OUTPUT2_FILE[0] = &get_monsno($line[$INPUT_INDEX[2]] );
			$OUTPUT3_FILE[0] = &get_monsno($line[$INPUT_INDEX[3]] );
			$OUTPUT4_FILE[0] = &get_monsno($line[$INPUT_INDEX[4]] );
			$OUTPUT5_FILE[0] = &get_monsno($line[$INPUT_INDEX[5]] );
			$OUTPUT6_FILE[0] = &get_monsno($line[$INPUT_INDEX[6]] );
			$OUTPUT7_FILE[0] = &get_monsno($line[$INPUT_INDEX[7]] );
			$OUTPUT8_FILE[0] = &get_monsno($line[$INPUT_INDEX[8]] );
			$OUTPUT9_FILE[0] = &get_monsno($line[$INPUT_INDEX[9]] );
			$OUTPUT10_FILE[0] = &get_monsno($line[$INPUT_INDEX[10]] );
			$OUTPUT11_FILE[0] = &get_monsno($line[$INPUT_INDEX[11]] );
			$OUTPUT12_FILE[0] = &get_monsno($line[$INPUT_INDEX[12]] );
		}elsif( ($count >= 3) && ($count <= 8) ){	#HP乱数～特防乱数
			$OUTPUT0_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[0]];
			$OUTPUT1_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[1]];
			$OUTPUT2_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[2]];
			$OUTPUT3_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[3]];
			$OUTPUT4_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[4]];
			$OUTPUT5_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[5]];
			$OUTPUT6_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[6]];
			$OUTPUT7_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[7]];
			$OUTPUT8_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[8]];
			$OUTPUT9_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[9]];
			$OUTPUT10_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[10]];
			$OUTPUT11_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[11]];
			$OUTPUT12_FILE[ 1+($count - 3) ] = $line[$INPUT_INDEX[12]];
		}elsif( $count == 9 ){	#特性
			$OUTPUT0_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[0]] );
			$OUTPUT1_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[1]] );
			$OUTPUT2_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[2]] );
			$OUTPUT3_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[3]] );
			$OUTPUT4_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[4]] );
			$OUTPUT5_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[5]] );
			$OUTPUT6_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[6]] );
			$OUTPUT7_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[7]] );
			$OUTPUT8_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[8]] );
			$OUTPUT9_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[9]] );
			$OUTPUT10_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[10]] );
			$OUTPUT11_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[11]] );
			$OUTPUT12_FILE[ 7 ] = &get_tokusyu($line[$INPUT_INDEX[12]] );
		}elsif( ($count >= 10) && ($count <= 16) ){	#ID～RND
			$OUTPUT0_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[0]];
			$OUTPUT1_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[1]];
			$OUTPUT2_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[2]];
			$OUTPUT3_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[3]];
			$OUTPUT4_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[4]];
			$OUTPUT5_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[5]];
			$OUTPUT6_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[6]];
			$OUTPUT7_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[7]];
			$OUTPUT8_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[8]];
			$OUTPUT9_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[9]];
			$OUTPUT10_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[10]];
			$OUTPUT11_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[11]];
			$OUTPUT12_FILE[ 8+($count - 10) ] = $line[$INPUT_INDEX[12]];
		}elsif( $count == 17 ){	#アイテム
			$OUTPUT0_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[0]] );
			$OUTPUT1_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[1]] );
			$OUTPUT2_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[2]] );
			$OUTPUT3_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[3]] );
			$OUTPUT4_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[4]] );
			$OUTPUT5_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[5]] );
			$OUTPUT6_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[6]] );
			$OUTPUT7_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[7]] );
			$OUTPUT8_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[8]] );
			$OUTPUT9_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[9]] );
			$OUTPUT10_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[10]] );
			$OUTPUT11_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[11]] );
			$OUTPUT12_FILE[ 15 ] = &get_item($line[$INPUT_INDEX[12]] );
		}elsif( ($count >= 19) && ($count <= 20) ){	#親性別　毛
			$OUTPUT0_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[0]];
			$OUTPUT1_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[1]];
			$OUTPUT2_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[2]];
			$OUTPUT3_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[3]];
			$OUTPUT4_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[4]];
			$OUTPUT5_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[5]];
			$OUTPUT6_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[6]];
			$OUTPUT7_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[7]];
			$OUTPUT8_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[8]];
			$OUTPUT9_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[9]];
			$OUTPUT10_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[10]];
			$OUTPUT11_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[11]];
			$OUTPUT12_FILE[ 16+($count - 19) ] = $line[$INPUT_INDEX[12]];
		}elsif( $count == 21 ){		#国コード
			$OUTPUT0_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[0]] );
			$OUTPUT1_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[1]] );
			$OUTPUT2_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[2]] );
			$OUTPUT3_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[3]] );
			$OUTPUT4_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[4]] );
			$OUTPUT5_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[5]] );
			$OUTPUT6_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[6]] );
			$OUTPUT7_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[7]] );
			$OUTPUT8_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[8]] );
			$OUTPUT9_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[9]] );
			$OUTPUT10_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[10]] );
			$OUTPUT11_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[11]] );
			$OUTPUT12_FILE[ 18 ] = &get_pmver($line[$INPUT_INDEX[12]] );
		}elsif( $count == 22 ){		#交換するポケモン
			$OUTPUT0_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[0]] );
			$OUTPUT1_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[1]] );
			$OUTPUT2_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[2]] );
			$OUTPUT3_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[3]] );
			$OUTPUT4_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[4]] );
			$OUTPUT5_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[5]] );
			$OUTPUT6_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[6]] );
			$OUTPUT7_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[7]] );
			$OUTPUT8_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[8]] );
			$OUTPUT9_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[9]] );
			$OUTPUT10_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[10]] );
			$OUTPUT11_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[11]] );
			$OUTPUT12_FILE[ 19 ] = &get_monsno($line[$INPUT_INDEX[12]] );
		}elsif( $count == 23 ){		#交換するポケモンの性別
			$OUTPUT0_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[0]] );
			$OUTPUT1_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[1]] );
			$OUTPUT2_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[2]] );
			$OUTPUT3_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[3]] );
			$OUTPUT4_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[4]] );
			$OUTPUT5_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[5]] );
			$OUTPUT6_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[6]] );
			$OUTPUT7_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[7]] );
			$OUTPUT8_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[8]] );
			$OUTPUT9_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[9]] );
			$OUTPUT10_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[10]] );
			$OUTPUT11_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[11]] );
			$OUTPUT12_FILE[ 20 ] = &get_monsno($line[$INPUT_INDEX[12]] );
		}
	}
	$count ++;
}


#書き出す
open( FILEOUT_0, ">fld_trade_00.dat" );
open( FILEOUT_0_T, ">fld_trade_00.txt" );
binmode( FILEOUT_0 );

open( FILEOUT_1, ">fld_trade_01.dat" );
open( FILEOUT_1_T, ">fld_trade_01.txt" );
binmode( FILEOUT_1 );

open( FILEOUT_2, ">fld_trade_02.dat" );
open( FILEOUT_2_T, ">fld_trade_02.txt" );
binmode( FILEOUT_2 );

open( FILEOUT_3, ">fld_trade_03.dat" );
open( FILEOUT_3_T, ">fld_trade_03.txt" );
binmode( FILEOUT_3 );

open( FILEOUT_4, ">fld_trade_04.dat" );
open( FILEOUT_4_T, ">fld_trade_04.txt" );
binmode( FILEOUT_4 );

open( FILEOUT_5, ">fld_trade_05.dat" );
open( FILEOUT_5_T, ">fld_trade_05.txt" );
binmode( FILEOUT_5 );

open( FILEOUT_6, ">fld_trade_06.dat" );
open( FILEOUT_6_T, ">fld_trade_06.txt" );
binmode( FILEOUT_6 );

open( FILEOUT_7, ">fld_trade_07.dat" );
open( FILEOUT_7_T, ">fld_trade_07.txt" );
binmode( FILEOUT_7 );

open( FILEOUT_8, ">fld_trade_08.dat" );
open( FILEOUT_8_T, ">fld_trade_08.txt" );
binmode( FILEOUT_8 );

open( FILEOUT_9, ">fld_trade_09.dat" );
open( FILEOUT_9_T, ">fld_trade_09.txt" );
binmode( FILEOUT_9 );

open( FILEOUT_10, ">fld_trade_10.dat" );
open( FILEOUT_10_T, ">fld_trade_10.txt" );
binmode( FILEOUT_10 );

open( FILEOUT_11, ">fld_trade_11.dat" );
open( FILEOUT_11_T, ">fld_trade_11.txt" );
binmode( FILEOUT_11 );

open( FILEOUT_12, ">fld_trade_12.dat" );
open( FILEOUT_12_T, ">fld_trade_12.txt" );
binmode( FILEOUT_12 );

for( $i=0; $i<21; $i++ ){
	print( FILEOUT_0 pack("I", $OUTPUT0_FILE[$i]) );
	print( FILEOUT_1 pack("I", $OUTPUT1_FILE[$i]) );
	print( FILEOUT_2 pack("I", $OUTPUT2_FILE[$i]) );
	print( FILEOUT_3 pack("I", $OUTPUT3_FILE[$i]) );
	print( FILEOUT_4 pack("I", $OUTPUT4_FILE[$i]) );
	print( FILEOUT_5 pack("I", $OUTPUT5_FILE[$i]) );
	print( FILEOUT_6 pack("I", $OUTPUT6_FILE[$i]) );
	print( FILEOUT_7 pack("I", $OUTPUT7_FILE[$i]) );
	print( FILEOUT_8 pack("I", $OUTPUT8_FILE[$i]) );
	print( FILEOUT_9 pack("I", $OUTPUT9_FILE[$i]) );
	print( FILEOUT_10 pack("I", $OUTPUT10_FILE[$i]) );
	print( FILEOUT_11 pack("I", $OUTPUT11_FILE[$i]) );
	print( FILEOUT_12 pack("I", $OUTPUT12_FILE[$i]) );

	print( FILEOUT_0_T $OUTPUT0_FILE[$i]."\r\n" );
	print( FILEOUT_1_T $OUTPUT1_FILE[$i]."\r\n" );
	print( FILEOUT_2_T $OUTPUT2_FILE[$i]."\r\n" );
	print( FILEOUT_3_T $OUTPUT3_FILE[$i]."\r\n" );
	print( FILEOUT_4_T $OUTPUT4_FILE[$i]."\r\n" );
	print( FILEOUT_5_T $OUTPUT5_FILE[$i]."\r\n" );
	print( FILEOUT_6_T $OUTPUT6_FILE[$i]."\r\n" );
	print( FILEOUT_7_T $OUTPUT7_FILE[$i]."\r\n" );
	print( FILEOUT_8_T $OUTPUT8_FILE[$i]."\r\n" );
	print( FILEOUT_9_T $OUTPUT9_FILE[$i]."\r\n" );
	print( FILEOUT_10_T $OUTPUT10_FILE[$i]."\r\n" );
	print( FILEOUT_11_T $OUTPUT11_FILE[$i]."\r\n" );
	print( FILEOUT_12_T $OUTPUT12_FILE[$i]."\r\n" );
}

close( FILEOUT_0 );
close( FILEOUT_1 );
close( FILEOUT_2 );
close( FILEOUT_3 );
close( FILEOUT_4 );
close( FILEOUT_5 );
close( FILEOUT_6 );
close( FILEOUT_7 );
close( FILEOUT_8 );
close( FILEOUT_9 );
close( FILEOUT_10 );
close( FILEOUT_11 );
close( FILEOUT_12 );

close( FILEOUT_0_T );
close( FILEOUT_1_T );
close( FILEOUT_2_T );
close( FILEOUT_3_T );
close( FILEOUT_4_T );
close( FILEOUT_5_T );
close( FILEOUT_6_T );
close( FILEOUT_7_T );
close( FILEOUT_8_T );
close( FILEOUT_9_T );
close( FILEOUT_10_T );
close( FILEOUT_11_T );
close( FILEOUT_12_T );

exit(0);


sub file_open{
	system( "../../../convert/exceltool/ExcelSeetConv -s tab fld_trade.xls" );
	open( FILEIN, "fld_trade.txt" );
	@FLD_TRADE_FILE = <FILEIN>;
	close( FILEIN );

	open( FILEIN, "monsno.h" );
	@MONSNO_H_FILE = <FILEIN>;
	close( FILEIN );

	open( FILEIN, "tokusyu_def.h" );
	@TOKUSYU_H_FILE = <FILEIN>;
	close( FILEIN );

	open( FILEIN, "itemsym.h" );
	@ITEMSYM_H_FILE = <FILEIN>;
	close( FILEIN );

	open( FILEIN, "pm_version.h" );
	@PMVER_H_FILE = <FILEIN>;
	close( FILEIN );
}
sub get_monsno{
	my( $key )  = @_;
	
	foreach $one ( @MONSNO_H_FILE ){

		if( $one =~ /#define\t*$key\t*\(([0-9]*)\)/ ){
			return $1;	#ヒットしたナンバーを返す
		}
	}
	return 0;
}
sub get_tokusyu{
	my( $key )  = @_;
	
	foreach $one ( @TOKUSYU_H_FILE ){

		if( $one =~ /#define\t*$key\t*\(([0-9]*)\)/ ){
			return $1;	#ヒットしたナンバーを返す
		}
	}
	return 0;
	
}
sub get_item{
	my( $key )  = @_;
	
	foreach $one ( @ITEMSYM_H_FILE ){

		if( $one =~ /#define $key\t*\( ([0-9]*) \)/ ){
			return $1;	#ヒットしたナンバーを返す
		}
	}
	return 0;
}
sub get_pmver{
	my( $key )  = @_;
	
	foreach $one ( @PMVER_H_FILE ){

		if( $one =~ /#define\t*$key\t*([0-9]*)/ ){
			return $1;	#ヒットしたナンバーを返す
		}
	}
	return 0;
}
