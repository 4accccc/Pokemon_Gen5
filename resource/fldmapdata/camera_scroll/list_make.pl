#######################################################
#
#
#
#   ""Ç≈àÕÇÌÇÍÇΩÇ‡ÇÃÇ∆ÇªÇ§Ç≈Ç»Ç¢Ç‡ÇÃÇîªï ÇµÇ»Ç™ÇÁÅAÉäÉXÉgÇçÏê¨
#
#
#
#######################################################

if( @ARGV == 0 )
{
  print( "perl list_make.pl list.txt" );
  exit(1);
}



open( FILEIN, $ARGV[0] );
@file = <FILEIN>;
close( FILEIN );



open( FILEOUT, ">".$ARGV[0] );

foreach $one ( @file )
{
  $one =~ s/\r\n//g;
  $one =~ s/\n//g;
  
  if( $one =~ /"/ )
  {
    print( FILEOUT $one."\n" );
  }
  else
  {
    print( FILEOUT "\"$one\"\n" );
  }
}


close( FILEOUT );

exit(0);
