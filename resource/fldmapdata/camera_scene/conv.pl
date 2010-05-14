#################################################
#
# conv.pl tab_file output
#   エクセル内容をtools/rail_editor/scenearea_conv.pl
#   でコンバートできる形式にして出力
#
#
#################################################

#ARGVインデックス
$ARGV_INDEX_TAB1 = 0;
$ARGV_INDEX_TAB2 = 1;
$ARGV_INDEX_OUTPUT = 2;
$ARGV_INDEX_MAX = 3;

if( @ARGV < $ARGV_INDEX_MAX )
{
  print( "conv.pl tab_file1 tab_file2 output\n" );
  exit(1);
}




#-------------------------------------------------------------
# アングル　変更
#-------------------------------------------------------------
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
$PARA_AREA_GRID_TRACE_OFF = 12;
$PARA_AREA_MAX = 13;


#エクセル情報取得
@EXCEL_FILE = undef;
open( FILEIN, $ARGV[$ARGV_INDEX_TAB1] );
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



#-------------------------------------------------------------
# アングル　ターゲットオフセット　変更
#-------------------------------------------------------------
#パラメータ定義
$PARA_AREA_GRID_OFFS_TAG = 0;
$PARA_AREA_GRID_OFFS_X = 1;
$PARA_AREA_GRID_OFFS_Z = 2;
$PARA_AREA_GRID_OFFS_SIZX = 3;
$PARA_AREA_GRID_OFFS_SIZZ = 4;
$PARA_AREA_GRID_OFFS_DEPTH = 5;
$PARA_AREA_GRID_OFFS_IN_PITCH =6;
$PARA_AREA_GRID_OFFS_IN_YAW = 7;
$PARA_AREA_GRID_OFFS_IN_LEN = 8;
$PARA_AREA_GRID_OFFS_IN_OFFS_X =9;
$PARA_AREA_GRID_OFFS_IN_OFFS_Y = 10;
$PARA_AREA_GRID_OFFS_IN_OFFS_Z = 11;
$PARA_AREA_GRID_OFFS_IN_FOVY = 12;
$PARA_AREA_GRID_OFFS_OUT_PITCH = 13;
$PARA_AREA_GRID_OFFS_OUT_YAW = 14;
$PARA_AREA_GRID_OFFS_OUT_LEN = 15;
$PARA_AREA_GRID_OFFS_OUT_OFFS_X =16;
$PARA_AREA_GRID_OFFS_OUT_OFFS_Y = 17;
$PARA_AREA_GRID_OFFS_OUT_OFFS_Z = 18;
$PARA_AREA_GRID_OFFS_OUT_FOVY = 19;
$PARA_AREA_GRID_OFFS_TRACE_OFF = 20;
$PARA_AREA_GRID_OFFS_MAX = 21;

#エクセル情報取得
@EXCEL_FILE = undef;
open( FILEIN, $ARGV[$ARGV_INDEX_TAB2] );
@EXCEL_FILE = <FILEIN>;
close( FILEIN );


#アングル ターゲットオフセット　チェンジ　シーン　の情報を取得
@GRID_ANGLEOFFS_CHANGE = undef;
$GRID_ANGLEOFFS_CHANGE_MAX  = 0;
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
      for( $i=0; $i<$PARA_AREA_GRID_OFFS_MAX; $i++ )
      {
        $GRID_ANGLEOFFS_CHANGE[ ($GRID_ANGLEOFFS_CHANGE_MAX * $PARA_AREA_GRID_OFFS_MAX) + $i ] = $line[$i];
      }
      $GRID_ANGLEOFFS_CHANGE_MAX ++;
    }
  }
}



#-------------------------------------------------------------
# 出力
#-------------------------------------------------------------
if( ($GRID_ANGLE_CHANGE_MAX + $GRID_ANGLEOFFS_CHANGE_MAX) <= 0 ){
  print( "データが１つもありません。\n" );
  exit(1);
}

#取得情報を使って情報を出力
open( FILEOUT, ">".$ARGV[$ARGV_INDEX_OUTPUT] );

  
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
  print( FILEOUT "--4BYTE::".$GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_TRACE_OFF ]."\n" );
}

for( $i=0; $i<$GRID_ANGLEOFFS_CHANGE_MAX; $i++ )
{
  $index = $i * $PARA_AREA_GRID_OFFS_MAX;
  $output = 0;
  
  print( FILEOUT "+SA\n" );
  print( FILEOUT "--NAME::ANGLE_OFFS_".$GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_TAG ]."\n" );
  print( FILEOUT "--AREACHECK::FLD_SCENEAREA_AREACHECK_GRID_RECT\n" );
  print( FILEOUT "--UPDATE::FLD_SCENEAREA_UPDATE_ANGLEOFFSCHANGE\n" );
  print( FILEOUT "--INSIDE::FLD_SCENEAREA_UPDATE_ANGLEOFFSCHANGE_IN\n" );
  print( FILEOUT "--OUTSIDE::FLD_SCENEAREA_UPDATE_ANGLEOFFSCHANGE_OUT\n" );
  print( FILEOUT "--2BYTE::".$GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_X ]."\n" );
  print( FILEOUT "--2BYTE::".$GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_Z ]."\n" );
  print( FILEOUT "--2BYTE::".$GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_SIZX ]."\n" );
  print( FILEOUT "--2BYTE::".$GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_SIZZ ]."\n" );
  print( FILEOUT "--2BYTE::".$GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_DEPTH ]."\n" );
  print( FILEOUT "--2BYTE::0\n" );  #パディング
  
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_IN_PITCH ]);
  print( FILEOUT "--2BYTE::".$output."\n" );
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_IN_YAW ]);
  print( FILEOUT "--2BYTE::".$output."\n" );
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_IN_LEN ]);
  print( FILEOUT "--4BYTE::".$output."\n" );
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_OUT_PITCH ]);
  print( FILEOUT "--2BYTE::".$output."\n" );
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_OUT_YAW ]);
  print( FILEOUT "--2BYTE::".$output."\n" );
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_OUT_LEN ]);
  print( FILEOUT "--4BYTE::".$output."\n" );
  print( FILEOUT "--4BYTE::".$GRID_ANGLE_CHANGE[ $index+$PARA_AREA_GRID_OFFS_TRACE_OFF ]."\n" );


  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_IN_OFFS_X ]);
  print( FILEOUT "--4BYTE::".$output."\n" );
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_IN_OFFS_Y ]);
  print( FILEOUT "--4BYTE::".$output."\n" );
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_IN_OFFS_Z ]);
  print( FILEOUT "--4BYTE::".$output."\n" );
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_OUT_OFFS_X ]);
  print( FILEOUT "--4BYTE::".$output."\n" );
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_OUT_OFFS_Y ]);
  print( FILEOUT "--4BYTE::".$output."\n" );
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_OUT_OFFS_Z ]);
  print( FILEOUT "--4BYTE::".$output."\n" );

  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_IN_FOVY ]);
  print( FILEOUT "--2BYTE::".$output."\n" );
  $output = hex($GRID_ANGLEOFFS_CHANGE[ $index+$PARA_AREA_GRID_OFFS_OUT_FOVY ]);
  print( FILEOUT "--2BYTE::".$output."\n" );
}


close( FILEOUT );


