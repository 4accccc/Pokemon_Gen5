#################################################
#
# WFBC イベントデータを出力
#
#  wfbc_event_conv.pl　tab_file land_patch_header bc_block_header wf_block_header out_bc_bin out_wf_bin
#
#
#################################################


#引数チェック
if( @ARGV < 6 )
{
  print( "wfbc_event_conv.pl tab_file land_patch_header bc_block_header wf_block_header out_bc_bin out_wf_bin\n" );
  exit(1);
}

#パラメータ
$PARA_TAG     = 0;
$PARA_IF_BLOCK_TAG  = 1;
$PARA_IF_BLOCK_ID   = 2;
$PARA_GRID_X  = 3;
$PARA_GRID_Z  = 4;
$PARA_SIZE_X  = 5;
$PARA_SIZE_Z  = 6;
$PARA_NextZoneID  = 7;
$PARA_NextDoorID  = 8;
$PARA_ExitDir     = 9;
$PARA_DoorType    = 10;
$PARA_MAX     = 11;




#エクセルデータ読み込み
open( FILEIN, $ARGV[0] );
@EXCEL_FILE = <FILEIN>;
close( FILEIN );

#land_data_patchヘッダー
open( FILEIN, $ARGV[1] );
@LAND_DATA_PATCH = <FILEIN>;
close( FILEIN );

#bc_block_header
open( FILEIN, $ARGV[2] );
@BC_BLOCK_HEADER = <FILEIN>;
close( FILEIN );

#wf_block_header
open( FILEIN, $ARGV[3] );
@WF_BLOCK_HEADER = <FILEIN>;
close( FILEIN );


#情報の取得
@EVENT_DATA_WF = undef;
$EVENT_DATA_WF_MAX = 0;

@EVENT_DATA_BC = undef;
$EVENT_DATA_BC_MAX = 0;

#情報
$data_in = 0;
$data_Type = 0; #最初がBC
foreach $one ( @EXCEL_FILE )
{
  $one =~ s/\r\n//g;
  $one =~ s/\n//g;

  @line = split( /\t/, $one );

  if( $data_in == 0 )
  {
    if( "".$line[0] =~ "TAG" )
    {
      $data_in = 1;
    }
  }
  else
  {
    if( "".$line[0] =~ "#END" )
    {
      $data_in = 0;
      $data_Type ++;
    }
    else
    {
      for( $i=0; $i<$PARA_MAX; $i++ )
      {
        if( $data_Type == 0 )
        {
          $EVENT_DATA_BC[ ($EVENT_DATA_BC_MAX * $PARA_MAX) + $i ] = $line[ $i ];
        }
        else
        {
          $EVENT_DATA_WF[ ($EVENT_DATA_WF_MAX * $PARA_MAX) + $i ] = $line[ $i ];
        }
      }

      if( $data_Type == 0 )
      {
        $EVENT_DATA_BC_MAX ++;
      }
      else
      {
        $EVENT_DATA_WF_MAX ++;
      }
    }
  }
}



#イベントの出現条件を出力
for( $i=0; $i<2; $i++ )
{

  open( FILEOUT, ">".$ARGV[ 4+$i ] );
  binmode( FILEOUT );


  if( $i == 0 )
  {
    for( $j=0; $j<$EVENT_DATA_BC_MAX; $j++ )
    {
      $data_index = $j * $PARA_MAX;
      print( FILEOUT pack( "S", &getBlackBlockTag( $EVENT_DATA_BC[$data_index + $PARA_IF_BLOCK_TAG] ) ) );
      print( FILEOUT pack( "S", &getLandDataPatchIdx( $EVENT_DATA_BC[$data_index + $PARA_IF_BLOCK_ID] ) ) );
    }
  }
  else
  {
    for( $j=0; $j<$EVENT_DATA_WF_MAX; $j++ )
    {
      $data_index = $j * $PARA_MAX;
      print( FILEOUT pack( "S", &getWhiteBlockTag( $EVENT_DATA_WF[$data_index + $PARA_IF_BLOCK_TAG] ) ) );
      print( FILEOUT pack( "S", &getLandDataPatchIdx( $EVENT_DATA_WF[$data_index + $PARA_IF_BLOCK_ID] ) ) );
    }
  }
  
  close( FILEOUT );
}

exit(0);



sub getLandDataPatchIdx
{
  my( $tag ) = @_;
  my( $label, @label_line );

  foreach $label ( @LAND_DATA_PATCH )
  {
    $label =~ s/\t/\s/g;
    
    @label_line = split( " ", $label );
    
    if( "".$label_line[1] eq "LAND_DATA_PATCH_".$tag )
    {
      return $label_line[3];
    }
  }

  print( "getLandDataPatchIdx $tag not found\n" );
  exit(1);
}


sub getBlackBlockTag
{
  my( $tag ) = @_;
  my( $label, @label_line );

  foreach $label ( @BC_BLOCK_HEADER )
  {
    $label =~ s/\t/\s/g;
    
    @label_line = split( " ", $label );

    if( "".$label_line[1] eq "".$tag )
    {
      return $label_line[3];
    }
  }
  
  print( "getBlackBlockTag $tag not found\n" );
  exit(1);
}

sub getWhiteBlockTag
{
  my( $tag ) = @_;
  my( $label, @label_line );

  foreach $label ( @WF_BLOCK_HEADER )
  {
    $label =~ s/\t/\s/g;
    
    @label_line = split( " ", $label );
    
    if( "".$label_line[1] eq "".$tag )
    {
      return $label_line[3];
    }
  }
  
  print( "getWhiteBlockTag $tag not found\n" );
  exit(1);
}

