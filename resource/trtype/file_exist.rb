#! ruby -Ks
#============================================================================================
#
#	トレーナーグラフィック　仮素材自動生成用Rubyスクリプト
#
#	2011.05.23  tamada@gamefreak.co.jp
#
#============================================================================================

  NC_REWRITE  = ENV["PROJECT_ROOT"] + '/tools/g2d/nclnkrp.rb'
  ext_array = [ ".nce", ".ncg", ".ncl", ".nmc" ]

	if ARGV.size < 2
		print "error: ruby file_exist.rb list_file copy_dir\n"
		print "list_file:チェック用リストファイル\n"
		print "copy_dir:仮素材のコピー先\n"
		exit( 1 )
	end

  ARGV_LIST_FILE = 0
  ARGV_COPY_DIR = 1

  read_data = []
  open( ARGV[ ARGV_LIST_FILE ] ) {|fp_r|
    while str = fp_r.gets
      read_data << str.tr( "\"\r\n", "" )
    end
  }

  exist_flag = 0

  read_data.size.times {|i|
    split_data = read_data[ i ].split(/,/)

    new_name = split_data[ 0 ]
    gender = split_data[ 1 ]
    file_name = ARGV[ ARGV_COPY_DIR ] + new_name + ".ncg" 

    if File.exist?( file_name ) == FALSE
      exist_flag = 1
      if new_name =~ /^trwb_/ then
        prefix = "trwb_"
      else
        prefix = "tbwb_"
      end
      if gender == "PTL_SEX_MALE"
        default_name = prefix + "male"
      else
        default_name = prefix + "female"
      end
      ext_array.each {|ext|
        #p ext
        target_file = ARGV[ ARGV_COPY_DIR ] + new_name + ext
        com = sprintf( "cp default_data/%s%s %s", default_name, ext, target_file )
        p com
        system( com )

        com = ""
        case ext
        when ".nce"
          com = "ruby #{NC_REWRITE} #{target_file} #{new_name}.ncg"
        when ".ncg"
          com = "ruby #{NC_REWRITE} #{target_file} #{new_name}.ncl"
        when ".nmc"
          com = "ruby #{NC_REWRITE} #{target_file} #{new_name}.nce"
        end
        if com != "" then
          p com
          system( com )
        end

        com = sprintf( "svn add %s", target_file )
        system( com )
        com = sprintf( "svn commit %s -m \"仮素材追加\"", target_file )
        system( com )
      }
    end
  }
  if exist_flag != 0
    system( "svn commit trfgra_wb.scr -m \"仮素材追加\"" )
#    system( "ipmsg /MSG /SEAL W00205 トレーナーグラフィック仮素材をコミットしました　コンバートお願いします" )
#    system( "ipmsg /MSG localhost トレーナーグラフィック仮素材があります　大村君からの返答を待ってください" )
  end

