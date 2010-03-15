#! ruby -Ks

  require File.dirname(__FILE__) + '/../constant'

class PARA
  enum_const_set %w[
    FREQUENCY
    VOLUME
    MAX
  ]
end

	if ARGV.size < 1
		print "error: ruby pokevoice.rb read_file\n"
		print "read_file:読み込むファイル\n"
		exit( 1 )
	end

  ARGV_READ_FILE = 0
  ARGV_WRITE_FILE = 1

  read_data = []
  cnt = 0
  open( ARGV[ ARGV_READ_FILE ] ) {|fp_r|
    while str = fp_r.gets
      str = str.tr( "\"\r\n", "" )
      split_data = str.split(/,/)
      next if split_data.size <= 1       #サーバからのエクスポートでゴミレコードが入ることがあるので、排除する
      read_data[ cnt ] = str
      cnt += 1
    end
  }

  fp_w = open( "pvp_000.bin", "w" )
  data = [ 0, 0 ].pack( "S2" )
  data.size.times{ |c|
    fp_w.printf("%c",data[ c ])
  }

  read_data.size.times {|i|
    file_name = "pvp_%03d.bin" % [ i + 1 ]
    fp_w = open( file_name, "w" )
    split_data = read_data[ i ].split(/,/)
    data = [ split_data[ PARA::FREQUENCY ].to_i, split_data[ PARA::VOLUME ].to_i ].pack( "S2" )
	  data.size.times{ |c|
		  fp_w.printf("%c",data[ c ])
	  }
    fp_w.close
  }

