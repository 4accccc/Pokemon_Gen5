#! ruby -Ks

#=======================================================================================
#
#	タイムスタンプ比較して別のrubyを実行
#
#=======================================================================================

#クラス定義

#サブルーチン

#メインルーチン
	if ARGV.size < 3
		print "error: ruby tschk.rb list_file res_dir copy_dir\n"
		print "list_file:タイムスタンプチェックするファイルリスト\n"
		print "res_dir:タイムスタンプチェックするファイルが存在するディレクトリ\n"
		print "copy_dir:コピー先ディレクトリ\n"
		exit( 1 )
	end

  ARGV_LIST_FILE  = 0
  ARGV_RES_DIR    = 1
  ARGV_COPY_DIR   = 2

  begin
    fp_list = open( ARGV[ ARGV_LIST_FILE ] )
  rescue
    printf( "FileOpenError:%s\n", ARGV[ ARGV_LIST_FILE ] )
    exit( 1 )
  end

  read_data = []
  while str = fp_list.gets
    read_data << ARGV[ ARGV_RES_DIR ] + str.tr( "\r\n", "" )
  end
  fp_list.close

  read_data.size.times {|i|
    frame_adj = 0
    if File.exist?( read_data[ i ] ) == TRUE
      com = "ruby " +  File::dirname(__FILE__) + "/nclinkrw.rb " + read_data[ i ] + " " + ARGV[ ARGV_COPY_DIR ]
      read_file = ARGV[ ARGV_COPY_DIR ] + File::basename( read_data[ i ] )
      if File.exist?( read_file ) == FALSE
        printf("copy %s\n", File::basename( read_data[ i ] ) )
        system( com )
        frame_adj = 1
      elsif File::stat( read_file ).mtime.tv_sec < File::stat( read_data[ i ] ).mtime.tv_sec
        printf("copy %s\n", File::basename( read_data[ i ] ) )
        system( com )
        frame_adj = 1
      end
    end
    if File::extname( read_data[ i ] ) == ".nmc" && frame_adj == 1
      com = "ruby " +  File::dirname(__FILE__) + "/frame_adj.rb " + read_data[ i ] + " " + ARGV[ ARGV_COPY_DIR ]
      system( com )
      if $?.exitstatus == 1
        exit( 1 )
      end
    end
  }

