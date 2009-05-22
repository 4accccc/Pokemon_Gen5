
#=======================================================================================
#
#	CLACTで使用するキャラデータをコンバート
#
#=======================================================================================

	$KCODE = "Shift-JIS"

#クラス定義

#サブルーチン
# ファイル又はディレクトリを削除するメソッドを作成
def deleteall(delthem)
  if FileTest.directory?(delthem) then  # ディレクトリかどうかを判別
    Dir.foreach( delthem ) do |file|    # 中身を一覧
      next if /^\.+$/ =~ file           # 上位ディレクトリと自身を対象から外す
      deleteall( delthem.sub(/\/+$/,"") + "/" + file )
    end
    Dir.rmdir(delthem) rescue ""        # 中身が空になったディレクトリを削除
  else
    File.delete(delthem)                # ディレクトリでなければ削除
  end
end

def error_action()
	deleteall("./clact_conv_temp")
	exit( 1 )
end

#メインルーチン
	if ARGV.size < 5
		print "error: ruby clact_conv.rb nce_file ncg_file ncl_file g2dcvtr_dir output_dir\n"
		print "nce_file:読み込むnceファイル\n"
		print "ncg_file:読み込むncgファイル\n"
		print "ncl_file:読み込むnclファイル\n"
		print "g2dcvtr_dir:g2dcvtr.exeが存在するディレクトリ\n"
		print "output_dir:g2dcvtr.exeでコンバートしたファイルを格納するディレクトリ\n"
		error_action()
	end

	ARGV_NCE_FILE = 0
	ARGV_NCG_FILE = 1
	ARGV_NCL_FILE = 2
	ARGV_G2D_DIR = 3
	ARGV_OUT_DIR = 4

	read_data = []
	data = []
	sig = []

	Dir::mkdir("./clact_conv_temp")

	nce_file = File::basename( ARGV[ ARGV_NCE_FILE ] )
	ncg_file = File::basename( ARGV[ ARGV_NCG_FILE ] )
	ncl_file = File::basename( ARGV[ ARGV_NCL_FILE ] )

#NCE Check
	begin
		fp_r = open( ARGV[ ARGV_NCE_FILE ] )
	rescue
		print "FileOpenError:" + ARGV[ ARGV_NCE_FILE ]
		error_action()
	end

	fp_r.binmode

	w_file = "./clact_conv_temp/" + nce_file

	fp_w = open( w_file, "wb" )

	read_data = fp_r.read( 16 )
	data = read_data.unpack("c4")

	cnt = 0
	"NCOB".each_byte  { |c|
		if data[ cnt ] != c
			print "NCE NG!\n"
			error_action()
		end
		cnt += 1
	}

	read_data.size.times{ |c|
		fp_w.printf("%c",read_data[ c ])
	}

	nce_sig = [ "CELL", "CNUM", "CHAR", "GRP ", "ANIM", "ACTL", "MODE", "LABL",
							"CMNT", "CCMT", "ECMT", "FCMT", "CLBL", "EXTR" ]

	nce_sig.size.times{ |sig_cnt|
		read_data = fp_r.read( 8 )
		sig, size = read_data.unpack("a4l")

		cnt = 0
		nce_sig[ sig_cnt ].each_byte  { |c|
			if sig[ cnt ] != c
				print nce_sig[ sig_cnt ] + "NG!\n"
				error_action()
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

	read_data = fp_r.read( 4 )
	data = read_data.unpack("c4")

	cnt = 0
	"LINK".each_byte  { |c|
		if data[ cnt ] != c
			print "LINKNG!\n"
			error_action()
		end
		cnt += 1
	}

	size = 8 + ncg_file.size 
	padding = 4 - ( size % 4 )
	size += padding
	
	data = ["LINK", size, ncg_file ].pack("a* l a*") 

	padding.times{
		data << 0
	}

	data.size.times{ |c|
		fp_w.printf("%c",data[ c ])
	}

	fp_r.close
	fp_w.close

#NCG Check
	begin
		fp_r = open( ARGV[ ARGV_NCG_FILE ] )
	rescue
		print "FileOpenError:" + ARGV[ ARGV_NCG_FILE ]
		error_action()
	end

	fp_r.binmode

	w_file = "./clact_conv_temp/" + ncg_file

	fp_w = open( w_file, "wb" )

	read_data = fp_r.read( 16 )
	data = read_data.unpack("c4")

	cnt = 0
	"NCCG".each_byte  { |c|
		if data[ cnt ] != c
			print "NCG NG!\n"
			error_action()
		end
		cnt += 1
	}

	read_data.size.times{ |c|
		fp_w.printf("%c",read_data[ c ])
	}

	nce_sig = [ "CHAR", "ATTR" ]

	nce_sig.size.times{ |sig_cnt|
		read_data = fp_r.read( 8 )
		sig, size = read_data.unpack("a4l")

		cnt = 0
		nce_sig[ sig_cnt ].each_byte  { |c|
			if sig[ cnt ] != c
				print nce_sig[ sig_cnt ] + "NG!\n"
				error_action()
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

	read_data = fp_r.read( 4 )
	data = read_data.unpack("c4")

	cnt = 0
	"LINK".each_byte  { |c|
		if data[ cnt ] != c
			print "LINKNG!\n"
			error_action()
		end
		cnt += 1
	}

	size = 8 + ncl_file.size 
	padding = 4 - ( size % 4 )
	size += padding
	
	data = [ "LINK", size, ncl_file ].pack("a* l a*") 

	padding.times{
		data << 0
	}

	data.size.times{ |c|
		fp_w.printf("%c",data[ c ])
	}

	fp_r.close
	fp_w.close

#NCL Check
#NCLは素直にコピーするだけでいいけど一応
	begin
		fp_r = open( ARGV[ ARGV_NCL_FILE ] )
	rescue
		print "FileOpenError:" + ARGV[ ARGV_NCL_FILE ]
		error_action()
	end

	fp_r.binmode

	w_file = "./clact_conv_temp/" + ncl_file

	fp_w = open( w_file, "wb" )

	fp_w.write( fp_r.read )

	fp_r.close
	fp_w.close

	com = ARGV[ ARGV_G2D_DIR ] + "g2dcvtr ./clact_conv_temp/" + nce_file + " -pcm -o/" + ARGV[ ARGV_OUT_DIR ]

	p com

	system( com )

	deleteall("./clact_conv_temp")
