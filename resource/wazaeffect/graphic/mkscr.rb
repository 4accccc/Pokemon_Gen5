
	if ARGV.size < 1
		print "error: ruby mkscr.rb write_file\n";
		print "write_file: 書き出すファイル名\n";
		exit( 1 )
	end

	ARGV_WRITE_FILE = 0

	fp_w = open( ARGV[ARGV_WRITE_FILE], "w" )

  Dir::glob("./*.ncg").each {|f|
    name = File::basename( f, '.*' )
    fp_w.print( "\"" + name + ".NCGR\"\n" )
    fp_w.print( "\"" + name + ".NSCR\"\n" )
    fp_w.print( "\"" + name + ".NCLR\"\n" )
  }

  fp_w.close
