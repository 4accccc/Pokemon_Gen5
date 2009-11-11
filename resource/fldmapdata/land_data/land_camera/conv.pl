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

@FILE_LIST = undef;
$list_output = 0;
$file_list_count = 0;

open( FILEIN, $ARGV[0] );
@map_matrix = <FILEIN>;
close( FILEIN );


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
    #タイムスタンプチェック
    #binファイルがあるか？
    #$ARGV[1].$data[3]00_00～binより最新のファイルがあるか？
    $combine = 0;
   
    #binファイルがあるか？
    if( -e $data[0].".bin" )
    {
      @bin_status = stat( $data[0].".bin" );
      
      #$ARGV[1].$data[3]00_00～binより最新のファイルがあるか？
      for( $z=0; $z<$data[2]; $z++ )
      {
        for( $x=0; $x<$data[1]; $x++ )
        {
          $filename = $ARGV[1].$data[3];
          if( $x < 10 )
          {
            $filename = $filename."0".$x."_";
          }
          else
          {
            $filename = $filename.$x."_";
          }

          if( $z < 10 )
          {
            $filename = $filename."0".$z.".imd";
          }
          else
          {
            $filename = $filename.$z.".imd";
          }

          if( -e $filename )
          {
            @status = stat( $filename );

            if( $status[9] > $bin_status[9] )
            {
              $combine = 1;
            }
          }
        }
      }
    }
    else
    {
      $combine = 1;
    }

    if( $combine )
    {
      print( "output ".$data[0]."...\n" );

      #エリア情報の出力             ファイル名          　横幅　　　　縦幅　　　　　マトリクスID　
      $cmd = "perl area_output.pl ".$ARGV[1].$data[3]." ".$data[1]." ".$data[2]." ".$data[0]." 72 40";
      #$cmd = "perl area_output.pl ".$ARGV[1].$data[3]." ".$data[1]." ".$data[2]." ".$data[0]." 168 168";
      if( system( $cmd ) )
      {
        exit(1);
      }
      $list_output = 1;
    }

    #ファイル保存
    $FILE_LIST[ $file_list_count ] = $data[0];
    $file_list_count ++;

  }
}

if( $list_output )
{
  open( FILEOUT, ">".$ARGV[2] );

  foreach $one ( @FILE_LIST )
  {
    #アーカイブリストの作成
    print( FILEOUT "\"".$one.".bin\"\n" ); 
  }

  close( FILEOUT );
}


exit(0);
