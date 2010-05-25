##########################################
#
# 引数のソースリストと、引数のテキストデータ
# から一致するソースがあったラインのみ出力
#
# まず、OS_TPrintf OS_Printfをリストアップ
# perl SEARCH
# 
# 削除すると意味がある物のみを選別
# perl CHECK check_printf.pl sourcelist....
#
#
#
##########################################

if( @ARGV < 1 ){
  print( "perl SEARCH or ↓\n" ); 
  print( "perl CHECK check_printf.pl sourcelist....\n" ); 
  exit(1);
}



if( "".$ARGV[0] eq "CHECK" ){


  open( FILEIN, "print_check.txt" );
  @file = <FILEIN>;
  close( FILEIN );

  foreach $line ( @file ){

    $check = $line;

    $check =~ s/\s//g;
    $check =~ s/\t//g;
    
    if( $check =~ /:\/\// ){
      #スキップ
      
    }else{
      for( $i=1; $i<@ARGV; $i++ ){

        $filename = $ARGV[ $i ];

        $filename =~ s/\\/@/g;
        $filename =~ s/\//@/g;

        $check = $line;
        $check =~ s/\\/@/g;
        $check =~ s/\//@/g;

        if( $check =~ /$filename/ ){
          print( $line );
        }
      }
    }
  }

  print( "\n" );
  
}else{

  system( 'findstr /S  /R OS_T*Printf src/*.c > print_check.txt' );
}


