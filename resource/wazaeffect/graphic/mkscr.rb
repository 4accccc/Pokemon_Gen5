
	if ARGV.size < 1
		print "error: ruby mkscr.rb write_file\n";
		print "write_file: 書き出すファイル名\n";
		exit( 1 )
	end

	ARGV_WRITE_FILE = 0

	fp_w = open( ARGV[ARGV_WRITE_FILE], "w" )

  #BGファイルリスト生成
  Dir::glob("./*.nsc").each {|f|
    name = File::basename( f, '.*' )
    fp_w.print( "\"" + name + ".NSCR\"\n" )
    fp_w.print( "\"" + name + ".NCGR\"\n" )
    fp_w.print( "\"" + name + ".NCLR\"\n" )
  }

=begin
  #OBJファイルリスト生成
  Dir::glob("./*.nce").each {|f|
    name = File::basename( f, '.*' )
    fp_w.print( "\"" + name + ".NCGR\"\n" )
    fp_w.print( "\"" + name + ".NCLR\"\n" )
    fp_w.print( "\"" + name + ".NCER\"\n" )
    fp_w.print( "\"" + name + ".NANR\"\n" )
  }
=end
  fp_w.print( "\"ball_anim.NCER\"\n" )
  fp_w.print( "\"ball_anim.NANR\"\n" )
  fp_w.print( "\"ball_03_anim.NCGR\"\n" )
  fp_w.print( "\"ball_03_anim.NCLR\"\n" )
  fp_w.print( "\"ball_02_anim.NCGR\"\n" )
  fp_w.print( "\"ball_02_anim.NCLR\"\n" )
  fp_w.print( "\"ball_01_anim.NCGR\"\n" )
  fp_w.print( "\"ball_01_anim.NCLR\"\n" )
  fp_w.print( "\"ball_anim.NCGR\"\n" )
  fp_w.print( "\"ball_anim.NCLR\"\n" )
  fp_w.print( "\"ball_04_anim.NCGR\"\n" )
  fp_w.print( "\"ball_04_anim.NCLR\"\n" )
  fp_w.print( "\"ball_05_anim.NCGR\"\n" )
  fp_w.print( "\"ball_05_anim.NCLR\"\n" )
  fp_w.print( "\"ball_06_anim.NCGR\"\n" )
  fp_w.print( "\"ball_06_anim.NCLR\"\n" )
  fp_w.print( "\"ball_07_anim.NCGR\"\n" )
  fp_w.print( "\"ball_07_anim.NCLR\"\n" )
  fp_w.print( "\"ball_08_anim.NCGR\"\n" )
  fp_w.print( "\"ball_08_anim.NCLR\"\n" )
  fp_w.print( "\"ball_09_anim.NCGR\"\n" )
  fp_w.print( "\"ball_09_anim.NCLR\"\n" )
  fp_w.print( "\"ball_10_anim.NCGR\"\n" )
  fp_w.print( "\"ball_10_anim.NCLR\"\n" )
  fp_w.print( "\"ball_11_anim.NCGR\"\n" )
  fp_w.print( "\"ball_11_anim.NCLR\"\n" )
  fp_w.print( "\"ball_13_anim.NCGR\"\n" )
  fp_w.print( "\"ball_13_anim.NCLR\"\n" )
  fp_w.print( "\"ball_14_anim.NCGR\"\n" )
  fp_w.print( "\"ball_14_anim.NCLR\"\n" )
  fp_w.print( "\"ball_12_anim.NCGR\"\n" )
  fp_w.print( "\"ball_12_anim.NCLR\"\n" )
  fp_w.print( "\"ball_15_anim.NCGR\"\n" )
  fp_w.print( "\"ball_15_anim.NCLR\"\n" )

  fp_w.close
