#! ruby -Ks

def get_palette( read_file )
  #キャラファイルからパレット情報を取り出す
  fp_r = open( read_file, "rb" )

  #ヘッダを読み飛ばし
  data = fp_r.read( 16 )
  head = data.unpack( "a4" )

  if head[ 0 ] != "NCCG"
    print "NCGファイルではありません\n"
    exit( 1 )
  end

  #ヘッダを読み飛ばし
  data = fp_r.read( 8 )
  head, size = data.unpack( "a4l" )

  if head != "CHAR"
    print "CHAR:NCGファイルではありません\n"
    exit( 1 )
  end
    
  data = fp_r.read( size - 8 )

  #ヘッダを読み飛ばし
  data = fp_r.read( 16 )
  head, size, width, height = data.unpack( "a4l3" )

  if head != "ATTR"
    print "ATTR:NCGファイルではありません\n"
    exit( 1 )
  end

  size -= 16
    
  pltt = 0
  data = fp_r.read( 5 )
  data = fp_r.read( 1 )
  pal = data.unpack( "C" )
  pltt = pal[ 0 ]
  pltt
end

	if ARGV.size < 2
		print "error: ruby poke_icon_conv.rb file_list tsc_file\n"
		print "file_list:読み込むファイルリスト\n"
		print "tsc_file:タイムスタンプチェックするファイル\n"
		exit( 1 )
	end

  ARGV_READ_FILE = 0
  ARGV_TSC_FILE = 1

  file_list = []
  cnt = 0
  open( ARGV[ ARGV_READ_FILE ] ) {|fp_r|
    while str = fp_r.gets
      str = str.tr( "\"\r\n", "" )
      file_list[ cnt ] = str
      cnt += 1
    end
  }

  fp_pal = open( "pokeicon.dat", "w" )
  fp_pal.print "//============================================================================================\n"
  fp_pal.print "/**\n"
  fp_pal.print "* Pokemon Icon Palette Attribute\n"
  fp_pal.print "*	2005/12/27 by nakahiro\n"
  fp_pal.print "*	シャチバージョン by soga\n"
  fp_pal.print "* シャチでオスメス書き分けに対応するため、下位4bitを♂、上位4bitを♀にしてます\n"
  fp_pal.print "*/\n"
  fp_pal.print "//============================================================================================\n"
  fp_pal.print "const u8 IconPalAtr[] = {\n"

  dir = ""

  file_list.size.times {|i|
    if i == 0
      dir = file_list[ i ]
    end
    next if i == 0
    file_name = file_list[ i ].sub( ".NCGR", ".ncg" )

    conv = 0

    if File::exist?( ARGV[ ARGV_TSC_FILE ] ) == true
      if File::stat( ARGV[ ARGV_TSC_FILE ] ).mtime.tv_sec >= File::stat( file_name ).mtime.tv_sec
        conv = 1
      end
    end

    if conv == 0
      #オスファイルコンバート
      cmd = "./nce_lnk poke_icon_128k.nce " + file_name
      system( cmd )
      p cmd

      cmd = "g2dcvtr " + file_name.sub( ".ncg", ".nce" ) + " -br"
      system( cmd )
      p cmd
    end

    osu_pltt = get_palette( file_name )

    if conv == 0
      #メスファイルが存在するならコンバート
      file_name = file_list[ i ].sub( "_m.NCGR", "_f.ncg" )
      if File.exist?( file_name ) == true 
        cmd = "./nce_lnk poke_icon_128k.nce " + file_name
        system( cmd )
        p cmd

        cmd = "g2dcvtr " + file_name.sub( ".ncg", ".nce" ) + " -br"
        system( cmd )
        p cmd

      else
        fp_w = open( file_name.sub( ".ncg", ".NCGR" ), "w" )
        fp_w.close
      end
    end
    #メスファイルが存在するならコンバート
    file_name = file_list[ i ].sub( "_m.NCGR", "_f.ncg" )
    if File.exist?( file_name ) == true 
      mesu_pltt = get_palette( file_name )
    else
      mesu_pltt = osu_pltt
    end
    fp_pal.printf("\t0x%d%d,\t\t// %d : %s\n", mesu_pltt, osu_pltt, i - 1, file_list[ i ].sub( ".NCGR", "" ) )
  }
  
  fp_pal.print "};\n"
  fp_pal.close
