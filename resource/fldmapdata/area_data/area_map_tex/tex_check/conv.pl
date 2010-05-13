############################
#
#   テクスチャ名内に、.2などがないかチェック
#
#
#
############################

$ARG_IDX_MAX  =0;

if( @ARGV < $ARG_IDX_MAX ){
}

@all_tex_file = glob( "../src_imd_files/*.imd" );

foreach $filename ( @all_tex_file ){

  print( $filename." ...\n" );
  
  open( FILEIN, $filename );

  foreach $line ( <FILEIN> ){

    if( $line =~ /tex_image/ ){
      if( $line =~ /name="([^"]*\.([0-9]*)[^"]*)"/ ){
        if( $2 > 1 ){
          print( "error ".$1."\n" );
        }
      }
    }
    
  }

  close( FILEIN );
}


