###############################################################################
# ディレクトリ内SEARCH
#   ファイル内comment抽出スクリプト
#
#
#     comment_salvage.pl [タイプ...]
#       
#         [タイプ]
#           -f  関数ヘッダ
#           -F  ファイルヘッダ
#           未設定　全部
#
#
#
#
#
###############################################################################

$FUNC_COMMENT_FALG = 0;
$FILE_COMMENT_FLAG = 0;


#ヘルプ表示
if( $ARGV[0] eq "-h" )
{
  print( "comment_salvage.pl [タイプ...]\n" ); 
  print( "[タイプ]\n" );
  print( "-f  関数ヘッダ\n" );
  print( "-F  ファイルヘッダ\n" );
  print( "未設定  全部\n" );
  exit(0);
}

#引数チェック
$i = 0;
for( $i=0; $i<@ARGV; $i++ )
{
  if( $ARGV[$i] eq "-f" )
  {
    $FUNC_COMMENT_FALG = 1;
  }

  if( $ARGV[$i] eq "-F" )
  {
    $FILE_COMMENT_FALG = 1;
  }
}

if( @ARGV == 0 )
{
  $FUNC_COMMENT_FALG = 1;
  $FILE_COMMENT_FALG = 1;
}


#メイン実行
call &main();


exit(0);

sub main {
  
  $root_dir = '.';

  system ("find $root_dir|grep c\$ > tmp.dat\n");
  if( open( FILE, "<tmp.dat") )
  {
    my @files = <FILE>;
    close(FILE);
    system "rm tmp.dat";
    my $cmd;

    foreach my $filename (@files)
    {
      $filename =~ s/\r\n//g;

      &output( $filename );
    }
  }



  system ("find $root_dir|grep ev\$ > tmp.dat\n");
  if( open( FILE, "<tmp.dat") )
  {
    my @files = <FILE>;
    close(FILE);
    system "rm tmp.dat";
    my $cmd;

    foreach my $filename (@files)
    {
      $filename =~ s/\r\n//g;

      &output( $filename );
    }
  }
}








sub output
{
  my( $output_file, $first, $comment_on ) = @_;

  #
  # 調査
  #
  open( FILEIN, $output_file );
  @INPUT_FILE = <FILEIN>;
  close( FILEIN );


  #ファイル情報の出力
  print( $output_file." output data\n" );

  $first = 0;
  $comment_on = 0;
  
  #
  foreach $one ( @INPUT_FILE )
  {
    #先頭commentをすべて出力  
    if( ($first == 0) && ($FILE_COMMENT_FALG == 1) )
    {
      if( $one =~ /^[\s\t]+$/ )
      {
      }
      elsif ( $one =~ /^\/\// )
      {
        print $one;
      }
      elsif ( $one =~ /^\/\*/ )
      {
        $comment_on = 1;
        print $one;
      }
      elsif ( $one =~ /\*\// )
      {
        $comment_on = 0;
        print $one;
      }
      elsif( $comment_on == 1 )
      {
        &JapanPrint( $one );
      }
      else
      {
        #先頭出力終了
        $first = 1;
      }
    }
    else
    {


      #関数commentの出力  
      if( ($FUNC_COMMENT_FALG == 1) )
      {
        if ( $one =~ /^\/\// )
        {
          print $one;
        }
        elsif ( $one =~ /^\/\*/ )
        {
          $comment_on = 1;
          print $one;
        }
        elsif ( $one =~ /\*\// )
        {
          $comment_on = 0;
          print $one;
        }
        elsif( $comment_on == 1 )
        {
          &JapanPrint( $one );
        }
      }

    }
  }


  print( "\n\n" );
}



sub JapanPrint
{
  my( $outdata ) = @_;
 
    
  # $str に ASCII 以外が含まれているか判定する
  if ($outdata =~ /[\x8E\xA1-\xFE]/) {
    print $outdata;
  }
}

