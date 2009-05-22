
#=======================================================================================
#
#	NARC用scrファイルを生成
#
#=======================================================================================

	$KCODE = "Shift-JIS"

#クラス定義

#サブルーチン

#メインルーチン
	if ARGV.size < 1
		print "error: ruby clact_conv.rb list_file write_file\n"
		print "list_file:読み込むファイル\n"
		print "write_file:書き込むファイル\n"
		exit( 1 )
	end

	ARGV_LIST_FILE = 0
	ARGV_WRITE_FILE = 1

	begin
		fp_r = open( ARGV[ ARGV_LIST_FILE ] )
	rescue
		print "FileOpenError:" + ARGV[ ARGV_LIST_FILE ]
		exit( 1 )
	end

	fp_w = open( ARGV[ ARGV_WRITE_FILE ], "w" )

	fp_r.read.each { |data|
		name = File::basename( data, '.*' )
		fp_w.print "\"" + name + ".NCGR\"\n"
		fp_w.print "\"" + name + ".NCLR\"\n"
		fp_w.print "\"" + name + ".NCER\"\n"
		fp_w.print "\"" + name + ".NANR\"\n"
	}

	fp_r.close
	fp_w.close
