#! ruby -Ks

  ext = [ "NANR", "NCBR", "NCEC", "NCER", "NCGR", "NCLR", "NMAR", "NMCR" ]

	if ARGV.size < 1
		print "error: ruby file_exist.rb lst_file\n"
		print "read_file:チェック用リストファイル\n"
		exit( 1 )
	end

  ARGV_LIST_FILE = 0

  read_data = []
  open( ARGV[ ARGV_LIST_FILE ] ) {|fp_r|
    while str = fp_r.gets
      read_data << str.tr( "\"\r\n", "" )
    end
  }

  read_data.size.times {|i|
    split_data = read_data[ i ].split(/,/)

    file_name = split_data[ 0 ] + ".NCGR" 

    if File.exist?( file_name ) == FALSE
      if split_data[ 1 ] == "PTL_SEX_MALE"
        gender = "trwb_male_"
      else
        gender = "trwb_female_"
      end
      ext.size.times {|j|
        com = sprintf( "cp %s%s %s.%s", gender, ext[ j ], split_data[ 0 ], ext[ j ] )
        system( com )
      }
    end
  }
