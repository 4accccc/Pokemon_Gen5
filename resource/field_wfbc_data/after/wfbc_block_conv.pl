#############################################
#
# WFBC  配置ブロック情報生成
#
# 
#   wfbc_block_conv.pl tab_file land_data_patch output
#
#
#
#
#############################################


#引数数をチェック
if( @ARGV < 3 )
{
  print( "wfbc_block_conv.pl tab_file land_data_patch output\n" );
  exit(1);
}

#ブロックがない場所に格納するデータ
$BLOCK_NONE_NUM = 0xff;

#
# ブロック数情報
#
$BLOCK_DEF_X = 8; #ブロック横の数
$BLOCK_DEF_Z = 8; #ブロック縦の数

$BLOCK_TAG_PARA_TAG = 0;
$BLOCK_TAG_PARA_NUM00 = 1;
$BLOCK_TAG_PARA_NUM01 = 2;
$BLOCK_TAG_PARA_NUM02 = 3;
$BLOCK_TAG_PARA_NUM03 = 4;
$BLOCK_TAG_PARA_NUM04 = 5;
$BLOCK_TAG_PARA_NUM05 = 6;
$BLOCK_TAG_PARA_NUM06 = 7;
$BLOCK_TAG_PARA_NUM07 = 8;
$BLOCK_TAG_PARA_NUM08 = 9;
$BLOCK_TAG_PARA_NUM09 = 10;
$BLOCK_TAG_PARA_NUM10 = 11;
$BLOCK_TAG_PARA_NUM11 = 12;
$BLOCK_TAG_PARA_NUM12 = 13;
$BLOCK_TAG_PARA_NUM13 = 14;
$BLOCK_TAG_PARA_NUM14 = 15;
$BLOCK_TAG_PARA_NUM15 = 16;
$BLOCK_TAG_PARA_NUM16 = 17;
$BLOCK_TAG_PARA_NUM17 = 18;
$BLOCK_TAG_PARA_NUM18 = 19;
$BLOCK_TAG_PARA_NUM19 = 20;
$BLOCK_TAG_PARA_NUM20 = 21;
$BLOCK_TAG_PARA_MAX = 22;


#ファイル読み込み
open( FILEIN, $ARGV[0] );
@EXCEL_DATA = <FILEIN>;
close( FILEIN );



#land_data_patch読み込み
open( FILEIN, $ARGV[1] );
@LAND_DATA_PATCH = <FILEIN>;
close( FILEIN );



#ブロック情報を読み込む！
@BLOCK_DATA = undef;
$BLOCK_DATA_INDEX = 0;

@BLOCK_TAG = undef;
$BLOCK_TAG_INDEX = 0;

$data_in = 0;
$data_in_tag = 0;
foreach $one ( @EXCEL_DATA )
{
  $one =~ s/\r\n//g;
  $one =~ s/\n//g;


  @line = split( /\t/, $one );

  #配置情報の取得
  if( $data_in == 0 )
  {
    if( "".$line[0] eq "#START" )
    {
      $data_in = 1;
    }
  }
  else
  {
    if( "".$line[0] eq "#END" )
    {
      $data_in = 0;
    }
    else
    {
      for( $i=0; $i<$BLOCK_DEF_X; $i++ )
      {
        $BLOCK_DATA[ $BLOCK_DATA_INDEX ] = $line[1 + $i];
        $BLOCK_DATA_INDEX ++;
      }
    }
  }


  #ブロックタグ情報
  if( $data_in_tag == 0 )
  {
    if( "".$line[0] eq "TAG" )
    {
      $data_in_tag = 1;
    }
  }
  else
  {
    if( "".$line[0] eq "#END" )
    {
      $data_in_tag = 0;
    }
    else
    {
      for( $i=0; $i<$BLOCK_TAG_PARA_MAX; $i++ )
      {
        $BLOCK_TAG[ ($BLOCK_TAG_PARA_MAX * $BLOCK_TAG_INDEX) + $i ] = $line[$i];
      }
      $BLOCK_TAG_INDEX ++;
    }
  }
}


#情報の出力
open( FILEOUT, ">".$ARGV[2] );
binmode( FILEOUT );

#配置情報を出力
for( $i=0; $i<$BLOCK_DATA_INDEX; $i++ )
{
  #print( "block idx $i = ".$BLOCK_DATA[ $i ]."\n" );
  print( FILEOUT pack( "C", &getTagIndex( $BLOCK_DATA[ $i ] ) ) ); 
}

#タグデータを出力
for( $i=0; $i<$BLOCK_TAG_INDEX; $i++ )
{
  for( $j=1; $j<$BLOCK_TAG_PARA_MAX; $j++ )
  {
    #print( "block idx $i $j = ".$BLOCK_TAG[ ($i*$BLOCK_TAG_PARA_MAX) + $j ]."\n" );
    print( FILEOUT pack( "I", &getBlockIndex( $BLOCK_TAG[ ($i*$BLOCK_TAG_PARA_MAX) + $j ] ) ) ); 
  }
}


close( FILEOUT );

exit(0);





#タグインデックスの取得
sub getTagIndex
{
  my( $tag ) = @_;
  my( $one, $index, $i );

  for( $i=0; $i<$BLOCK_TAG_INDEX; $i++ )
  {
    if( "".$BLOCK_TAG[($i * $BLOCK_TAG_PARA_MAX) + $BLOCK_TAG_PARA_TAG] eq "".$tag )
    {
      return $i;
    }
  }

  #知らない場合は、NONEを返す
  return $BLOCK_NONE_NUM;
}


#ブロックインデックスの取得
sub getBlockIndex
{
  my( $block_idx ) = @_;
  my( $one, $index, $i, @line );

  foreach $one ( @LAND_DATA_PATCH )
  {
    $one =~ s/\r\n//g;
    $one =~ s/\n//g;

    @line = split( /\s/, $one );

    if( "LAND_DATA_PATCH_".$block_idx  eq "".$line[1] )
    {
      return $line[ 3 ];
    }
  }

  print( "blockの定義がみあたりません $block_idx \n" );
  exit(1);
}



