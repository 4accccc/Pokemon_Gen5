#####################################################################
#
# カメラ可動範囲の自動生成
#
# conv.pl   map_matrix　imdフォルダ　アーカイブリスト
#
#
#####################################################################

if( @ARGV != 3 )
{
  print( "conv.pl map_matrix imdフォルダ アーカイブリスト\n" );
  exit(1);
}


open( FILEIN, $ARGV[0] );
@map_matrix = <FILEIN>;
close( FILEIN );

open( FILEOUT, ">".$ARGV[2] );

$skip = 1;
foreach $one ( @map_matrix )
{
  @data = split( /\s/, $one );
  
  if( $skip == 1 )
  {
    $skip = 0;
  }
  elsif( $data[0] eq "#END" )
  {
  }
  else
  {

    print( "output ".$data[0]."...\n" );

    #エリア情報の出力             ファイル名          　横幅　　　　縦幅　　　　　マトリクスID　
    $cmd = "perl area_output.pl ".$ARGV[1].$data[3]." ".$data[1]." ".$data[2]." ".$data[0]." 72 40";
    if( system( $cmd ) )
    {
      close( FILEOUT );
      exit(1);
    }

    #アーカイブリストの作成
    print( FILEOUT "\"".$data[0].".bin\"\n" ); 
  }
}

close( FILEOUT );


exit(0);
