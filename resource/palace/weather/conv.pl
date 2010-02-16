#####################################################################
#
# パレス　天気（FOG）判断到達フラグ　データ出力
#     conv.pl excel_tab output_cdat output_header
#
#
#
#
#####################################################################


if( @ARGV < 2 ){
  print( "conv.pl excel_tab output_cdat \n" );
  exit(1);
}


#読み込み
open( FILEIN, $ARGV[0] );
@EXCEL_FILE = <FILEIN>;
close( FILEIN );


#データ
@DATA_ARRAY = undef;
$DATA_NUM = 0;

$DATA_IDX_ZONE_ID = 0;
$DATA_IDX_CHECK_TOWN_ID = 1;
$DATA_IDX_MAX = 2;

$data_in = 0;
$data_index = 0;


#解析
foreach $one ( @EXCEL_FILE )
{
  @line = split( /\t/, $one );

  if( $data_in == 0 )
  {
    if( "".$line[0] eq "#start" )
    {
      $data_in = 1;
      $data_index = 0;
    }
  }
  else
  {
    if( "".$line[0] eq "#end" )
    {
      $data_in = 0;
    }
    elsif( $data_index > 1 ){
      
      $DATA_ARRAY[ $DATA_NUM * $DATA_IDX_MAX ] = $line[0];
      $DATA_ARRAY[ ($DATA_NUM * $DATA_IDX_MAX) + 1 ] = $line[1];
      $DATA_NUM ++;
    }

    $data_index ++;
  }
}

#出力
open( FILEOUT, ">".$ARGV[1] );

print( FILEOUT "// このファイルはresource/palace/weather/conv.plで生成されています。\n" );
print( FILEOUT "// パレス上のゾーンに霧の天気をかけるかをチェックする到着フラグテーブルです。\n" );
print( FILEOUT "#include \"arc/fieldmap/zone_id.h\"\n" );
print( FILEOUT "#include \"../../../resource/fldmapdata/flagwork/flag_define.h\"\n\n" );

print( FILEOUT "typedef struct {\n" );
print( FILEOUT "  u16 zone_id;\n" );
print( FILEOUT "  u16 sys_flag;\n" );
print( FILEOUT "} PALACE_WEATHER_DATA;\n\n" );

print( FILEOUT "static const PALACE_WEATHER_DATA sc_PALACE_WEATHER_DATA[ $DATA_NUM ] = {\n" );


for( $i=0; $i<$DATA_NUM; $i++ )
{
  print( FILEOUT "  { ZONE_ID_".$DATA_ARRAY[ ($i * $DATA_IDX_MAX) ].", SYS_FLAG_ARRIVE_".$DATA_ARRAY[ ($i * $DATA_IDX_MAX) + 1 ]." },\n" );
  
}

print( FILEOUT "};\n\n" );

close( FILEOUT );

exit(0);
