#################################################
#
#   依存チェック　スクリプト
#
# 依存設定
#   xls mev1 mev2 mev3 mev4
#
# check_mev.pl check_file
#
#################################################
if( @ARGV < 1 )
{
  print( "check_mev.pl check_file\n" );
  exit(1);
}

#ファイルがあるかチェック
if( -e $ARGV[0] )
{
}
else
{
  exit(0);
}

#ファイル読み込み
open( FILEIN, $ARGV[0] );
@CHECK_LIST_FILE = <FILEIN>;
close( FILEIN );



#依存チェック
foreach $one ( @CHECK_LIST_FILE )
{
  $one =~ s/\r\n//g;
  $one =~ s/\n//g;
  @list = split( " ", $one );

  $touch_file = 0;

  #ファイル更新時間チェック
  if( -e $list[0] )
  {
    #生成ファイルがない or 生成ファイル内の時間が変わっている時　$list[0]を更新

    for( $i=1; $i<@list; $i++ )
    {
      #print( $list[$i]." check\n" );
      if( -e $list[$i] )
      {
            
        #最終更新時間チェック
        open( FILEIN, $list[$i] );
        foreach $mev ( <FILEIN> )
        {
          if( $mev =~ /#save date/ )
          {
            if( "".$mev ne "#save date:2000/00/00 00:00:00\r\n" )
            {
              #日付が変わった！
              $touch_file = 1;
            }
          }
        }
        close( FILEIN );
      }
      else
      {
        #ないので、タッチ
        $touch_file = 1;
      }
    }
  }
  else
  {
    print( "check file ".$list[0]." not found\n" );
  }

  if( $touch_file )
  {
    $cmd = "touch ".$list[0];
    system( $cmd );
  }
}


exit(0);





