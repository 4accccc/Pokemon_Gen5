##################################################
#
#   Pokemon_WB_SE_Env.xls
#     コンバート環境
#
#   conv.pl excel.csv output
#
#
##################################################


if( @ARGV < 2 ){
  print( "conv.pl excel.csv output\n" );
  exit(1);
}


open( FILEIN, @ARGV[0] );
@EXCELFILE = <FILEIN>;
close( FILEIN );



$DATA_IDX_LABEL = 0;
$DATA_IDX_LOOP = 1;

@LABEL_TBL = undef;
@LOOP_TBL = undef;
$DATA_NUM = 0;

$data_in = 0;

foreach $one (@EXCELFILE)
{
  $one =~ s/\n//g;
  $one =~ s/\r\n//g;
  
  @line = split( /,/, $one );
  
  if( $data_in == 0 )
  {
    if( "".$line[$DATA_IDX_LABEL] eq "ラベル名" ){
      $data_in = 1;
    }
  }
  elsif( $data_in == 1 )
  {
    
    if( "".$line[$DATA_IDX_LABEL] eq "#END" ){
      $data_in = 0;
    }else{

      if( $line[$DATA_IDX_LABEL] =~ /^SEQ/ ){

        $LABEL_TBL[ $DATA_NUM ] = $line[$DATA_IDX_LABEL];
        if( "".$line[$DATA_IDX_LOOP] eq "Loop" ){
          $LOOP_TBL[ $DATA_NUM ] = "TRUE";
        }
        elsif( "".$line[$DATA_IDX_LOOP] eq "単発" ){
          $LOOP_TBL[ $DATA_NUM ] = "FALSE";
        }
        else{
          print( "Loop設定　不正".$line[$DATA_IDX_LOOP]."\n" );
          exit(1);
        }


        $DATA_NUM ++;
      }
    }
  }
}

#出力

open( FILEOUT, ">".$ARGV[1] );

print( FILEOUT "// このソースはresource/fldmapdata/envse/conv.plから出力されています。\n" );
print( FILEOUT "\n" );
print( FILEOUT "\n" );
print( FILEOUT "const FLD_ENVSE_DATA c_FLD_ENVSE_DATA[] = {\n" );
for( $i=0; $i<$DATA_NUM; $i++ ){
  print( "label ".$LABEL_TBL[ $i ]." loop ".$LOOP_TBL[ $i ]."\n" );
  print( FILEOUT "{ ".$LABEL_TBL[ $i ].", ".$LOOP_TBL[ $i ]."},\n" );
}
print( FILEOUT "};\n" );

close( FILEOUT );

