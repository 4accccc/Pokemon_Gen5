
#=======================================================================================
#
#	NitroCharacterファイルのLINK情報を書き換える
#
#=======================================================================================

	$KCODE = "Shift-JIS"

#クラス定義

#サブルーチン

#メインルーチン
	if ARGV.size > 2
		print "error: ruby nclnkrp.rb read_file replace_file\n"
		print "read_file:読み込むファイル\n"
		print "replace_file:書き換えるファイル名\n"
		error_action()
	end

	ARGV_READ_FILE = 0
	ARGV_REPLACE_FILE = 1

  FILE_TYPE_NMC = 0
  FILE_TYPE_NCE = 1
  FILE_TYPE_NCG = 2

	head = [ "NCMC", "NCOB", "NCCG" ]

	signature = [
		#NMC
		[ "MCEL", "CCTL", "GRP ", "ANIM", "ACTL", "LABL", "CMNT", "CCMT", "ECMT", "FCMT", "CLBL" ],
		#NCE
		[ "CELL", "CNUM", "CHAR", "GRP ", "ANIM", "ACTL", "MODE", "LABL", "CMNT", "CCMT", "ECMT", "FCMT", "CLBL", "EXTR" ],
		#NCG
		[ "CHAR", "ATTR" ]
	]

  file_type = {
    ".nmc"=>0,
    ".nce"=>1,
    ".ncg"=>2,
    ".NMC"=>0,
    ".NCE"=>1,
    ".NCG"=>2,
  }

	read_data = []
	data = []
	sig = []

	read_file = ARGV[ ARGV_READ_FILE ]

	begin
		fp_r = open( read_file )
	rescue
		print "FileOpenError:" + read_file + "\n"
		exit( 1 )
	end

	fp_r.binmode

	fp_w = open( "temp", "wb" )

	read_data = fp_r.read( 16 )
	data = read_data.unpack("c4")

	type = file_type[ File::extname( ARGV[ ARGV_READ_FILE ] ) ]

  if type == nil
    exit( 0 )
  end

	cnt = 0
	ng_flag = 0
	head[ type ].each_byte  { |c|
		if data[ cnt ] != c
			ng_flag = 1
			break
		end
		cnt += 1
	}

	if ng_flag == 1
		fp_r.close
		fp_w.close
		exit( 0 )
	end

	#
	read_data.size.times{ |c|
		fp_w.printf("%c",read_data[ c ])
	}

	signature[ type ].size.times{ |sig_cnt|
		read_data = fp_r.read( 8 )
		sig, size = read_data.unpack("a4l")

		if type == FILE_TYPE_NCE && signature[ type ][ sig_cnt ] == "CHAR"
			if sig == "GRP "
				fp_r.seek( -8, IO::SEEK_CUR )
				next		
			end
		end

		cnt = 0
		signature[ type ][ sig_cnt ].each_byte  { |c|
			if sig[ cnt ] != c
				print signature[ type ][ sig_cnt ] + "NG!\n"
				exit( 1 )
			end
			cnt += 1
		}

		read_data.size.times{ |c|
			fp_w.printf("%c",read_data[ c ])
		}

		size -= 8

		read_data = fp_r.read( size )

		read_data.size.times{ |c|
			fp_w.printf("%c",read_data[ c ])
		}
	}

	read_data = fp_r.read( 8 )
	data, size = read_data.unpack("a4l")

	cnt = 0
	"LINK".each_byte  { |c|
		if data[ cnt ] != c
			print "LINKNG!\n"
			exit( 1 )
		end
		cnt += 1
	}

  if ARGV.size == 1
    if type == FILE_TYPE_NMC
      file_name = File::basename( ARGV[ ARGV_READ_FILE ], ".nmc" ) + ".nce"
    elsif type == FILE_TYPE_NCE
      file_name = File::basename( ARGV[ ARGV_READ_FILE ], "nce" )
      file_name = file_name.sub(/_(\d\d\d)/,'_\1\2\3c')
      file_name = file_name.sub(/\./,"_m\.") + "ncg"
    else
      file_name = File::basename( ARGV[ ARGV_READ_FILE ], "ncg" )
      file_name = file_name.sub(/_(\d\d\d)c/,'_\1\2\3')
      file_name = file_name.sub(/pfwb_/,'pmwb_')
      file_name = file_name.sub(/pbwb_/,'pmwb_')
      file_name = file_name.sub(/_m\./,"_n\.") + "ncl"
    end
  else
  	file_name = ARGV[ ARGV_REPLACE_FILE ]
  end

	size = 8 + file_name.size 
	padding = 4 - ( size % 4 )
	size += padding

	data = [ "LINK", size, file_name ].pack("a* l a*") 

	padding.times{
		data << 0
	}

	data.size.times{ |c|
		fp_w.printf("%c",data[ c ])
	}

  if type == FILE_TYPE_NCG
	  data = [ "CMNT", 12, 0 ].pack("a* l l") 

	  data.size.times{ |c|
		  fp_w.printf("%c",data[ c ])
	  }
  end

	fp_r.close
	fp_w.close

  system( "rm -f " + ARGV[ ARGV_READ_FILE ] )
  system( "mv temp " + ARGV[ ARGV_READ_FILE ] )

