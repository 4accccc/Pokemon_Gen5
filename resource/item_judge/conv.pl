################################################
#
#   鑑定士データコンバーター
#
#     conv.pl out.tab item_header
#
#
#
#
################################################

if( @ARGV < 2 ){
  print( "perl conv.pl out.tab item_header\n" ); 
  exit(1);
}


#ファイル読み込み
open( FILEIN, $ARGV[0] );
@EXCEL_FILE = <FILEIN>;
close( FILEIN );

open( FILEIN, $ARGV[1] );
@ITEM_HEADER = <FILEIN>;
close( FILEIN );


#読み込み情報
@ITEM_LIST = undef;
@GOURMET_LIST = undef;        #グルメ　お金データ
@STONE_MANIA_LIST = undef;    #石マニアお金データ
@RICH_LIST = undef;           #大富豪　お金データ

$DATA_NUM = 0;



$data_in = 0;

foreach $one ( @EXCEL_FILE )
{
  $one =~ s/\r\n/\t/g;
  $one =~ s/\n/\t/g;


  @line = split( /\t/, $one );

  if( $data_in == 0 )
  {
    if( "".$line[1] eq "道具名" ){
      $data_in = 1;
    }
  }
  elsif( $data_in == 1 )
  {
    if( "".$line[1] eq "#END" ){
      $data_in = 0;
    }else{

      $ITEM_LIST[ $DATA_NUM ] = $line[1];
      $GOURMET_LIST[ $DATA_NUM ] = $line[2];
      $STONE_MANIA_LIST[ $DATA_NUM ] = $line[3];
      $RICH_LIST[ $DATA_NUM ] = $line[4];
      $DATA_NUM ++;
    }
  }
}

#アイテムを数値に変更
for( $i=0; $i<$DATA_NUM; $i++ ){
  $ITEM_LIST[ $i ] = &GET_ItemNo( $ITEM_LIST[ $i ] );
}



#各人のアイテムと金額を出力
open( FILEOUT, ">gourmet.dat" );
binmode( FILEOUT );
&DEBUG_Puts("gourmet\n");
for( $i=0; $i<$DATA_NUM; $i++ ){
  if( $GOURMET_LIST[ $i ] > 0 ){
    &DEBUG_Puts($ITEM_LIST[ $i ]."\n");
    &DEBUG_Puts($GOURMET_LIST[ $i ]."\n");
    print( FILEOUT pack( "S", $ITEM_LIST[ $i ] ) );
    print( FILEOUT pack( "S", $GOURMET_LIST[ $i ] ) );
  }
}
close( FILEOUT );


open( FILEOUT, ">stone_mania.dat" );
binmode( FILEOUT );
for( $i=0; $i<$DATA_NUM; $i++ ){
  if( $STONE_MANIA_LIST[ $i ] > 0 ){
    &DEBUG_Puts($ITEM_LIST[ $i ]."\n");
    &DEBUG_Puts($STONE_MANIA_LIST[ $i ]."\n");
    print( FILEOUT pack( "S", $ITEM_LIST[ $i ] ) );
    print( FILEOUT pack( "S", $STONE_MANIA_LIST[ $i ] ) );
  }
}
close( FILEOUT );

open( FILEOUT, ">rich.dat" );
binmode( FILEOUT );
for( $i=0; $i<$DATA_NUM; $i++ ){
  if( $RICH_LIST[ $i ] > 0 ){
    &DEBUG_Puts($ITEM_LIST[ $i ]."\n");
    &DEBUG_Puts($RICH_LIST[ $i ]."\n");
    print( FILEOUT pack( "S", $ITEM_LIST[ $i ] ) );
    print( FILEOUT pack( "S", $RICH_LIST[ $i ] ) );
  }
}
close( FILEOUT );


exit(0);


sub GET_ItemNo
{
  my( $item ) = @_;
  my( $item_def, $item_count, $item_no );
  

  $item =~ s/\s//g;

  for( $item_count=0; $item_count<@ITEM_HEADER; $item_count++ ){
    #//comment　部分を抽出
    $item_def = $ITEM_HEADER[$item_count];

    if( $item_def =~ /\( ([0-9]*) \)\t\t\/\/(.*)$/ ){

      $item_no = $1;
      $item_def = $2;
      $item_def =~ s/\s//g;
      &DEBUG_Puts( "$item_def == $item  $item_no\n" );
      if( "".$item_def eq "".$item ){
        &DEBUG_Puts( "return \n" );
        return $item_no;
      }
    }
  }

  print( "$item がみつからない\n" );
  exit(1);
}

sub DEBUG_Puts
{
  my( $str ) = @_;
  #print( $str );
}
