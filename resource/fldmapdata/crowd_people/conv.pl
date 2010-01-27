##########################################################
#
#   conv.pl   xcel_tab  objcode dir zone_id output_list output_arc
#
##########################################################


if( @ARGV < 6 )
{
  print( "conv.pl   xcel_tab  objcode dir zone_id output_list output_arc\n" );
  exit(1);
}


#情報の読み込み
open( FILEIN, $ARGV[0] );
@EXCEL_FILE = <FILEIN>;
close( FILEIN );

open( FILEIN, $ARGV[1] );
@OBJCODE = <FILEIN>;
close( FILEIN );

open( FILEIN, $ARGV[2] );
@DIR_DATA = <FILEIN>;
close( FILEIN );

open( FILEIN, $ARGV[3] );
@ZONE_ID = <FILEIN>;
close( FILEIN );


$DATA_NUM = 0;
@DATA_ZONEID = undef;
@DATA_WAIT = undef;
@DATA_POINT_NUM = undef;
@DATA_OBJCODE_NUM = undef;
@DATA_OBJCODE00 = undef;
@DATA_OBJCODE01 = undef;
@DATA_OBJCODE02 = undef;
@DATA_OBJCODE03 = undef;
@DATA_POINT00_LEFT = undef;
@DATA_POINT00_RIGHT = undef;
@DATA_POINT00_TOP = undef;
@DATA_POINT00_BOTTOM = undef;
@DATA_POINT00_DIR = undef;
@DATA_POINT00_GRID_NUM = undef;
@DATA_POINT01_LEFT = undef;
@DATA_POINT01_RIGHT = undef;
@DATA_POINT01_TOP = undef;
@DATA_POINT01_BOTTOM = undef;
@DATA_POINT01_DIR = undef;
@DATA_POINT01_GRID_NUM = undef;



#情報を収集
$data_in = 0;
foreach $one ( @EXCEL_FILE )
{
  #\r\nを消す
  $one =~ s/\r\n//g;
  $one =~ s/\n//g;

  @line = split( /\t/, $one );

  if( $data_in == 0 )
  {
    if( "".$line[0] eq "#START" )
    {
      $data_in = 1;
      $data_count = 0;

      #情報数をチェック
      while( "".$line[1+$DATA_NUM] ne "#END" )
      {
        $DATA_NUM ++;
      }
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
      if( $data_count == 0 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_ZONEID[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 1 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_WAIT[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 2 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT_NUM[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 3 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_OBJCODE_NUM[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 4 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_OBJCODE00[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 5 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_OBJCODE01[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 6 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_OBJCODE02[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 7 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_OBJCODE03[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 8 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_LEFT[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 9 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_RIGHT[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 10 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_TOP[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 11 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_BOTTOM[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 12 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_DIR[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 13 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_GRID_NUM[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 14 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_LEFT[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 15 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_RIGHT[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 16 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_TOP[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 17 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_BOTTOM[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 18 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_DIR[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == 19 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_GRID_NUM[ $i ] = $line[1 + $i];
        }
      }
      
      $data_count ++;
    }
  }
}


open( OUTLIST, ">".$ARGV[5] );


#出力
for( $i=0; $i<$DATA_NUM; $i++ )
{
  $file_name = lc($DATA_ZONEID[$i]).".bin";
  print( OUTLIST "\"".$file_name."\"\n" );
  
  open( FILEOUT, ">".$file_name );
  binmode( FILEOUT );
  
  print( FILEOUT pack( "s", $DATA_WAIT[$i] ) );
  print( FILEOUT pack( "C", $DATA_POINT_NUM[$i] ) );
  print( FILEOUT pack( "C", $DATA_OBJCODE_NUM[$i] ) );
  print( FILEOUT pack( "S", &getOBJCODE($DATA_OBJCODE00[$i]) ) );
  print( FILEOUT pack( "S", &getOBJCODE($DATA_OBJCODE01[$i]) ) );
  print( FILEOUT pack( "S", &getOBJCODE($DATA_OBJCODE02[$i]) ) );
  print( FILEOUT pack( "S", &getOBJCODE($DATA_OBJCODE03[$i]) ) );
  print( FILEOUT pack( "S", $DATA_POINT00_TOP[$i] ) );
  print( FILEOUT pack( "S", $DATA_POINT00_BOTTOM[$i] ) );
  print( FILEOUT pack( "S", $DATA_POINT00_LEFT[$i] ) );
  print( FILEOUT pack( "S", $DATA_POINT00_RIGHT[$i] ) );
  print( FILEOUT pack( "S", &getDIR($DATA_POINT00_DIR[$i]) ) );
  print( FILEOUT pack( "S", $DATA_POINT00_GRID_NUM[$i] ) );
  print( FILEOUT pack( "S", $DATA_POINT01_TOP[$i] ) );
  print( FILEOUT pack( "S", $DATA_POINT01_BOTTOM[$i] ) );
  print( FILEOUT pack( "S", $DATA_POINT01_LEFT[$i] ) );
  print( FILEOUT pack( "S", $DATA_POINT01_RIGHT[$i] ) );
  print( FILEOUT pack( "S", &getDIR($DATA_POINT01_DIR[$i]) ) );
  print( FILEOUT pack( "S", $DATA_POINT01_GRID_NUM[$i] ) );

  close( FILEOUT );
}

#リストの出力
open( FILEOUT, ">".$ARGV[4] );
binmode( FILEOUT );


print( OUTLIST "\"".$ARGV[4]."\"\n" );

for( $i=0; $i<$DATA_NUM; $i++ )
{
  print( FILEOUT pack( "I", &getZONEID( $DATA_ZONEID[$i] ) ) );
}

close( FILEOUT );




close( OUTLIST );


exit(0);



sub getZONEID
{
  my( $data ) = @_;
  my( $zone, @line );

  foreach $zone ( @ZONE_ID )
  {
    $zone =~ s/\(//g;
    $zone =~ s/\)//g;
    $zone =~ s/ +/ /g;
    $zone =~ s/\t+/ /g;
    $zone =~ s/\r\n//g;
    $zone =~ s/\n//g;
    
    @line = split( /\s/, $zone );
    
    if( "".$line[1] eq "".$data )
    {
      return $line[2];
    }
  }

  print( "ZONEIDから $data が見つかりません。\n" );
  exit(1);
}


sub getOBJCODE
{
  my( $data ) = @_;
  my( $objcode, @line );

  if( "".$data eq "NONE" ) 
  {
    return 0xff;
  }

  foreach $objcode ( @OBJCODE )
  {
    $objcode =~ s/ +/ /g;
    $objcode =~ s/\t+/ /g;
    $objcode =~ s/\(//g;
    $objcode =~ s/\)//g;
    $objcode =~ s/\r\n//g;
    $objcode =~ s/\n//g;
    
    @line = split( /\s/, $objcode );
    
    if( "".$line[1] eq "".$data )
    {
      return hex($line[2]);
    }
  }

  print( "$data が見つかりません。\n" );
  exit(1);
}


sub getDIR
{
  my( $data ) = @_;
  my( $dir, @line );

  foreach $dir ( @DIR_DATA )
  {
    $dir =~ s/ +/ /g;
    $dir =~ s/\t+/ /g;
    $dir =~ s/\(//g;
    $dir =~ s/\)//g;
    $dir =~ s/\r\n//g;
    $dir =~ s/\n//g;
    
    @line = split( /\s/, $dir );
    
    if( "".$line[1] eq "".$data )
    {
      return $line[2];
    }
  }

  print( "$data が見つかりません。\n" );
  exit(1);
}
