##########################################################
#
#   conv.pl   xcel_tab  objcode dir zone_id output_list output_arc script_h output_script
#
##########################################################


if( @ARGV < 8 )
{
  print( "conv.pl   xcel_tab  objcode dir zone_id output_list output_arc script_h output_script\n" );
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

open( FILEIN, $ARGV[6] );
@SCRIPT_H = <FILEIN>;
close( FILEIN );


$DATA_NUM = 0;
@DATA_ZONEID = undef;
@DATA_WAIT_MORNING = undef;
@DATA_WAIT_NOON = undef;
@DATA_WAIT_EVENING = undef;
@DATA_WAIT_NIGHT = undef;
@DATA_WAIT_MIDNIGHT = undef;
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

$DATA_IDX_ZONEID = 0;
$DATA_IDX_WAIT_MORNING = 1;
$DATA_IDX_WAIT_NOON = 2;
$DATA_IDX_WAIT_EVENING = 3;
$DATA_IDX_WAIT_NIGHT = 4;
$DATA_IDX_WAIT_MIDNIGHT = 5;
$DATA_IDX_POINT_NUM = 6;
$DATA_IDX_OBJCODE_NUM = 7;
$DATA_IDX_OBJCODE00 = 8;
$DATA_IDX_OBJCODE01 = 9;
$DATA_IDX_OBJCODE02 = 10;
$DATA_IDX_OBJCODE03 = 11;
$DATA_IDX_POINT00_LEFT = 12;
$DATA_IDX_POINT00_RIGHT = 13;
$DATA_IDX_POINT00_TOP = 14;
$DATA_IDX_POINT00_BOTTOM = 15;
$DATA_IDX_POINT00_DIR = 16;
$DATA_IDX_POINT00_GRID_NUM = 17;
$DATA_IDX_POINT01_LEFT = 18;
$DATA_IDX_POINT01_RIGHT = 19;
$DATA_IDX_POINT01_TOP = 20;
$DATA_IDX_POINT01_BOTTOM = 21;
$DATA_IDX_POINT01_DIR = 22;
$DATA_IDX_POINT01_GRID_NUM = 23;
$DATA_IDX_NUM = 23;




@SCRIPT_DATA = undef;
$SCRIPT_DATA_NUM = 0;
$SCRIPT_DATA_IDX_OBJCODE = 0;
$SCRIPT_DATA_IDX_SCRIPT00 = 1;
$SCRIPT_DATA_IDX_SCRIPT01 = 2;
$SCRIPT_DATA_IDX_SCRIPT02 = 3;
$SCRIPT_DATA_IDX_SCRIPT03 = 4;
$SCRIPT_DATA_IDX_NUM = 5;



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
    elsif( "".$line[0] eq "#STARTOBJCODE" )
    {
      $data_in = 2;
      $SCRIPT_DATA_NUM = 0;
    }
  }
  elsif ( $data_in == 1 )
  {
    if( "".$line[0] eq "#END" )
    {
      $data_in = 0;
    }
    else
    {
      if( $data_count == $DATA_IDX_ZONEID )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_ZONEID[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_WAIT_MORNING )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_WAIT_MORNING[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_WAIT_NOON )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_WAIT_NOON[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_WAIT_EVENING )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_WAIT_EVENING[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_WAIT_NIGHT )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_WAIT_NIGHT[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_WAIT_MIDNIGHT )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_WAIT_MIDNIGHT[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT_NUM )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT_NUM[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_OBJCODE_NUM )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_OBJCODE_NUM[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_OBJCODE00 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_OBJCODE00[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_OBJCODE01 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_OBJCODE01[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_OBJCODE02 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_OBJCODE02[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_OBJCODE03 )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_OBJCODE03[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT00_LEFT )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_LEFT[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT00_RIGHT )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_RIGHT[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT00_TOP )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_TOP[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT00_BOTTOM )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_BOTTOM[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT00_DIR )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_DIR[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT00_GRID_NUM )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT00_GRID_NUM[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT01_LEFT )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_LEFT[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT01_RIGHT )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_RIGHT[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT01_TOP )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_TOP[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT01_BOTTOM )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_BOTTOM[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT01_DIR )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_DIR[ $i ] = $line[1 + $i];
        }
      }
      elsif ( $data_count == $DATA_IDX_POINT01_GRID_NUM )
      {
        for( $i = 0; $i < $DATA_NUM; $i ++ )
        {
          $DATA_POINT01_GRID_NUM[ $i ] = $line[1 + $i];
        }
      }
      
      $data_count ++;
    }
  }
  # OBJCODEのスクリプト
  elsif ( $data_in == 2 )
  {
    if( "".$line[0] eq "#END" )
    {
      $data_in = 0;
    }
    else
    {
      for( $i = 0; $i <$SCRIPT_DATA_IDX_NUM; $i ++ )
      {
        $SCRIPT_DATA[ ($SCRIPT_DATA_NUM*$SCRIPT_DATA_IDX_NUM)+$i ] = $line[$i];
      }

      $SCRIPT_DATA_NUM ++;
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
  
  print( FILEOUT pack( "s", $DATA_WAIT_MORNING[$i] ) );
  print( FILEOUT pack( "s", $DATA_WAIT_NOON[$i] ) );
  print( FILEOUT pack( "s", $DATA_WAIT_EVENING[$i] ) );
  print( FILEOUT pack( "s", $DATA_WAIT_NIGHT[$i] ) );
  print( FILEOUT pack( "s", $DATA_WAIT_MIDNIGHT[$i] ) );
  print( FILEOUT pack( "s", 0 ) );    #padding
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





####
#   OBJCODEのスクリプトテーブルを出力
####
open( FILEOUT, ">".$ARGV[7] );
binmode( FILEOUT );

print( OUTLIST "\"".$ARGV[7]."\"\n" );

for( $i=0; $i<$SCRIPT_DATA_NUM; $i++ )
{
  print( FILEOUT pack( "I", &getOBJCODE($SCRIPT_DATA[ ($i*$SCRIPT_DATA_IDX_NUM) + $SCRIPT_DATA_IDX_OBJCODE ]) ) );
  print( FILEOUT pack( "S", &getSCRIPT_H($SCRIPT_DATA[ ($i*$SCRIPT_DATA_IDX_NUM) + $SCRIPT_DATA_IDX_SCRIPT00 ]) ) );
  print( FILEOUT pack( "S", &getSCRIPT_H($SCRIPT_DATA[ ($i*$SCRIPT_DATA_IDX_NUM) + $SCRIPT_DATA_IDX_SCRIPT01 ]) ) );
  print( FILEOUT pack( "S", &getSCRIPT_H($SCRIPT_DATA[ ($i*$SCRIPT_DATA_IDX_NUM) + $SCRIPT_DATA_IDX_SCRIPT02 ]) ) );
  print( FILEOUT pack( "S", &getSCRIPT_H($SCRIPT_DATA[ ($i*$SCRIPT_DATA_IDX_NUM) + $SCRIPT_DATA_IDX_SCRIPT03 ]) ) );
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


sub getSCRIPT_H
{
  my( $data ) = @_;
  my( $script_h, @line );

  foreach $script_h ( @SCRIPT_H )
  {
    $script_h =~ s/ +/ /g;
    $script_h =~ s/\t+/ /g;
    $script_h =~ s/\(//g;
    $script_h =~ s/\)//g;
    $script_h =~ s/\r\n//g;
    $script_h =~ s/\n//g;
    
    @line = split( /\s/, $script_h );
    
    if( "".$line[1] eq "".$data )
    {
      return $line[2];
    }
  }

  print( "$data が見つかりません。\n" );
  exit(1);
}


