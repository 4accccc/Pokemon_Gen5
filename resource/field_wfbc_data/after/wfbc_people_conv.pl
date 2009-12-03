################################################################
#
#
#   WFBC  人物情報のコンバート
#
#   wfbc_people_conv.pl excel_tab objcode trtypedef monsno_def  script_h item_def output_header
#
#
#
################################################################


#引数　チェック
if( @ARGV < 7 )
{
  print( "wfbc_people_conv.pl excel_tab objcode trtypedef monsno_def  script_h item_def output_header\n" );
  exit(1);
}

@EXCEL = undef;
@OBJCODE = undef;
@TRTYPE_DEF = undef;
@MONSNO_DEF = undef;
@SCRIPT = undef;
@ITEM_DEF = undef;



#
# 各ファイルの読み込み
#
open( FILEIN, $ARGV[0] );
@EXCEL = <FILEIN>;
close( FILEIN );
open( FILEIN, $ARGV[1] );
@OBJCODE = <FILEIN>;
close( FILEIN );
open( FILEIN, $ARGV[2] );
@TRTYPE_DEF = <FILEIN>;
close( FILEIN );
open( FILEIN, $ARGV[3] );
@MONSNO_DEF = <FILEIN>;
close( FILEIN );
open( FILEIN, $ARGV[4] );
@SCRIPT = <FILEIN>;
close( FILEIN );
open( FILEIN, $ARGV[5] );
@ITEM_DEF = <FILEIN>;
close( FILEIN );


#
#人物情報を取得
#
$PEOPLE_MAX = 0;  #人物最大数

@PP_DATA = undef;

#項目順番
$PP_DATA_IDX_NO = 0;
$PP_DATA_IDX_NAME = 1;
$PP_DATA_IDX_OBJCODE = 2;
$PP_DATA_IDX_TRTYPE = 3;
$PP_DATA_IDX_MOOD_WF = 4;
$PP_DATA_IDX_MOOD_BC = 5;
$PP_DATA_IDX_ENC_MONSNO_00 = 6;
$PP_DATA_IDX_ENC_LV_00 = 7;
$PP_DATA_IDX_ENC_PERCENT_00 = 8;
$PP_DATA_IDX_ENC_MONSNO_01 = 9;
$PP_DATA_IDX_ENC_LV_01 = 10;
$PP_DATA_IDX_ENC_PERCENT_01 = 11;
$PP_DATA_IDX_ENC_MONSNO_02 = 12;
$PP_DATA_IDX_ENC_LV_02 = 13;
$PP_DATA_IDX_ENC_PERCENT_02 = 14;
$PP_DATA_IDX_BTL_MONSNO_00 = 15;
$PP_DATA_IDX_BTL_LV_00 = 16;
$PP_DATA_IDX_BTL_SEX_00 = 17;
$PP_DATA_IDX_BTL_TOKUSEI_00 = 18;
$PP_DATA_IDX_BTL_MONSNO_01 = 19;
$PP_DATA_IDX_BTL_LV_01 = 20;
$PP_DATA_IDX_BTL_SEX_01 = 21;
$PP_DATA_IDX_BTL_TOKUSEI_01 = 22;
$PP_DATA_IDX_BTL_MONSNO_02 = 23;
$PP_DATA_IDX_BTL_LV_02 = 24;
$PP_DATA_IDX_BTL_SEX_02 = 25;
$PP_DATA_IDX_BTL_TOKUSEI_02 = 26;
$PP_DATA_IDX_SCRIPT_WF_00 = 27;
$PP_DATA_IDX_SCRIPT_BC_00 = 28;
$PP_DATA_IDX_ITEM_WF = 29;
$PP_DATA_IDX_ITEM_PERCENT_WF = 30;
$PP_DATA_IDX_ITEM_BC = 31;
$PP_DATA_IDX_ITEM_MONEY_BC = 32;
$PP_DATA_IDX_NUM = 33;



#データを格納
$data_in = 0;
foreach $one ( @EXCEL )
{
  @line = split( /\t/, $one );

  #print( $line[0]."-".$line[1]."-".$line[2]."\n"  );
  
  if( $data_in == 0 )
  {
    if( "".$line[0] eq "no" )
    {
      #print( "input_start\n" );
      $data_in = 1;
    }
  }
  else
  {
    if( "".$line[0] eq "#END" )
    {
      #print( "input_end\n" );
      $data_in = 0;
    }
    else
    {
      #格納
      for( $i=0; $i<$PP_DATA_IDX_NUM; $i++ )
      {
        #print( "input ".(($PEOPLE_MAX*$PP_DATA_IDX_NUM) + $i)." to ".$line[$i]."\n" );
        $PP_DATA[ ($PEOPLE_MAX*$PP_DATA_IDX_NUM) + $i ] = $line[$i];
      }
      $PEOPLE_MAX ++;
    }
  }
}



#情報の出力

#まずヘッダー
open( FILEOUT, ">".$ARGV[6] );

print( FILEOUT "// output resource/field_wfbc_data/wfbc_people_conv.pl\n" );
print( FILEOUT "#pragma once\n" );

print( FILEOUT "//-------------------------------------\n" );
print( FILEOUT "///	NPCIDの最大値\n" );
print( FILEOUT "//=====================================\n" );
print( FILEOUT "#define FIELD_WFBC_NPCID_MAX  ( $PEOPLE_MAX )\n" );

close( FILEOUT );


for( $i=0; $i<$PEOPLE_MAX; $i++ )
{
  if( $i < 10 )
  {
    open( FILEOUT, ">wfbc_people_0".$i.".bin" );
  }
  else
  {
    open( FILEOUT, ">wfbc_people_".$i.".bin" );
  }
  binmode( FILEOUT );

  $index = $i * $PP_DATA_IDX_NUM;
  
  #文字列を数値にしながら出力
  #OBJCODE
  $outnum = &OBJCODE_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_OBJCODE ] );
  print( FILEOUT pack( "S", $outnum ) );
  #TRTYPE
  $outnum = &TRTYPE_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_TRTYPE ] );
  print( FILEOUT pack( "S", $outnum ) );
  #MOOD WF
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_MOOD_WF ];
  print( FILEOUT pack( "S", $outnum ) );
  #MOOD BC
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_MOOD_BC ];
  print( FILEOUT pack( "S", $outnum ) );
  #ENC MONSNO
  $outnum = &MONSNO_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_ENC_MONSNO_00 ] );
  print( FILEOUT pack( "S", $outnum ) );
  $outnum = &MONSNO_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_ENC_MONSNO_01 ] );
  print( FILEOUT pack( "S", $outnum ) );
  $outnum = &MONSNO_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_ENC_MONSNO_02 ] );
  print( FILEOUT pack( "S", $outnum ) );
  print( FILEOUT pack( "S", 0 ) );  #PAD
  #ENC LV
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_ENC_LV_00 ];
  print( FILEOUT pack( "C", $outnum ) );
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_ENC_LV_01 ];
  print( FILEOUT pack( "C", $outnum ) );
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_ENC_LV_02 ];
  print( FILEOUT pack( "C", $outnum ) );
  print( FILEOUT pack( "C", 0 ) );  #PAD
  #ENC ％
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_ENC_PERCENT_00 ];
  print( FILEOUT pack( "C", $outnum ) );
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_ENC_PERCENT_01 ];
  print( FILEOUT pack( "C", $outnum ) );
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_ENC_PERCENT_02 ];
  print( FILEOUT pack( "C", $outnum ) );
  print( FILEOUT pack( "C", 0 ) );  #PAD
  #BTL MONSNO
  $outnum = &MONSNO_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_BTL_MONSNO_00 ] );
  print( FILEOUT pack( "S", $outnum ) );
  $outnum = &MONSNO_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_BTL_MONSNO_01 ] );
  print( FILEOUT pack( "S", $outnum ) );
  $outnum = &MONSNO_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_BTL_MONSNO_02 ] );
  print( FILEOUT pack( "S", $outnum ) );
  print( FILEOUT pack( "S", 0 ) );  #PAD
  #BTL LV
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_BTL_LV_00 ];
  print( FILEOUT pack( "C", $outnum ) );
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_BTL_LV_01 ];
  print( FILEOUT pack( "C", $outnum ) );
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_BTL_LV_02 ];
  print( FILEOUT pack( "C", $outnum ) );
  print( FILEOUT pack( "C", 0 ) ); #PAD
  #性別
  $outnum = &SEX_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_BTL_SEX_00 ] );
  print( FILEOUT pack( "C", $outnum ) );
  $outnum = &SEX_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_BTL_SEX_01 ] );
  print( FILEOUT pack( "C", $outnum ) );
  $outnum = &SEX_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_BTL_SEX_02 ] );
  print( FILEOUT pack( "C", $outnum ) );
  print( FILEOUT pack( "C", 0 ) );  #PAD
  #特性
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_BTL_TOKUSEI_00 ];
  print( FILEOUT pack( "C", $outnum ) );
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_BTL_TOKUSEI_01 ];
  print( FILEOUT pack( "C", $outnum ) );
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_BTL_TOKUSEI_02 ];
  print( FILEOUT pack( "C", $outnum ) );
  print( FILEOUT pack( "C", 0 ) ); #pad
  #スクリプト　WF
  $outnum = &SCRIPT_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_SCRIPT_WF_00 ] );
  print( FILEOUT pack( "S", $outnum ) );
  #スクリプト　BC
  $outnum = &SCRIPT_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_SCRIPT_BC_00 ] );
  print( FILEOUT pack( "S", $outnum ) );
  #道具 WF
  $outnum = &ITEMDEF_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_ITEM_WF ] );
  print( FILEOUT pack( "S", $outnum ) );
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_ITEM_PERCENT_WF ];
  print( FILEOUT pack( "S", $outnum ) );
  #道具 BC
  $outnum = &ITEMDEF_GetIdx( $PP_DATA[ $index + $PP_DATA_IDX_ITEM_BC ] );
  print( FILEOUT pack( "S", $outnum ) );
  $outnum = $PP_DATA[ $index + $PP_DATA_IDX_ITEM_MONEY_BC ];
  print( FILEOUT pack( "S", $outnum ) );
  
  close( FILEOUT );
}


exit(0);


sub OBJCODE_GetIdx
{
  my( $name ) = @_;
  my( $code, @codeline );

  #大文字化
  $name = uc( $name );

  foreach $code (@OBJCODE)
  {
    $code =~ s/ +/ /g;
    $code =~ s/ +/ /g;
    @codeline = split( /\s/, $code );
    if( "".$codeline[1] eq "".$name )
    {
      $codeline[2] =~ s/\(//;
      $codeline[2] =~ s/\)//;
      return hex( $codeline[2] )
    }
  }

  print( "objcode $name がみつかりません\n" );
  exit(1);
}


sub TRTYPE_GetIdx
{
  my( $name ) = @_;
  my( $code, @codeline );

  #大文字化
  $name = uc( $name );

  foreach $code (@TRTYPE_DEF)
  {
    $code =~ s/ +/ /g;
    $code =~ s/ +/ /g;
    @codeline = split( /\s/, $code );
    if( "".$codeline[1] eq "TRTYPE_".$name )
    {
      return $codeline[3];
    }
  }

  print( "trtype $name がみつかりません\n" );
  exit(1);
}

sub MONSNO_GetIdx
{
  my( $name ) = @_;
  my( $code, @codeline );

  #大文字化
  $name = uc( $name );

  foreach $code (@MONSNO_DEF)
  {
    $code =~ s/ +/ /g;
    $code =~ s/\t+/ /g;
    @codeline = split( /\s/, $code );
    if( "".$codeline[1] eq "MONSNO_".$name )
    {
      $codeline[3] =~ s/\(//;
      $codeline[3] =~ s/\)//;
      return $codeline[3];
    }
  }

  print( "monsno $name がみつかりません\n" );
  exit(1);
}

sub SCRIPT_GetIdx
{
  my( $name ) = @_;
  my( $code, @codeline );

  #大文字化
  $name = uc( $name );

  foreach $code (@SCRIPT)
  {
    $code =~ s/ +/ /g;
    $code =~ s/\t+/ /g;
    @codeline = split( /\s/, $code );
    if( "".$codeline[1] eq "".$name )
    {
      $codeline[2] =~ s/\(//;
      $codeline[2] =~ s/\)//;
      return $codeline[2];
    }
  }

  print( "script $name がみつかりません\n" );
  exit(1);
}

sub ITEMDEF_GetIdx
{
  my( $name ) = @_;
  my( $code, @codeline );

  #大文字化
  $name = uc( $name );

  foreach $code (@ITEM_DEF)
  {
    $code =~ s/ +/ /g;
    $code =~ s/\t+/ /g;
    @codeline = split( /\s/, $code );
    if( "".$codeline[3] eq "".$name )
    {
      $codeline[2] =~ s/\/\/No\.//;
      return $codeline[2];
    }
  }

  print( "item $name がみつかりません\n" );
  exit(1);
}

sub SEX_GetIdx
{
  my( $name ) = @_;
  my( $code, @codeline );

  
  if( "".$name eq "♂" )
  {
    return 0;
  }
  elsif ( "".$name eq "♀" )
  {
    return 1;
  }

  print( "sex $name がみつかりません\n" );
  exit(1);
}

