####################################################
#
#   conv.pl excel_tab tag_header arc_list
#
####################################################


#引数チェック
if( @ARGV < 3 )
{
  print( "conv.pl excel_tab tag_header arc_list\n" );
  exit(1);
}


#Excelファイル読み込み
@EXCEL_FILE = undef;
open( FILEIN, $ARGV[0] );
@EXCEL_FILE = <FILEIN>; 
close( FILEIN );


#-------------------------------------
# 内容
#=====================================
$PARA_TAG_IDX   = 0;
$PARA_XGRID_IDX = 1;
$PARA_ZGRID_IDX = 2;
$PARA_ATTR_IDX  = 3;
$PARA_MDL_IDX   = 4;
$PARA_MAX       = 5;


#情報を作成
@INPUT_DATA = undef;
$INPUT_DATA_MAX = 0;
$data_in = 0;
foreach $one ( @EXCEL_FILE )
{
  @line = split( /\t/, $one );
  
  if( $data_in == 0 )
  {
    if( "".$line[0] eq "TAG" )
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
      #データ格納
      for( $i=0; $i<$PARA_MAX; $i++ )
      {
        $INPUT_DATA[ ($INPUT_DATA_MAX * $PARA_MAX) + $i ] = $line[$i];
      }
      $INPUT_DATA_MAX ++;
    }
  }
}

#アークリストの出力
open( ARCLIST_FILE, ">".$ARGV[2] );

#ヘッダーの出力
open( HEADER_FILE, ">".$ARGV[1] );
print( HEADER_FILE "// output resource/fldmapdata/land_data_patch/conv.pl\n" );
print( HEADER_FILE "#pragma once\n\n" );


#アトリビュート情報をグリッド単位の情報に変更
#binaryとして固める
for( $i=0; $i<$INPUT_DATA_MAX; $i++ )
{
  $input_grid_x = $INPUT_DATA[ ($i * $PARA_MAX) + $PARA_XGRID_IDX ];
  $input_grid_z = $INPUT_DATA[ ($i * $PARA_MAX) + $PARA_ZGRID_IDX ];
  
  #.bin
  $filename = $INPUT_DATA[ ($i * $PARA_MAX) + $PARA_ATTR_IDX ];
  open( FILEIN, $filename );
  binmode( FILEIN );

  @data_s = unpack( "S*", <FILEIN> ); #unsigned shortでみる
  @data_c = unpack( "C*", <FILEIN> ); #unsigned charでみる

  close( FILEIN );

  #.3dmd
  $filename_3dmd = $INPUT_DATA[ ($i * $PARA_MAX) + $PARA_MDL_IDX ];
  open( FILEIN, $filename_3dmd );
  binmode( FILEIN );

  @data3dmd_c = unpack( "C*", <FILEIN> ); #unsigned charでみる

  close( FILEIN );

  #最初の2つはグリッド数
  $grid_x =  $data_s[0];
  $grid_z =  $data_s[1];
  #print( "gridx=$grid_x girdz=$grid_x\n" );

  #ファイルサイズから、１つのデータのサイズを計算
  ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
    $atime,$mtime,$ctime,$blksize,$blocks) = stat($filename);

  #1グリッドのデータサイズを計算
  $size     -= 4; #ヘッダー分を減らす
  $size_one = $size;
  $size_one /= ($grid_x * $grid_z);

  #print( "one_size = $size_one\n" );


  #出力ファイルに変換
  #binaryファイルをまとめて出力します。
  $filename = $INPUT_DATA[ ($i * $PARA_MAX) + $PARA_TAG_IDX ]."\.dat";

  #必要部分のみ出力しなおし
  open( FILEOUT, ">".$filename );
  binmode( FILEOUT );
  

  #各情報へのオフセット
  print( FILEOUT pack( "S", 4 ) );
  print( FILEOUT pack( "S", ($size + 4) ) );
  
  #グリッドサイズ
  print( FILEOUT pack( "S", $input_grid_x ) );
  print( FILEOUT pack( "S", $input_grid_z ) );

  #print( "input_grid_x = $input_grid_x\n" );
  #print( "input_grid_z = $input_grid_z\n" );

  #print( "ofs attr = 8\n" );
  #$size_ff = $size + 8;
  #print( "ofs 3dmd = ".$size_ff."\n" );

  #必要部分のみ出力
  for( $j=0; $j<$input_grid_z; $j++ )
  {
    for( $k=0; $k<$input_grid_x; $k++ )
    {
      $data_index = (($j * $grid_x) + $k) * $size_one;

      #サイズ分情報を出力
      for( $l=0; $l<$size_one; $l++ )
      {
        print( FILEOUT pack( "C", $data_c[ $data_index + $l ] ) );
      }

    }
  }

  #3dmdを出力
  print( FILEOUT pack( "C*", @data3dmd_c ) );
  
  close( FILEOUT );


  #ヘッダーに定義を追加
  print( HEADER_FILE "#define LAND_DATA_PATCH_".$INPUT_DATA[ ($i * $PARA_MAX) + $PARA_TAG_IDX ]." ( $i )\n" );

  #アークリストに追加
  print( ARCLIST_FILE "\"$filename\"\n" );
}

#ヘッダーに定義を追加
print( HEADER_FILE "#define LAND_DATA_PATCH_MAX ( $INPUT_DATA_MAX )\n" );



#アークリストの出力完了
close( ARCLIST_FILE );

#ヘッダーの出力完了
close( HEADER_FILE );

exit(0);


