######################################################
#
#
#   FOGテーブルコンバート
#
#
######################################################

if( @ARGV < 3 )
{
  print( "table_conv.pl need 3 param\n" );
  print( "table_conv.pl zonefog_table.csv zone_id.h zonefog_dat_list\n" );

  exit(1);
}

open( FILEIN, $ARGV[0] );
@DATA = <FILEIN>;
close( FILEIN );

open( FILEIN, $ARGV[1] );
@HEADER = <FILEIN>;
close( FILEIN );

open( FILEIN, $ARGV[2] );
@DAT_LIST = <FILEIN>;
close( FILEIN );


@ZONE_ID = undef;
@DATA_ID = undef;

$datacount =0;
$datain = 0;
foreach $one ( @DATA )
{
  @linearray = split( /,/, $one );

  if( $datain == 0 )
  {
    if( $linearray[0] eq "#START" )
    {
      $datain = 1;
    }
  }
  else
  {
    if( $linearray[0] eq "#END" )
    {
      $datain = 0;
    }
    else
    {
      $ZONE_ID[ $datacount ] = &getZoneId( $linearray[1] );
      $DATA_ID[ $datacount ] = &getDatID( $linearray[2] );
      $datacount ++;
    } 
  }
}

#出力
$output = $ARGV[0];
$output =~ s/\..*/\.bin/;
open( FILEOUT, ">".$output );
binmode( FILEOUT );

for( $i=0; $i<$datacount; $i++ )
{
  print( FILEOUT pack( "S", $ZONE_ID[ $i ] ) );
  print( FILEOUT pack( "S", $DATA_ID[ $i ] ) );
}

close( FILEOUT );

exit(0);


sub getZoneId
{
  my( $zoneid ) = @_;
  my( $line );

  foreach $line ( @HEADER )
  {
    #spaceを排除
    $line =~ s/[\s\t]//g;
    
    if( $line =~ /#define$zoneid\(([0-9]+)\)/ )
    {
      return $1;
    }
  }

  print( "zoneid not found $zoneid\n" );
  exit(1);
}


sub getDatID
{
  my( $datid ) = @_;
  my( $line ,$count );

  $datid =~ s/\r\n//g;
  $datid =~ s/\n//g;

  if( $datid =~ s/\.fog/\.dat/ )
  {
    $count  = 0;
    foreach $line ( @DAT_LIST )
    {
      $line =~ s/\r\n//g;
      $line =~ s/\n//g;
      
      #print( $line." == ".$datid."\n"  );
      if( $line =~ /$datid/ )
      {
        return $count;
      }
      $count  ++;
    }
  }


  print( "list id not found $datid\n" );
  exit(1);
}

