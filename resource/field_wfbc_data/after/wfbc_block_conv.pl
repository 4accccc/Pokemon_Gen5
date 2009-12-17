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
$BLOCK_TAG_PARA_SIZ_X = 1;
$BLOCK_TAG_PARA_SIZ_Z = 2;
$BLOCK_TAG_PARA_NUM00 = 3;
$BLOCK_TAG_PARA_NUM01 = 4;
$BLOCK_TAG_PARA_NUM02 = 5;
$BLOCK_TAG_PARA_NUM03 = 6;
$BLOCK_TAG_PARA_NUM04 = 7;
$BLOCK_TAG_PARA_NUM05 = 8;
$BLOCK_TAG_PARA_NUM06 = 9;
$BLOCK_TAG_PARA_NUM07 = 10;
$BLOCK_TAG_PARA_NUM08 = 11;
$BLOCK_TAG_PARA_NUM09 = 12;
$BLOCK_TAG_PARA_NUM10 = 13;
$BLOCK_TAG_PARA_NUM11 = 14;
$BLOCK_TAG_PARA_NUM12 = 15;
$BLOCK_TAG_PARA_NUM13 = 16;
$BLOCK_TAG_PARA_NUM14 = 17;
$BLOCK_TAG_PARA_NUM15 = 18;
$BLOCK_TAG_PARA_MAX = 19;


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


#ブロック配置情報
#X　Z　ブロックインデックス　セットの情報を作成
$BLOCK_ALL_DATA_PARA_SIZE_X = 0;
$BLOCK_ALL_DATA_PARA_SIZE_Z = 1;
$BLOCK_ALL_DATA_PARA_TAG    = 2;
$BLOCK_ALL_DATA_PARA_MAX    = 3;

#情報
@BLOCK_ALL_DATA = ( 
  0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 
  0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 
  0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 
  0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 
  0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 
  0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 
  0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 
  0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM, 0,0,$BLOCK_NONE_NUM
);

#配置情報を作成
for( $i=0; $i<$BLOCK_DATA_INDEX; $i++ )
{
  $tag_index = &getTagIndex( $BLOCK_DATA[ $i ] );
  if( $tag_index != $BLOCK_NONE_NUM )
  {
    $block_x = $BLOCK_TAG[ ($tag_index*$BLOCK_TAG_PARA_MAX) + $BLOCK_TAG_PARA_SIZ_X ];
    $block_z = $BLOCK_TAG[ ($tag_index*$BLOCK_TAG_PARA_MAX) + $BLOCK_TAG_PARA_SIZ_Z ];

    if( $block_x >= 8 )
    {
      print( "ブロックサイズオーバー ブロックNo=$tag_index\n" );
      exit(1);
    }
    if( $block_z >= 8 )
    {
      print( "ブロックサイズオーバー ブロックNo=$tag_index\n" );
      exit(1);
    }

    #print( "block x $block_x  block z $block_z \n" );

    $block_idx_z = int($i/8);
    $block_idx_x = $i%8;
    
    #範囲内に、データを設定
    for( $z = 0; $z<$block_z; $z++ )
    {
      for( $x = 0; $x<$block_x; $x++ )
      {
        $block_all_index = ((($z+$block_idx_z) * 8) + ($x+$block_idx_x)) * $BLOCK_ALL_DATA_PARA_MAX;

        #print( "block index $block_all_index tag $tag_index \n" );
        
        $BLOCK_ALL_DATA[ $block_all_index + $BLOCK_ALL_DATA_PARA_SIZE_X ] = $x;
        $BLOCK_ALL_DATA[ $block_all_index + $BLOCK_ALL_DATA_PARA_SIZE_Z ] = $z;
        $BLOCK_ALL_DATA[ $block_all_index + $BLOCK_ALL_DATA_PARA_TAG ] = $tag_index;
      }
    }
  }
}



#情報の出力
open( FILEOUT, ">".$ARGV[2] );
binmode( FILEOUT );

#配置情報を出力
for( $i=0; $i<64; $i++ )  # 8 * 8 で 64
{
  $pos = ($BLOCK_ALL_DATA[($i*$BLOCK_ALL_DATA_PARA_MAX) + $BLOCK_ALL_DATA_PARA_SIZE_X]) | ($BLOCK_ALL_DATA[($i*$BLOCK_ALL_DATA_PARA_MAX) + $BLOCK_ALL_DATA_PARA_SIZE_Z] << 4);

  #print( "block tag".$BLOCK_ALL_DATA[($i*$BLOCK_ALL_DATA_PARA_MAX) + $BLOCK_ALL_DATA_PARA_TAG]." pos x".$BLOCK_ALL_DATA[($i*$BLOCK_ALL_DATA_PARA_MAX) + $BLOCK_ALL_DATA_PARA_SIZE_X]." pos z".$BLOCK_ALL_DATA[($i*$BLOCK_ALL_DATA_PARA_MAX) + $BLOCK_ALL_DATA_PARA_SIZE_Z]."\n" );
  
  print( FILEOUT pack( "C", $pos ) ); 
  print( FILEOUT pack( "C", $BLOCK_ALL_DATA[($i*$BLOCK_ALL_DATA_PARA_MAX) + $BLOCK_ALL_DATA_PARA_TAG] ) ); 
}


#タグデータを出力
for( $i=0; $i<$BLOCK_TAG_INDEX; $i++ )
{
  for( $j=$BLOCK_TAG_PARA_NUM00; $j<$BLOCK_TAG_PARA_MAX; $j++ )
  {
    #print( "block idx $i $j = ".$BLOCK_TAG[ ($i*$BLOCK_TAG_PARA_MAX) + $j ]."\n" );
    print( FILEOUT pack( "I", &getBlockIndex( $BLOCK_TAG[ ($i*$BLOCK_TAG_PARA_MAX) + $j ] ) ) ); 
  }
}


close( FILEOUT );






#BLOCK_TAGヘッダー生成
$filename = $ARGV[2];
$filename =~ s/\..*/.h/g;
open( FILEOUT, ">".$filename );

for( $i=0; $i<$BLOCK_TAG_INDEX; $i++ )
{
  print( FILEOUT "#define ".$BLOCK_TAG[ ($i*$BLOCK_TAG_PARA_MAX) + $BLOCK_TAG_PARA_TAG ]." ( $i )\n" );
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



