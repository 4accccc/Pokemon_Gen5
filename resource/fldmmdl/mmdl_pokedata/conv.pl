###############################################################
#
#
#       連れ歩きポケモン　検索データ作成
#
#     conv.pl outdata excel_tab objcode pm_version symbol
#
#
#
#
#
###############################################################
#!/usr/bin/perl

use encode;
use utf8;

use open OUT => ":encoding(shiftjis)";

binmode(STDERR,":encoding(shiftjis)");
binmode(STDOUT,":encoding(shiftjis)");

if( @ARGV < 5 ){
  print( "conv.pl outdata excel_tab objcode pm_version symbol\n" );
  exit(1);
}


#データ取得
open( FILEIN, '<:encoding(shiftjis)', $ARGV[1] );
@EXCEL_FILE = <FILEIN>;
close( FILEIN );


#ヘッダー読み込み
open( FILEIN, $ARGV[2] );
@OBJCODE = <FILEIN>;
close( FILEIN );

#性別読み込み
open( FILEIN, $ARGV[3] );
@SEXCODE = <FILEIN>;
close( FILEIN );

#symbol読み込み
open( FILEIN, $ARGV[4] );
@SYMBOL = <FILEIN>;
close( FILEIN );



##
# 情報のインデックス
##
$IDX_TAG        = 3;
$IDX_NO         = 6;
$IDX_OBJCODE    = 7;
$IDX_SIZE       = 15;


##
# 情報の取得
##
@DATA_TAG       = undef;
@DATA_MONSNO    = undef;
@DATA_SEX       = undef;
@DATA_FORM      = undef;
@DATA_OBJCODE   = undef;
@DATA_SIZE      = undef;

$DATA_NUM       = 0;

$FORM_NO_COUNT = 0;
$FORM_BASE_NAME = "DUMMY";

$data_in = 0;

foreach $one ( @EXCEL_FILE )
{
  $one =~ s/\r\n/\t/g;
  $one =~ s/\n/\t/g;
  
  @line = split( /\t/, $one );

  if( $data_in == 0 ){

    #開始チェック
    if( "".$line[0] eq "no" ){
      $data_in = 1;
    }
  }else{

    #終了チェック
    if( "".$line[0] eq "#END" ){
      $data_in = 0;
    }else{

      #格納
      $DATA_TAG[ $DATA_NUM ] = $line[ $IDX_TAG ];
      
      #モンスターナンバー
      #直値
      $tmp = $line[ $IDX_NO ];
      if( $tmp =~ /ポケモンNo\.([0-9]*)/ ){
        $DATA_MONSNO[ $DATA_NUM ] = $1;
      }else{
        print( $DATA_TAG[ $DATA_NUM ]."のポケモンNoが不明です。\n" );
        exit(1);
      }
      
      #オブジェコード
      $DATA_OBJCODE[ $DATA_NUM ] = $line[ $IDX_OBJCODE ];
      
      #サイズ
      $DATA_SIZE[ $DATA_NUM ] = $line[ $IDX_SIZE ];

      #性別
      $tmp = $line[ $IDX_TAG ];
      if( $tmp =~ /／オス/ ){
        $DATA_SEX[ $DATA_NUM ] = "PM_MALE";
      }elsif( $tmp =~ /／メス/ ){
        $DATA_SEX[ $DATA_NUM ] = "PM_FEMALE";
      }else{
        $DATA_SEX[ $DATA_NUM ] = "PM_NEUTRAL";
      }

      #フォルムナンバー
      #直値
      #ベースの名前が変わるまで、カウントし続ける
      if( $tmp =~ /(.*)・/ ){
        $local_form_base = $1;
      }else{
        $local_form_base = $line[ $IDX_TAG ];
      }
      if( "".$local_form_base eq "".$FORM_BASE_NAME ){
        
        $DATA_FORM[ $DATA_NUM ] = $FORM_NO_COUNT;
        $FORM_NO_COUNT ++;
      }else{
        
        $DATA_FORM[ $DATA_NUM ] = 0;
        $FORM_NO_COUNT = 1;
        $FORM_BASE_NAME = $local_form_base;
      }

      $DATA_NUM ++;
    }
  }
}

#モンスターナンバー u16
#性別               u8
#フォルムナンバー   u8
#オブジェコード     u16
#大きさ             u16
#
#のテーブルを作成

open( FILEOUT, ">".$ARGV[0] );
binmode( FILEOUT );

for( $i=0; $i<$DATA_NUM; $i++ ){

  &DEBUG_print( "tag = ".$DATA_TAG[$i]."\n" );
  &DEBUG_print( "monsno = ".$DATA_MONSNO[$i]."\n" );
  &DEBUG_print( "sex = ".$DATA_SEX[$i]."\n" );
  &DEBUG_print( "form = ".$DATA_FORM[$i]."\n" );
  &DEBUG_print( "objcode = ".$DATA_OBJCODE[$i]."\n" );
  &DEBUG_print( "mmdlsize = ".$DATA_SIZE[$i]."\n" );
  
  print( FILEOUT pack( "S", $DATA_MONSNO[$i] ) );
  print( FILEOUT pack( "C", &getSex( $DATA_SEX[$i] ) ) );
  print( FILEOUT pack( "C", $DATA_FORM[$i] ) );
  print( FILEOUT pack( "S", &getObjCode( $DATA_OBJCODE[$i] ) ) );
  print( FILEOUT pack( "S", &getMMdlSize( $DATA_SIZE[$i] ) ) );

}

close( FILEOUT );

exit(0);


sub DEBUG_print 
{
  my( $str ) = @_;

  #print( $str );
}


sub getObjCode
{
  my( $objcode ) = @_;
  my( $roop );

  for( $roop = 0; $roop<@OBJCODE; $roop++ ){
    
    if( $OBJCODE[$roop] =~ /#define $objcode [^\/]*\/\/([0-9]*)/ ){
      return $1;
    }
  }
   
  print( "OBJCODE $objcode がみつかりません\n" );
  exit(1);
}

sub getSex
{
  my( $sex ) = @_;
  my( $roop );

  for( $roop = 0; $roop<@SEXCODE; $roop++ ){
    
    if( $SEXCODE[$roop] =~ /#define\t\t$sex[^0-9]*([0-9]*)/ ){
      return $1;
    }
  }
   
  print( "SEX $sex がみつかりません\n" );
  exit(1);
}

sub getMMdlSize
{
  my( $size ) = @_;
  my( $roop );

  for( $roop = 0; $roop<@SYMBOL; $roop++ ){
    
    if( @SYMBOL[$roop] =~ /#define\s$size\s([0-9]*)/ ){
      return $1;
    }
  }
   
  print( "symbol $size がみつかりません\n" );
  exit(1);
}

