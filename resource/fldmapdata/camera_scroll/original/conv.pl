#####################################################################
#
# originalカメラスクロール範囲設定のコンバート
#
# conv.pl   入力ファイル　field_cameraヘッダー
#
#
#####################################################################


if( @ARGV != 2 )
{
  print( "conv.pl 入力ファイル　field_cameraヘッダー\n" );
  exit(1);
}


@AREA_TYPE_DEF  = undef;   #カメラ範囲タイプ定義
@CONT_TYPE_DEF  = undef;   #座標管理タイプ定義


##-------------------------------------
##	パラメータの定義
##=====================================
$PARAM_MAX   = 6;
$BIN_MAX     = 16;    #パラメータ部に必要なバイトサイズ

##-------------------------------------
##	矩形パラメータの情報
##=====================================
$RECT_PARAM_BYTE[0] = "i";    #s32
$RECT_PARAM_BYTE[1] = "i";    #s32
$RECT_PARAM_BYTE[2] = "i";    #s32
$RECT_PARAM_BYTE[3] = "i";    #s32
$RECT_PARAM_FOOTER_BYTE = 0;

##-------------------------------------
##	サークルパラメータの情報
##=====================================
$CIRCLE_PARAM_BYTE[0] = "i";    #fx32
$CIRCLE_PARAM_BYTE[1] = "i";    #fx32
$CIRCLE_PARAM_BYTE[2] = "i";    #fx32
$CIRCLE_PARAM_BYTE[3] = "S";    #u16
$CIRCLE_PARAM_BYTE[4] = "S";    #u16
$CIRCLE_PARAM_FOOTER_BYTE = 0;




open( FILEIN, $ARGV[0] );
@input_file = <FILEIN>;
close( FILEIN );

#ヘッダー情報の取得
&LoadHeader( $ARGV[1] );


#データ数を計算
$DATANUM = 0;
$data_in = 0;
foreach $data ( @input_file )
{
  $data =~ s/\r\n//;
  $data =~ s/\n//;
  
  @data_array = split( /\t/, $data );
  
  if( $data_in == 0 )
  {
    if( $data_array[0] =~ /#START/ )
    {
      $data_in = 1;
    }
  }
  elsif( $data_in == 1 )
  {

    if( $data_array[0] =~ /#END/ )
    {
      $data_in = 2;
    }
    else
    {
      $DATANUM ++;
    }
  }
}


#出力
$filename = $ARGV[0];
$filename =~ s/\.txt//g;
open( FILEOUT, ">".$filename.".bin" );
binmode( FILEOUT );
{
  print( FILEOUT pack( "I", $DATANUM ) ); 

  
  #入力ファイル情報の取得
  $TAGNAME    = 0;   #タグ名
  $AREA_TYPE  = 0;   #カメラ範囲タイプ
  $CONT_TYPE  = 0;   #座標管理タイプ
  @PARAM      = undef;  #パラメータ

  $data_in = 0;
  foreach $data ( @input_file )
  {
    $data =~ s/\r\n//;
    $data =~ s/\n//;
    
    @data_array = split( /\t/, $data );
    
    if( $data_in == 0 )
    {
      if( $data_array[0] =~ /#START/ )
      {
        $data_in = 1;
      }
    }
    elsif( $data_in == 1 )
    {

      if( $data_array[0] =~ /#END/ )
      {
        $data_in = 2;
      }
      else
      {

        #入力＋出力
        $TAGNAME    = $data_array[0];
        $AREA_TYPE  = &getCameraAreaType( $data_array[1] );
        $CONT_TYPE  = &getCameraContType( $data_array[2] );

        for( $i=0; $i<$PARAM_MAX; $i++ )
        {
          if( $data_array[3+$i] =~ /0x/ )
          {
            $PARAM[$i] = oct( $data_array[3+$i] );
          }
          else
          {
            $PARAM[$i] = $data_array[3+$i];
          }
        }




        print( FILEOUT pack( "I", $AREA_TYPE ) ); 
        print( FILEOUT pack( "I", $CONT_TYPE ) ); 

        if( $AREA_TYPE_DEF[ $AREA_TYPE ] eq "FIELD_CAMERA_AREA_RECT"  )
        {
          for( $i=0; $i<@RECT_PARAM_BYTE; $i++ )
          {
            print( FILEOUT pack( $RECT_PARAM_BYTE[$i], $PARAM[$i] ) ); 
          }

          #あまりバイト
          for( $i=0; $i<$RECT_PARAM_FOOTER_BYTE; $i++ )
          {
            print( FILEOUT pack( "c", 0 ) ); 
          }
        }
        else
        {
          for( $i=0; $i<@CIRCLE_PARAM_BYTE; $i++ )
          {
            print( FILEOUT pack( $CIRCLE_PARAM_BYTE[$i], $PARAM[$i] ) ); 
          }

          #あまりバイト
          for( $i=0; $i<$CIRCLE_PARAM_FOOTER_BYTE; $i++ )
          {
            print( FILEOUT pack( "c", 0 ) ); 
          }
        }
        
      }
    }
  }
  close( FILEOUT );
}






exit(0);







sub LoadHeader
{
  my( $filein ) = @_;
  my( @header, $one, $count, $input_count, $input_type );
  
  open( HEADERIN, $filein );
  @header = <HEADERIN>;
  close( HEADERIN );


  #定義を見つける
  $input_type = 0;
  $input_count = 0;
  foreach $one ( @header )
  {
    if( $input_type == 0 )
    {
      if( $one =~ /\[CAMERA_AREA\]/ )
      {
        $input_type = 1;
        $count = 0;
        $input_count ++;
        print( "CAMERA_AREA取得\n" );
      }
      elsif( $one =~ /\[CAMERA_CONT\]/ )
      {
        $input_type = 2;
        $count = 0;
        $input_count ++;
        print( "CAMERA_CONT取得\n" );
      }
    }
    elsif( $input_type == 1 )
    {
      #CAMERA_AREA
      if( $one =~ /}/ )
      {
        $input_type = 0;
      }
      else
      {
        if( $one =~ /[\t\s]*([^,]*),/ )
        {
          $AREA_TYPE_DEF[ $count ] = $1;
          $count ++;
        }
      }
    }
    elsif( $input_type == 2 )
    {
      #CAMERA_CONT
      if( $one =~ /}/ )
      {
        $input_type = 0;
      }
      else
      {
        if( $one =~ /[\t\s]*([^,]*),/ )
        {
          $CONT_TYPE_DEF[ $count ] = $1;
          $count ++;
        }
      }
    }
  }


  if( $input_count != 2 )
  {
    print( "field_camera.hの情報を正しく取得できませんでした。\n" );
  }
}




sub getCameraAreaType
{
  my( $areatype ) = @_;
  my( $area_count, $area_one );


  $area_count = 0;

  foreach $area_one ( @AREA_TYPE_DEF )
  {
    if( $area_one =~ /$areatype/ )
    {
      return $area_count;
    }
    $area_count ++;
  }


  print( "Camera AreaType None $areatype\n" );
  exit(1);
}


sub getCameraContType
{
  my( $conttype ) = @_;
  my( $cont_count, $cont_one );


  $cont_count = 0;

  foreach $cont_one ( @CONT_TYPE_DEF )
  {
    if( $cont_one =~ /$conttype/ )
    {
      return $cont_count;
    }
    $cont_count ++;
  }


  print( "Camera ContType None $conttype\n" );
  exit(1);
}
