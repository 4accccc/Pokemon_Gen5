##############################################
#
#
#   タイムゾーンテーブルを作成する。
#
#   conv.pl excel.tab output
#
#
##############################################

if( @ARGV < 2 ){
  print( "conv.pl excel.tab output\n" );
  exit(1);
}

#定数
$ONE_DAY_DATA_NUM = 24;
$SEASON_SPRING = 0;
$SEASON_SUMMER = 1;
$SEASON_AUGUST = 2;
$SEASON_WINTER = 3;
$SEASON_NUM = 4;


#読み込み
open( FILEIN, $ARGV[0] );
@excelFile = <FILEIN>;
close( FILEIN );


@TIMEZONE_TBL = undef;
$DATA_NUM = 0;


# タイムゾーンテーブルの情報収集
$data_in = 0;
foreach $one ( @excelFile )
{
  $one =~ s/\r\n/\t/g;
  $one =~ s/\n/\t/g;
  
  @line = split( /\t/, $one );
  

  if( $data_in == 0 )
  {
    if( "".$line[0] eq "時間" ){
      $data_in = 1;
    }
  }
  elsif( $data_in == 1 )
  {
    
    if( "".$line[0] eq "#END" ){
      $data_in = 0;
    }else{
      #情報の格納
      for( $i=0; $i<$SEASON_NUM; $i++ ){
        $TIMEZONE_TBL[ $DATA_NUM ] = "TIMEZONE_".$line[1 + $i];
        $DATA_NUM ++;
      }
    }
  }
}

#出力
open( FILEOUT, ">".$ARGV[1] );
{
  print( FILEOUT "// このソースはresource/fldmapdata/timezone/conv.plから出力されています。\n" );
  print( FILEOUT "#include \"gamesystem/pm_season.h\"\n" );
  print( FILEOUT "#include \"system/timezone.h\"\n" );
  print( FILEOUT "\n" );
  print( FILEOUT "\n" );
  print( FILEOUT "static const u8 sc_SEASON_TIMEZONE[PMSEASON_TOTAL][24] = {\n" );

  print( FILEOUT "  // 春\n" );
  print( FILEOUT "  {\n" );
  print( FILEOUT "    " );
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    print( FILEOUT $TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SPRING ].", " );
  }
  print( FILEOUT "\n  },\n" );


  print( FILEOUT "  // 夏\n" );
  print( FILEOUT "  {\n" );
  print( FILEOUT "    " );
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    print( FILEOUT $TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SUMMER ].", " );
  }
  print( FILEOUT "\n  },\n" );

  print( FILEOUT "  // 秋\n" );
  print( FILEOUT "  {\n" );
  print( FILEOUT "    " );
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    print( FILEOUT $TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_AUGUST ].", " );
  }
  print( FILEOUT "\n  },\n" );

  print( FILEOUT "  // 冬\n" );
  print( FILEOUT "  {\n" );
  print( FILEOUT "    " );
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    print( FILEOUT $TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_WINTER ].", " );
  }
  print( FILEOUT "\n  },\n" );

  print( FILEOUT "};\n" );
}
#四季分の時間帯切り替えタイミングテーブル
{
  $roop_end = 0;

  print( FILEOUT "static const u8 sc_SEASON_TIMEZONE_CHANGE[PMSEASON_TOTAL][TIMEZONE_MAX] = {\n" );

  print( FILEOUT "  // 春\n" );
  print( FILEOUT "  {\n" );
  print( FILEOUT "    " );
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_MORNING" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SPRING ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_NOON" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SPRING ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_EVENING" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SPRING ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_NIGHT" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SPRING ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_MIDNIGHT" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SPRING ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  print( FILEOUT "\n  },\n" );


  print( FILEOUT "  // 夏\n" );
  print( FILEOUT "  {\n" );
  print( FILEOUT "    " );
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_MORNING" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SUMMER ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_NOON" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SUMMER ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_EVENING" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SUMMER ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_NIGHT" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SUMMER ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_MIDNIGHT" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_SUMMER ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  print( FILEOUT "\n  },\n" );

  print( FILEOUT "  // 秋\n" );
  print( FILEOUT "  {\n" );
  print( FILEOUT "    " );
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_MORNING" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_AUGUST ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_NOON" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_AUGUST ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_EVENING" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_AUGUST ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_NIGHT" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_AUGUST ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_MIDNIGHT" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_AUGUST ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  print( FILEOUT "\n  },\n" );

  print( FILEOUT "  // 冬\n" );
  print( FILEOUT "  {\n" );
  print( FILEOUT "    " );
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_MORNING" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_WINTER ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_NOON" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_WINTER ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_EVENING" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_WINTER ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_NIGHT" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_WINTER ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  $roop_end = 0;
  for( $i=0; $i<$ONE_DAY_DATA_NUM; $i++ ){
    if( $roop_end == 0 ){
      if( "TIMEZONE_MIDNIGHT" eq "".$TIMEZONE_TBL[ ($i*$SEASON_NUM) + $SEASON_WINTER ] ){
        print( FILEOUT "$i, " );
        $roop_end = 1;
      }
    }
  }
  print( FILEOUT "\n  },\n" );

  print( FILEOUT "};\n" );
}
close( FILEOUT );


exit(0);
