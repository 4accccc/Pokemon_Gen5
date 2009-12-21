#################################################
#
# conv.pl tab_file output
#   エクセル内容をtools/rail_editor/scenearea_conv.pl
#   でコンバートできる形式にして出力
#
#
#################################################

if( @ARGV < 2 )
{
  print( "conv.pl tab_file output\n" );
  exit(1);
}


#パラメータ定義
$PARA_TAG = 0;
$PARA_AREA_GRID_X = 1;
$PARA_AREA_GRID_Z = 2;
$PARA_AREA_GRID_SIZX = 3;
$PARA_AREA_GRID_SIZZ = 4;
$PARA_AREA_GRID_DEPTH = 5;
$PARA_AREA_GRID_IN_PITCH =6;
$PARA_AREA_GRID_IN_YAW = 7;
$PARA_AREA_GRID_IN_LEN = 8;
$PARA_AREA_GRID_OUT_PITCH = 9;
$PARA_AREA_GRID_OUT_YAW = 10;
$PARA_AREA_GRID_OUT_LEN = 11;
$PARA_AREA_MAX = 12;


#エクセル情報取得
@EXCEL_FILE = undef;
open( FILEIN, $ARGV[0] );
@EXCEL_FILE = <FILEIN>;
close( FILEIN );


#アングルチェンジ　シーン　の情報を取得
@GRID_ANGLE_CHANGE = undef;
$GRID_ANGLE_CHANGE_MAX  = 0;
$data_in = 0;
foreach $one ( @EXCEL_FILE )
{
  $one =~ s/\r\n//g;
  $one =~ s/\n//g;

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
      #情報入手
      for( $i=0; $i<$PARA_AREA_MAX; $i++ )
      {
        $GRID_ANGLE_CHANGE[ ($GRID_ANGLE_CHANGE_MAX * $PARA_AREA_MAX) + $i ] = $line[$i];
      }
      $GRID_ANGLE_CHANGE_MAX ++;
    }
  }
}



#取得情報を使って情報を出力
open( FILEOUT, ">".$ARGV[1] );
  
for( $i=0; $i<$GRID_ANGLE_CHANGE_MAX; $i++ )
{
  $index = $i * $PARA_AREA_MAX;
  $output = 0;
  
  print( FILEOUT "+SA\n" );
  print( FILEOUT "--NAME::".$GRID_ANGLE_CHANGE[ $index+$PARA_TAG ]."\n" );
  print( FILEOUT "--AREACHECK::FLD_SCENEAREA_AREACHECK_GRID_RECT\n" );
  print( FILEOUT "--UPDATE::FLD_SCENEAREA_UPDATE_ANGLECHANGE\n" );
  print( FILEOUT "--INSIDE::FLD_SCENEAREA_UPDATE_ANGLECHANGE_IN\n" );
  print( FILEOUT "--OUTSIDE::FLD_SCENEAREA_UPDATE_ANGLECHANGE_OUT\n" );
  print( FILEOUT "--2BYTE::".$GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_X ]."\n" );
  print( FILEOUT "--2BYTE::".$GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_Z ]."\n" );
  print( FILEOUT "--2BYTE::".$GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_SIZX ]."\n" );
  print( FILEOUT "--2BYTE::".$GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_SIZZ ]."\n" );
  print( FILEOUT "--2BYTE::".$GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_DEPTH ]."\n" );
  print( FILEOUT "--2BYTE::0\n" );  #パディング
  $output = hex($GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_IN_PITCH ]);
  print( FILEOUT "--2BYTE::".$output."\n" );
  $output = hex($GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_IN_YAW ]);
  print( FILEOUT "--2BYTE::".$output."\n" );
  $output = hex($GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_IN_LEN ]);
  print( FILEOUT "--4BYTE::".$output."\n" );
  $output = hex($GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_OUT_PITCH ]);
  print( FILEOUT "--2BYTE::".$output."\n" );
  $output = hex($GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_OUT_YAW ]);
  print( FILEOUT "--2BYTE::".$output."\n" );
  $output = hex($GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_OUT_LEN ]);
  print( FILEOUT "--4BYTE::".$output."\n" );
}


close( FILEOUT );


