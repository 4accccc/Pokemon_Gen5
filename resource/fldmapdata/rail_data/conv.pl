#################################################
#
# レール構成データのコンバート
# 
#   conv.pl   レールデータ rail_dataのnaix  sceneareaのnaix
#
#################################################

$NONE_NUM = 0xffff; 
$DATA_OUT_SIZE = "S";  #packに渡す文字

if( @ARGV != 3 )
{
  print( "conv.pl   レールデータ rail_dataのnaix  sceneareaのnaix\n" );
  exit(1);
}

@RAIL_DATA_NAIX = undef;
@AREA_DATA_NAIX = undef;


#読み込み
open( FILEIN, $ARGV[0] );
@input_file = <FILEIN>;
close( FILEIN );

#rail_dataのNAIX読み込み
&loadNaix( $ARGV[1], \@RAIL_DATA_NAIX );

#sceneareaのNAIX読み込み
&loadNaix( $ARGV[2], \@AREA_DATA_NAIX );


$TAG = 0;
$RAIL_ID = 0;
$SCENE_ID = 0;

$data_in = 0;
foreach $one ( @input_file )
{
  $one =~ s/\r\n//;
  $one =~ s/\n//;
  
  @data_one = split( /\t/, $one );

  if( $data_in == 0 )
  {
    if( $data_one[0] =~ /#START/ )
    {
      $data_in = 1;
    }
  }
  elsif( $data_in == 1 )
  {
    if( $data_one[0] =~ /#END/ )
    {
      $data_in = 2;
    }
    else
    {
    
      $TAG        = $data_one[0];
      $data_one[1] =~ s/\.rail/_dat/;
      $data_one[2] =~ s/\.sa/_dat/;
      $RAIL_ID    = &getNaixIndex( \@RAIL_DATA_NAIX, $data_one[1] );
      $SCENE_ID   = &getNaixIndex( \@AREA_DATA_NAIX, $data_one[2] );


      #出力
      open( FILEOUT, ">".$TAG.".bin" );
      binmode( FILEOUT );

      print( FILEOUT pack( $DATA_OUT_SIZE, $RAIL_ID ) );
      print( FILEOUT pack( $DATA_OUT_SIZE, $SCENE_ID ) );
      
      close( FILEOUT );
    }
  }
}


exit(0);








## naixの読み込み
sub loadNaix
{
  my( $filename, $buff ) = @_;
  my( $load_one, @naix_data, $naix_data_in, $naix_count );
  
  open( NAIXFILE, $filename );
  @naix_data = <NAIXFILE>;
  close( NAIXFILE );

  $naix_data_in = 0;
  $naix_count = 0;
  foreach $load_one ( @naix_data )
  {
    if( $naix_data_in == 0 )
    {
      if( $load_one =~ /{/ )
      {
        $naix_data_in = 1;
      }
    }
    elsif( $naix_data_in == 1 )
    {
      if( $load_one =~ /}/ )
      {
        $naix_data_in = 2;
      }
      else
      {
        if( $load_one =~ /[\s\t]*([^\s]*) = / )
        {
          $$buff[ $naix_count ] = $1;
          $naix_count ++;
        }
      }
    }
  }
}


sub getNaixIndex
{
  my( $buff, $sarch ) = @_;
  my( $sarch_one, $sarch_count );

  if( $sarch =~ /NONE/ )
  {
    return $NONE_NUM;
  }

  $sarch_count = 0;
  foreach $sarch_one ( @$buff )
  {
    if( $sarch_one =~ /$sarch/ )
    {
      return $sarch_count;
    }

    $sarch_count++;
  }

  print( "$sarch none\n" );
  exit(1);
}




