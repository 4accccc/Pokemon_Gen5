#! ruby -Ks
#=======================================================================================
#
#	NONSTOPアニメーションのポケモングラフィックを抽出
#
#=======================================================================================

  require File.dirname(__FILE__) + '/../hash/monsno_hash.rb'
  require File.dirname(__FILE__) + '/../constant'

	if ARGV.size < 1
		print "error: ruby nonstop_list_mk.rb read_file\n"
		print "read_file:読み込むファイル\n"
		exit( 1 )
	end

  ARGV_READ_FILE = 0

  NCEC_HEADER_SIZE = 12
  NCEC_DATA_SIZE = 12 * 4

class PARA
  enum_const_set %w[
    GRA_FILE
    MONS_NO
    FORM
  ]
end

  dir = {
    "pfwb"=>"正面",
    "pbwb"=>"背面",
  }

  read_data = []
  cnt = 0
  open( ARGV[ ARGV_READ_FILE ] ) {|fp_r|
    while str = fp_r.gets
      read_data[ cnt ] = str.tr( "\"\r\n", "" )
      cnt += 1
    end
  }

  fp_w = open( "pokeanime_nonstop.csv", "w" )

  #見出し生成 
  fp_w.print( "ポケモン名,向き,フォルム名,NONSTOP\n" )

  read_data.size.times {|i|
    split_data = read_data[ i ].split(/,/)
    fp_r = open( split_data[ PARA::GRA_FILE ], "rb" )

	  data = fp_r.read( NCEC_HEADER_SIZE )
	  cells, size_x, size_y, ofs_x, ofs_y = data.unpack( "LS2s2" )

    fp_r.read( NCEC_DATA_SIZE * cells )

    data = fp_r.read( 1 )
	  nonstop = data.unpack( "C" )

    fp_r.close

    if nonstop[ 0 ] == 0xff
      fp_w.printf( "%s,%s,%s,○\n", $monsname[ split_data[ PARA::MONS_NO ].to_i ],
                                    dir[ split_data[ PARA::GRA_FILE ][ 0..3 ] ],
                                    split_data[ PARA::FORM ] )
    else
      fp_w.printf( "%s,%s,%s,×\n", $monsname[ split_data[ PARA::MONS_NO ].to_i ],
                                    dir[ split_data[ PARA::GRA_FILE ][ 0..3 ] ],
                                    split_data[ PARA::FORM ] )
    end
  }
  fp_w.close
