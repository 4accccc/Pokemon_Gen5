#=======================================================================
# land_list.rb
# 地形データ一覧を作成する。
# land_list.rb land.csv..
# land.csv テキスト変換済みの地形データ管理表 複数指定可能
#=======================================================================

#=======================================================================
# 定数
#=======================================================================
FNAME_LAND_LIST = "land_list"
FNAME_ARC_LIST = "land_arc_list"
FNAME_MDL_LIST = "land_model_list"
FNAME_3DPPACK_LIST = "land_depend_list"
FNAME_MAKE_DEPEND = "land_make_depend"

LNAME_LAND_LIST = "CONCAT_TARGET"
LNAME_MDL_LIST = "G3D_IMD"
LNAME_3DPPACK_LIST = "LAND_3DPPACK_LIST"

DIR_LANDRES = "DIR_LANDRES"

#=======================================================================
# 
#=======================================================================
file_land_list = File.open( FNAME_LAND_LIST, "w" )
file_arc_list = File.open( FNAME_ARC_LIST, "w" )
file_model_list = File.open( FNAME_MDL_LIST, "w" )
file_3dppack_list = File.open( FNAME_3DPPACK_LIST, "w" )
file_make_depend = File.open( FNAME_MAKE_DEPEND, "w" )

file_land_list.printf( "%s = \\\n", LNAME_LAND_LIST )
file_model_list.printf( "%s = \\\n", LNAME_MDL_LIST )
file_3dppack_list.printf( "%s = \\\n", LNAME_3DPPACK_LIST )

yen_flg = 0

while file_name = ARGV.shift
	file_csv = File.open( file_name, "r" )
  
	line = file_csv.gets #データ開始位置まで進める
	line = file_csv.gets
	line = file_csv.gets
  
	while line = file_csv.gets
		if( line =~ /^#END/ )
			break
		end
    
		if( yen_flg != 0 )
			file_land_list.printf( " \\\n" )
			file_model_list.printf( " \\\n" )
			file_3dppack_list.printf( " \\\n" )
		end
    
		yen_flg = 1
		column = line.split ","
		name = column[1]
    
    #land_list
		file_land_list.printf( "\t$(%s)/%s.bin", DIR_LANDRES, name )
		file_land_list.printf( " $(%s)/%s.imd", DIR_LANDRES, name )
    
    #land_arc_list
		file_arc_list.printf( "\"land_3dppack/%s.3dppack\"\n", name )
    
    #land_model_list
		file_model_list.printf( "\t$(%s)/%s.imd", DIR_LANDRES, name )
    
    #land_3dppack_list
		file_3dppack_list.printf( "\tland_3dppack/%s.3dppack", name )

    #land_make_depend
    file_make_depend.printf( "#%s\n", name )
    
    file_make_depend.printf( "land_3dppack/%s.3dppack: land_g3dcvtr/%s.nsbmd land_res/%s.bin land_res/%s.3dmd\n", name, name, name, name )
    file_make_depend.printf( "\t@echo create 3dppack %s\n", name )
    file_make_depend.printf( "\t@$(BINLINKER) land_g3dcvtr/%s.nsbmd land_res/%s.bin land_res/%s.3dmd land_3dppack/%s.3dppack WB\n\n", name, name, name, name )

    file_make_depend.printf( "land_g3dcvtr/%s.nsbmd: land_res/%s.imd\n", name, name )
    file_make_depend.printf( "\t@echo create_nsbmd %s\n", name )
    file_make_depend.printf( "\t@$(G3DCVTR) land_res/%s.imd -o land_g3dcvtr/%s.nsbmd\n\n", name, name )
	end
	file_csv.close
end

file_make_depend.close
file_3dppack_list.close
file_model_list.close
file_arc_list.close
file_land_list.close
