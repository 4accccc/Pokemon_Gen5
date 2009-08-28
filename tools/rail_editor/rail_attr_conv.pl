##-----------------------------------------------------------------------------
#	.railファイルをBinary情報にコンバート
#
#
#	rail_attr_conv.pl	.attr
#
#
#
##-----------------------------------------------------------------------------

##-----------------------------------------------------------------------------
#　定数
##-----------------------------------------------------------------------------
$DEF_RAIL_ATTR_SEQ_ALLDATA = 0;
$DEF_RAIL_ATTR_SEQ_SEARCHLINE = 1;
$DEF_RAIL_ATTR_SEQ_LINEX = 2;
$DEF_RAIL_ATTR_SEQ_LINEZ = 3;
$DEF_RAIL_ATTR_SEQ_ATTR = 4;

##-----------------------------------------------------------------------------
#　データ
##-----------------------------------------------------------------------------
# 読み込み情報
@RAIL_ATTR_DATA = undef;

##-----------------------------------------------------------------------------
#　メインルーチン
##-----------------------------------------------------------------------------

#引数チェック
if( @ARGV < 1 )
{
	print( "rail_attr_conv.pl	.attr \n" );
	exit(1);
}


#情報の読み込み
open( FILEIN, $ARGV[0] );
@RAIL_ATTR_DATA = <FILEIN>;
close( FILEIN );

&output( \@RAIL_ATTR_DATA, $ARGV[0] );

exit(0);


#出力
sub output
{
  my( $data, $name ) = @_;
  my( $one, $seq, $linenum, $read_line, @attr );
  
  #拡張子部分を削る
  $name =~ s/\..*//;

	open( FILEOUT, ">".$name.".atdat" );
  binmode( FILEOUT );
  
  #各書記か
  $seq = $DEF_RAIL_ATTR_SEQ_ALLDATA;
  $linenum = 0;
  $read_line = 0;
  #情報の取得＋出力
  foreach $one ( @$data )
  {
    #ライン数の取得
    if( $seq == $DEF_RAIL_ATTR_SEQ_ALLDATA )
    {
      #ライン数を出力する
      if( $one =~ /--LINENUM::([0-9]*)/ )
      {
        $linenum = $1;

        print( "linenum = $linenum\n" );
        print( FILEOUT pack( "I", $linenum ) );

        #LINEデータSEARCH
        $seq = $DEF_RAIL_ATTR_SEQ_SEARCHLINE;
      }
    }
    elsif( $seq == $DEF_RAIL_ATTR_SEQ_SEARCHLINE )
    {
      if( $one =~ /--NAME::(.*)/ )
      {
        $read_line++;
        print( "out linename = $1\n" );
        $seq = $DEF_RAIL_ATTR_SEQ_LINEX;
      }
    }
    elsif( $seq == $DEF_RAIL_ATTR_SEQ_LINEX )
    {
      if( $one =~ /--X::([0-9]*)/ )
      {
        print( "x=$1\n" );
        print( FILEOUT pack( "S", $1 ) );
        $seq = $DEF_RAIL_ATTR_SEQ_LINEZ;
      }
    }
    elsif( $seq == $DEF_RAIL_ATTR_SEQ_LINEZ )
    {
      if( $one =~ /--Z::([0-9]*)/ )
      {
        print( "z=$1\n" );
        print( FILEOUT pack( "S", $1 ) );
        $seq = $DEF_RAIL_ATTR_SEQ_ATTR;
      }
    }
    elsif( $seq == $DEF_RAIL_ATTR_SEQ_ATTR )
    {
      #終了チェック
      if( $one =~ /\+LINE/ )
      {
        $seq = $DEF_RAIL_ATTR_SEQ_SEARCHLINE;
      }
      elsif( $one =~ /--LINEATTR::(.*)/ )
      {
        @attr = split( /\s/, $1 );
        print( "flag = ".$attr[1]." data = ".$attr[2]."\n" );
        print( FILEOUT pack( "I", ($attr[1]<<16) | $attr[2] ) );
      }
    }
  }



  close( FILEOUT );

  if(  $read_line != $linenum )
  {
    print( "ERR ALL LineNum = $linenum  Read LineNum = $read_line\n" );
    exit(1);
  }
}
