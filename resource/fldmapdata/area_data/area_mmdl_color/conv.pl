###########################################################
#
#
# conv.pl   excel_file output_extension
#
# モデルカラー調整データの出力
#
###########################################################


if( @ARGV < 1 )
{
  print( "conv.pl excel_file output_extension\n" );
  exit(1);
}


#ファイル読み込み
open( FILEIN, $ARGV[0] );
@EXCEL_FILE = <FILEIN>;
close( FILEIN );

#各情報格納先
@TAG_TBL = undef;
@NORM_X = undef;
@NORM_Y = undef;
@NORM_Z = undef;
@DEF_R  = undef;
@DEF_G  = undef;
@DEF_B  = undef;
@AMB_R  = undef;
@AMB_G  = undef;
@AMB_B  = undef;
@SPQ_R  = undef;
@SPQ_G  = undef;
@SPQ_B  = undef;
@EMI_R  = undef;
@EMI_G  = undef;
@EMI_B  = undef;

$DATA_IN = 0;
foreach $one ( @EXCEL_FILE )
{
  $one =~ s/\r\n//g;
  $one =~ s/\n//g;

  @line = split( "\t", $one );

  if( $DATA_IN == 0 )
  {
    #開始チェック
    if( "".$line[0] eq "ラベル名"  )
    {
      $DATA_IN = 1;
      $TBL_COUNT = 0;
      $DATA_COUNT = 0;
    }
  }
  
  if( $DATA_IN == 1 )
  {
    #終了チェック
    if( "".$line[0] eq "#END" )
    {
      $DATA_IN = 0;
    }
    else
    {
      #データ格納
      if( $TBL_COUNT == 0 )
      {

        while( "".$line[$DATA_COUNT + 1] ne "#END" )
        {
          print( "TAG INPUT ".$line[$DATA_COUNT + 1]."\n" ); 
          #タグの収集
          $TAG_TBL[ $DATA_COUNT ] = $line[$DATA_COUNT + 1];
          $DATA_COUNT ++;
        }
      }
      ## 法線格納 ##
      elsif( $TBL_COUNT == 1 )
      {
        #法線Xの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $NORM_X[ $i ] = $line[$i + 1];
        }
      }
      elsif( $TBL_COUNT == 2 )
      {
        #法線Yの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $NORM_Y[ $i ] = $line[$i + 1];
        }
      }
      elsif( $TBL_COUNT == 3 )
      {
        #法線Zの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $NORM_Z[ $i ] = $line[$i + 1];
        }
      }
      ## ディフューズ格納 ##
      elsif( $TBL_COUNT == 4 )
      {
        #ディフューズRの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $DEF_R[ $i ] = $line[$i + 1];
        }
      }
      elsif( $TBL_COUNT == 5 )
      {
        #ディフューズGの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $DEF_G[ $i ] = $line[$i + 1];
        }
      }
      elsif( $TBL_COUNT == 6 )
      {
        #ディフューズBの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $DEF_B[ $i ] = $line[$i + 1];
        }
      }
      ## アンビエント格納 ##
      elsif( $TBL_COUNT == 7 )
      {
        #アンビエントRの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $AMB_R[ $i ] = $line[$i + 1];
        }
      }
      elsif( $TBL_COUNT == 8 )
      {
        #アンビエントGの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $AMB_G[ $i ] = $line[$i + 1];
        }
      }
      elsif( $TBL_COUNT == 9 )
      {
        #アンビエントBの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $AMB_B[ $i ] = $line[$i + 1];
        }
      }
      ## スペキュラー格納 ##
      elsif( $TBL_COUNT == 10 )
      {
        #スペキュラーRの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $SPQ_R[ $i ] = $line[$i + 1];
        }
      }
      elsif( $TBL_COUNT == 11 )
      {
        #スペキュラーGの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $SPQ_G[ $i ] = $line[$i + 1];
        }
      }
      elsif( $TBL_COUNT == 12 )
      {
        #スペキュラーBの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $SPQ_B[ $i ] = $line[$i + 1];
        }
      }
      ## エミッション格納 ##
      elsif( $TBL_COUNT == 13 )
      {
        #エミッションRの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $EMI_R[ $i ] = $line[$i + 1];
        }
      }
      elsif( $TBL_COUNT == 14 )
      {
        #エミッションGの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $EMI_G[ $i ] = $line[$i + 1];
        }
      }
      elsif( $TBL_COUNT == 15 )
      {
        #エミッションBの格納
        for( $i=0; $i<$DATA_COUNT; $i++ )
        {
          $EMI_B[ $i ] = $line[$i + 1];
        }
      }

      #次のテーブル格納へ
      $TBL_COUNT ++;
    }
  }

}






##############
# 出力部
##############

for( $i=0; $i<$DATA_COUNT; $i++ )
{
  $filename = lc( $TAG_TBL[$i] ).".".$ARGV[1];
  
  open( FILEOUT, ">$filename" );
  binmode( FILEOUT );

  #法線方向
  print( FILEOUT pack( "I", hex( $NORM_X[$i] ) ) );
  print( FILEOUT pack( "I", hex( $NORM_Y[$i] ) ) );
  print( FILEOUT pack( "I", hex( $NORM_Z[$i] ) ) );
  #ディフューズカラ
  print( FILEOUT pack( "S", $DEF_R[$i] ) );
  print( FILEOUT pack( "S", $DEF_G[$i] ) );
  print( FILEOUT pack( "S", $DEF_B[$i] ) );

  #アンビエントカラー
  print( FILEOUT pack( "S", $AMB_R[$i] ) );
  print( FILEOUT pack( "S", $AMB_G[$i] ) );
  print( FILEOUT pack( "S", $AMB_B[$i] ) );
  #スペキュラーカラー
  print( FILEOUT pack( "S", $SPQ_R[$i] ) );
  print( FILEOUT pack( "S", $SPQ_G[$i] ) );
  print( FILEOUT pack( "S", $SPQ_B[$i] ) );
  #エミッションカラー
  print( FILEOUT pack( "S", $EMI_R[$i] ) );
  print( FILEOUT pack( "S", $EMI_G[$i] ) );
  print( FILEOUT pack( "S", $EMI_B[$i] ) );

  
  
  close( FILEOUT );
}


exit(0);
