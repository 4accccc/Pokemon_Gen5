#=======================================================================
# land_list.rb
# 地形データ一覧を作成する。
# land_list.rb land.csv..
# land.csv テキスト変換済みの地形データ管理表 複数指定可能
#=======================================================================

#=======================================================================
# 定数
#=======================================================================
FNAME_OUTPUT_LIST = "land_output_list"
FNAME_MODEL_LIST = "land_model_list"
FNAME_DEPEND_LIST = "land_depend_list"
FNAME_MAKE_DEPEND = "land_make_depend"

DIR_OUTPUT = "land_output"

DIRSTR_OUTPUT = "$(DIR_OUTPUT)"
DIRSTR_TEMP = "$(DIR_TEMP)"
DIRSTR_RES = "$(DIR_RES)"

LNAME_MODEL_LIST = "LAND_MODEL_LIST"
LNAME_DEPEND_LIST = "LAND_DEPEND_LIST"

STR_BINLINKER = "$(BINLINKER)"
STR_G3DCVTR = "$(G3DCVTR)"

#=======================================================================
# 
#=======================================================================
file_output_list = File.open( FNAME_OUTPUT_LIST, "w" )
file_model_list = File.open( FNAME_MODEL_LIST, "w" )
file_depend_list = File.open( FNAME_DEPEND_LIST, "w" )
file_make_depend = File.open( FNAME_MAKE_DEPEND, "w" )

file_model_list.printf( "%s = \\\n", LNAME_MODEL_LIST )
file_depend_list.printf( "%s = \\\n", LNAME_DEPEND_LIST )

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
			file_model_list.printf( " \\\n" )
			file_depend_list.printf( " \\\n" )
		end
    
		yen_flg = 1
		column = line.split ","
		name = column[1]
    
    #land_output_list
		file_output_list.printf( "\"%s/%s.3dppack\"\n", DIR_OUTPUT, name )
    
    #land_model_list
		file_model_list.printf( "\t%s/%s.imd", DIRSTR_RES, name )
    
    #land_3dppack_list
		file_depend_list.printf( "\t%s/%s.3dppack", DIRSTR_OUTPUT, name )
    
    #land_make_depend
    file_make_depend.printf( "#%s\n", name )
    
    file_make_depend.printf(
      "%s/%s.3dppack: %s/%s.nsbmd %s/%s.bin %s/%s.3dmd\n",
      DIRSTR_OUTPUT, name,
      DIRSTR_TEMP, name,
      DIRSTR_RES, name,
      DIRSTR_RES, name )
    
    file_make_depend.printf( "\t@echo create 3dppack %s\n", name )
    
    file_make_depend.printf(
      "\t@%s %s/%s.nsbmd %s/%s.bin %s/%s.3dmd %s/%s.3dppack WB\n\n",
      STR_BINLINKER,
      DIRSTR_TEMP, name,
      DIRSTR_RES, name,
      DIRSTR_RES, name,
      DIRSTR_OUTPUT, name )
    
    file_make_depend.printf( "%s/%s.nsbmd: %s/%s.imd\n",
      DIRSTR_TEMP, name,
      DIRSTR_RES, name )
    
    file_make_depend.printf( "\t@echo create_nsbmd %s\n", name )
    
    file_make_depend.printf( "\t@%s %s/%s.imd -o %s/%s.nsbmd\n\n",
      STR_G3DCVTR, DIRSTR_RES, name, DIRSTR_TEMP, name )
	end
	file_csv.close
end

file_make_depend.close
file_depend_list.close
file_model_list.close
file_output_list.close
