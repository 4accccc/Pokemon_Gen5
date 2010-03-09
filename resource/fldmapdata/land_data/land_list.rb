#=======================================================================
# land_list.rb
# 地形データ一覧を作成する。
# land_list.rb land.csv..
# land.csv テキスト変換済みの地形データ管理表 複数指定可能
#=======================================================================
$KCODE = "SJIS"

#=======================================================================
# 定数
#=======================================================================
#作成するファイル名
FNAME_OUTPUT_LIST = "land_output_list"
FNAME_TEMP_LIST = "land_model_list"
FNAME_DEPEND_LIST = "land_depend_list"
FNAME_MAKE_DEPEND = "land_make_depend"
FNAME_HEIGHT_LIST = "heightlist.lst"
FNAME_HEIGHT_RESULT = "convresult"
FNAME_DEPEND_WBBIN_TBL_LIST = "wbbin_tbl_depend_list"

#land_outputディレクトリ名
DIR_OUTPUT = "land_output"
DIR_RES = "land_res"

#makefileで定義されている各ディレクトリ名
DIRSTR_OUTPUT = "$(DIR_OUTPUT)"
DIRSTR_TEMP = "$(DIR_TEMP)"
DIRSTR_RES = "$(DIR_RES)"
DIRSTR_WBBIN = "$(DIR_WBBIN)"
DIRSTR_WBTMP = "$(DIR_WBTMP)"

#makefileで使用するラベル名
LNAME_TEMP_LIST = "LAND_TEMP_LIST"
LNAME_DEPEND_LIST = "LAND_DEPEND_LIST"

#makefileで定義されている文字列
STR_BINLINKER = "$(BINLINKER)"
STR_G3DCVTR = "$(G3DCVTR)"
STR_HEIGHTCNV = "$(HEIGHTCNV)"
STR_HEIGHTRETFILE = "$(HEIGHTRETFILE)"
STR_CHECK_SIZE  = "$(CHECK_SIZE)"

#ダミーファイル
FNAME_DMYFILE_GRID_3DMD = "dmygmap.3dmd"
FNAME_DMYFILE_GRID_BIN = "dmygmap.bin"
FNAME_DMYFILE_GRID_IMD = "dmygmap.imd"
FNAME_DMYFILE_NOGRID_3DMD = "dummy_ng.3dmd"
FNAME_DMYFILE_NOGRID_IMD = "dummy_ng.imd"
FNAME_DMYFILE_NOGRID_H_IMD = "dummy_h_ng.imd"

#マップタイプ
MAPTYPESTR_GRID = "MAPTYPE_GRID"
MAPTYPESTR_CROSS = "MAPTYPE_CROSS"
MAPTYPESTR_NOGRID = "MAPTYPE_NOGRID"
MAPTYPESTR_RANDOM = "MAPTYPE_RANDOM"

#4byte補正rubyファイル名
RUBYNAME_PAD4BYTE = "pad4byte.rb"

#wbbin生成に依存するファイル
STR_WBBIN_CONVERTER = "ruby wb_bin_conv.rb"
STR_WBBIN_ALLTBL = "bintmp/normal_d_bin.alltbl"
STR_WBBIN_TBLCHNAGE_LOG = "normal_d_bin.alltbl"
STR_WBBIN_CDAT = "$(WBBIN_CDAT)"
STR_DIFFCOPY_CONVERTER = "ruby $(PROJECT_ROOT)tools/diffcopy.rb"


#=======================================================================
#	エラー
#=======================================================================
def error_end(
  file_output_list, fpath_output_list,
  file_temp_list, fpath_temp_list,
  file_depend_list, fpath_depend_list,
  file_make_depend, fpath_make_depend, 
  file_wbbin_tbl_depend, fpath_wbbin_tbl_depend )
  file_make_depend.close
  file_depend_list.close
  file_temp_list.close
  file_output_list.close
  file_wbbin_tbl_depend.close
  File.delete( fpath_output_list )
  File.delete( fpath_temp_list )
  File.delete( fpath_depend_list )
  File.delete( fpath_make_depend )
  File.delete( fpath_wbbin_tbl_depend )
end

#=======================================================================
#	ファイルコピー
#=======================================================================
def file_copy( srcpath, copypath )
#  FileUtils.copy( srcpath, copypath )
	open( srcpath, "rb" ){ |input|
		open( copypath, "wb" ){ |output|
			output.write( input.read )
		}
	}
end

#=======================================================================
# マップタイプ　グリッド用一覧、出力ルール書き込み
#=======================================================================
def file_write_grid( name,
  file_output_list, file_temp_list, file_depend_list, file_make_depend, file_wbbin_tbl_depend, binary_type )
  #land_output_list
	file_output_list.printf( "\"%s/%s.3dppack\"\n", DIR_OUTPUT, name )
  
  #land_temp_list
	file_temp_list.printf( "\t%s/%s.nsbmd", DIRSTR_TEMP, name )
  
  #land_depend_list
	file_depend_list.printf( "\t%s/%s.3dppack", DIRSTR_OUTPUT, name )
  
  #land_make_depend 3dppack
  file_make_depend.printf( "#%s\n", name )
   
  file_make_depend.printf(
    "%s/%s.3dppack: %s/%s.nsbmd %s/%s.wbbin %s/%s.3dmd\n",
    DIRSTR_OUTPUT, name,
    DIRSTR_TEMP, name,
    DIRSTR_WBBIN, name,
    DIRSTR_RES, name )
  
  file_make_depend.printf( "\t@echo create 3dppack %s\n", name )
  
  file_make_depend.printf(
    "\t@%s %s/%s.nsbmd %s/%s.wbbin %s/%s.3dmd %s/%s.3dppack #{binary_type}\n\n",
    STR_BINLINKER,
    DIRSTR_TEMP, name,
    DIRSTR_WBBIN, name,
    DIRSTR_RES, name,
    DIRSTR_OUTPUT, name )
  
  #land_make_depend nsbmd
  file_make_depend.printf( "%s/%s.nsbmd: %s/%s.imd\n",
    DIRSTR_TEMP, name,
    DIRSTR_RES, name )

  file_make_depend.printf( "\t@echo create_nsbmd %s\n", name )
  
  file_make_depend.printf( "\t@%s %s/%s.imd -emdl -o %s/%s.nsbmd\n\n",
    STR_G3DCVTR, DIRSTR_RES, name, DIRSTR_TEMP, name )

  #land_make_depend wbbin
  file_make_depend.printf( "%s/%s.wbbin: %s/%s.bin %s\n",
    DIRSTR_WBBIN, name,
    DIRSTR_RES, name,
    STR_WBBIN_TBLCHNAGE_LOG )

  file_make_depend.printf( "\t@echo create_wbbin %s\n", name )
  
  file_make_depend.printf( "\t@%s BIN %s/%s.bin \n\n",
    STR_WBBIN_CONVERTER, DIRSTR_RES, name )
  
  #land_make_depend exist file check
  check = DIR_RES + "/" + name + ".imd"
  if( FileTest.exist?(check) != true )
    printf( "%s.imdをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_GRID_IMD, check )
  end
 
  check = DIR_RES + "/" + name + ".bin"
  if( FileTest.exist?(check) != true )
    printf( "%s.binをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_GRID_BIN, check )
  end
  
  check = DIR_RES + "/" + name + ".3dmd"
  if( FileTest.exist?(check) != true )
    printf( "%s.3dmdをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_GRID_3DMD, check )
  end



  #wbbin_tbl_make_depend
  file_wbbin_tbl_depend.printf( "%s/%s.tbl: %s/%s.bin\n",
    DIRSTR_WBTMP, name,
    DIRSTR_RES, name )

  file_wbbin_tbl_depend.printf( "\t@echo create_bintbl %s\n", name )
  
  file_wbbin_tbl_depend.printf( "\t@%s TBL %s/%s.bin\n\n",
    STR_WBBIN_CONVERTER, DIRSTR_RES, name )

  str_wbtbl = ""
  str_wbtbl += "#{DIRSTR_WBTMP}/#{name}.tbl "

  return str_wbtbl
end

#=======================================================================
# マップタイプ　グリッド立体交差用一覧、出力ルール書き込み
#=======================================================================
def file_write_cross( name,
  file_output_list, file_temp_list, file_depend_list, file_make_depend, file_wbbin_tbl_depend )
  #land_output_list
	file_output_list.printf( "\"%s/%s.3dppack\"\n", DIR_OUTPUT, name )
  
  #land_temp_list
	file_temp_list.printf( "\t%s/%s.nsbmd", DIRSTR_TEMP, name )
  
  #land_depend_list
	file_depend_list.printf( "\t%s/%s.3dppack", DIRSTR_OUTPUT, name )
  
  #land_make_depend 3dppack
  file_make_depend.printf( "#%s\n", name )
   
  file_make_depend.printf(
    "%s/%s.3dppack: %s/%s.nsbmd %s/%s.wbbin %s/%s_ex.wbbin %s/%s.3dmd\n",
    DIRSTR_OUTPUT, name,
    DIRSTR_TEMP, name,
    DIRSTR_WBBIN, name,
    DIRSTR_WBBIN, name,
    DIRSTR_RES, name )
  
  file_make_depend.printf( "\t@echo create 3dppack %s\n", name )
  
  file_make_depend.printf(
    "\t@%s %s/%s.nsbmd %s/%s.wbbin %s/%s_ex.wbbin %s/%s.3dmd %s/%s.3dppack GC\n\n",
    STR_BINLINKER,
    DIRSTR_TEMP, name,
    DIRSTR_WBBIN, name,
    DIRSTR_WBBIN, name,
    DIRSTR_RES, name,
    DIRSTR_OUTPUT, name )
  
  #land_make_depend nsbmd
  file_make_depend.printf( "%s/%s.nsbmd: %s/%s.imd\n",
    DIRSTR_TEMP, name,
    DIRSTR_RES, name )

  file_make_depend.printf( "\t@echo create_nsbmd %s\n", name )
  
  file_make_depend.printf( "\t@%s %s/%s.imd -emdl -o %s/%s.nsbmd\n\n",
    STR_G3DCVTR, DIRSTR_RES, name, DIRSTR_TEMP, name )


  #land_make_depend wbbin
  file_make_depend.printf( "%s/%s.wbbin: %s/%s.bin %s\n",
    DIRSTR_WBBIN, name,
    DIRSTR_RES, name,
    STR_WBBIN_TBLCHNAGE_LOG )

  file_make_depend.printf( "\t@echo create_wbbin %s\n", name )
  
  file_make_depend.printf( "\t@%s BIN %s/%s.bin\n\n",
    STR_WBBIN_CONVERTER, 
    DIRSTR_RES, name )

  #land_make_depend _ex.wbbin
  file_make_depend.printf( "%s/%s_ex.wbbin: %s/%s_ex.bin %s\n",
    DIRSTR_WBBIN, name,
    DIRSTR_RES, name,
    STR_WBBIN_TBLCHNAGE_LOG )

  file_make_depend.printf( "\t@echo create_wbbin %s_ex\n", name )
  
  file_make_depend.printf( "\t@%s BIN %s/%s_ex.bin\n\n",
    STR_WBBIN_CONVERTER, 
    DIRSTR_RES, name )
  
  #land_make_depend exist file check
  check = DIR_RES + "/" + name + ".imd"
  if( FileTest.exist?(check) != true )
    printf( "%s.imdをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_GRID_IMD, check )
  end
  #デフォルトアトリビュート 
  check = DIR_RES + "/" + name + ".bin"
  if( FileTest.exist?(check) != true )
    printf( "%s.binをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_GRID_BIN, check )
  end
  #Exアトリビュート(立体交差用)
  check = DIR_RES + "/" + name + "_ex.bin"
  if( FileTest.exist?(check) != true )
    printf( "%s_ex.binをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_GRID_BIN, check )
  end
  
  check = DIR_RES + "/" + name + ".3dmd"
  if( FileTest.exist?(check) != true )
    printf( "%s.3dmdをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_GRID_3DMD, check )
  end

  #wbbin_tbl_make_depend
  file_wbbin_tbl_depend.printf( "%s/%s.tbl: %s/%s.bin\n",
    DIRSTR_WBTMP, name,
    DIRSTR_RES, name )
  file_wbbin_tbl_depend.printf( "\t@echo create_bintbl %s\n", name )
  
  file_wbbin_tbl_depend.printf( "\t@%s TBL %s/%s.bin\n\n",
    STR_WBBIN_CONVERTER, DIRSTR_RES, name )

  file_wbbin_tbl_depend.printf( "%s/%s_ex.tbl: %s/%s_ex.bin\n",
    DIRSTR_WBTMP, name,
    DIRSTR_RES, name )
  file_wbbin_tbl_depend.printf( "\t@echo create_bintbl %s\n", name )
  
  file_wbbin_tbl_depend.printf( "\t@%s TBL %s/%s_ex.bin\n\n",
    STR_WBBIN_CONVERTER, DIRSTR_RES, name )

  str_wbtbl = ""
  str_wbtbl += "#{DIRSTR_WBTMP}/#{name}.tbl "
  str_wbtbl += "#{DIRSTR_WBTMP}/#{name}_ex.tbl "

  return str_wbtbl
end


#=======================================================================
# マップタイプ　ランダムマップ用一覧、出力ルール書き込み
#=======================================================================
def file_write_random( name,
  file_output_list, file_temp_list, file_depend_list, file_make_depend, binary_type )
  #land_output_list
	file_output_list.printf( "\"%s/%s.3dppack\"\n", DIR_OUTPUT, name )
  
  #land_temp_list
	file_temp_list.printf( "\t%s/%s.nsbmd", DIRSTR_TEMP, name )
  
  #land_depend_list
	file_depend_list.printf( "\t%s/%s.3dppack", DIRSTR_OUTPUT, name )
  
  #land_make_depend 3dppack
  file_make_depend.printf( "#%s\n", name )
   
  file_make_depend.printf(
    "%s/%s.3dppack: %s/%s.nsbmd %s/%s.bin %s/%s.3dmd\n",
    DIRSTR_OUTPUT, name,
    DIRSTR_TEMP, name,
    DIRSTR_RES, name,
    DIRSTR_RES, name )
  
  file_make_depend.printf( "\t@echo create 3dppack %s\n", name )
  
  file_make_depend.printf(
    "\t@%s %s/%s.nsbmd %s/%s.bin %s/%s.3dmd %s/%s.3dppack #{binary_type}\n\n",
    STR_BINLINKER,
    DIRSTR_TEMP, name,
    DIRSTR_RES, name,
    DIRSTR_RES, name,
    DIRSTR_OUTPUT, name )
  
  #land_make_depend nsbmd
  file_make_depend.printf( "%s/%s.nsbmd: %s/%s.imd\n",
    DIRSTR_TEMP, name,
    DIRSTR_RES, name )

  file_make_depend.printf( "\t@echo create_nsbmd %s\n", name )
  
  file_make_depend.printf( "\t@%s %s/%s.imd -emdl -o %s/%s.nsbmd\n\n",
    STR_G3DCVTR, DIRSTR_RES, name, DIRSTR_TEMP, name )
  
  #land_make_depend exist file check
  check = DIR_RES + "/" + name + ".imd"
  if( FileTest.exist?(check) != true )
    printf( "%s.imdをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_GRID_IMD, check )
  end
 
  check = DIR_RES + "/" + name + ".bin"
  if( FileTest.exist?(check) != true )
    printf( "%s.binをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_GRID_BIN, check )
  end
  
  check = DIR_RES + "/" + name + ".3dmd"
  if( FileTest.exist?(check) != true )
    printf( "%s.3dmdをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_GRID_3DMD, check )
  end

end

#=======================================================================
# マップタイプ　非グリッド用一覧、出力ルール書き込み
#=======================================================================
def file_write_nogrid( name,
  file_output_list, file_temp_list, file_depend_list, file_make_depend )
  #land_output_list
	file_output_list.printf( "\"%s/%s.ngpack\"\n", DIR_OUTPUT, name )
  
  #land_temp_list
	file_temp_list.printf( "\t%s/%s.nsbmd", DIRSTR_TEMP, name )
	#file_temp_list.printf( "\t%s/%s.bhc", DIRSTR_TEMP, name )
  
  #land_depend_list
	file_depend_list.printf( "\t%s/%s.ngpack", DIRSTR_OUTPUT, name )
  
  #land_make_depend ngpack
  file_make_depend.printf( "#%s\n", name )
  
  file_make_depend.printf(
    #"%s/%s.ngpack: %s/%s.nsbmd %s/%s.bhc %s/%s.3dmd\n",
    "%s/%s.ngpack: %s/%s.nsbmd %s/%s.3dmd\n",
    DIRSTR_OUTPUT, name,
    DIRSTR_TEMP, name,
    #DIRSTR_TEMP, name,
    DIRSTR_RES, name )
  
  file_make_depend.printf( "\t@echo create ngpack %s\n", name )
  
  file_make_depend.printf(
    #"\t@%s %s/%s.nsbmd %s/%s.bhc %s/%s.3dmd %s/%s.ngpack BR\n\n",
    "\t@%s %s/%s.nsbmd %s/%s.3dmd %s/%s.ngpack NG\n\n",
    STR_BINLINKER,
    DIRSTR_TEMP, name,
    #DIRSTR_TEMP, name,
    DIRSTR_RES, name,
    DIRSTR_OUTPUT, name )
  
  #land_make_depend nsbmd
  file_make_depend.printf( "%s/%s.nsbmd: %s/%s.imd\n",
    DIRSTR_TEMP, name,
    DIRSTR_RES, name )
  
  file_make_depend.printf( "\t@echo create_nsbmd %s\n", name )
   
  file_make_depend.printf( "\t@%s %s/%s.imd -emdl -o %s/%s.nsbmd\n",
    STR_G3DCVTR, DIRSTR_RES, name, DIRSTR_TEMP, name )

  file_make_depend.printf( "\t@%s %s/%s.nsbmd\n\n",
    STR_CHECK_SIZE, DIRSTR_TEMP, name )

=begin
  #land_make_depend bhc
  file_make_depend.printf( "%s/%s.bhc: %s/%sh.imd\n",
    DIRSTR_TEMP, name, DIRSTR_RES, name )
  
  file_make_depend.printf( "\t@echo create_bhc %s\n", name )
  file_make_depend.printf(
    "\t@echo ./%s/%sh > %s\n", DIRSTR_RES, name, FNAME_HEIGHT_LIST )
  file_make_depend.printf( "\t@echo \\#END >> %s\n", FNAME_HEIGHT_LIST )
  
  file_make_depend.printf(
   "\t@%s %s < %s > %s\n", STR_HEIGHTCNV, FNAME_HEIGHT_LIST,
     STR_HEIGHTRETFILE, FNAME_HEIGHT_RESULT )
  
  file_make_depend.printf(
    "\t@ruby %s %s/%sh.bhc %s/%s.bhc\n",
    RUBYNAME_PAD4BYTE, DIRSTR_RES, name, DIRSTR_TEMP, name )
  file_make_depend.printf( "\t@rm %s/%sh.bhc\n\n", DIRSTR_RES, name )
=end
  
  #land_make_depend exist file check
  check = DIR_RES + "/" + name + ".imd"
  if( FileTest.exist?(check) != true )
    printf( "%s.imdをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_NOGRID_IMD, check )
  end
  
=begin
  check = DIR_RES + "/" + name + "h.imd"
  if( FileTest.exist?(check) != true )
    printf( "%sh.imdをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_NOGRID_H_IMD, check )
  end
=end
  
  check = DIR_RES + "/" + name + ".3dmd"
  if( FileTest.exist?(check) != true )
    printf( "%s.3dmdをダミーファイルから生成します\n", name )
    file_copy( DIR_RES+"/"+FNAME_DMYFILE_NOGRID_3DMD, check )
  end
end

#=======================================================================
# 一覧作成
#=======================================================================
file_output_list = File.open( FNAME_OUTPUT_LIST, "w" )
file_temp_list = File.open( FNAME_TEMP_LIST, "w" )
file_depend_list = File.open( FNAME_DEPEND_LIST, "w" )
file_make_depend = File.open( FNAME_MAKE_DEPEND, "w" )
file_wbbin_tbl_depend = File.open( FNAME_DEPEND_WBBIN_TBL_LIST, "w" )

file_temp_list.printf( "%s = \\\n", LNAME_TEMP_LIST )
file_depend_list.printf( "%s = \\\n", LNAME_DEPEND_LIST )



#alltblに必要なtblを格納
wbbin_alltbl_need_file = ""

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
			file_temp_list.printf( " \\\n" )
			file_depend_list.printf( " \\\n" )
		end
    
		yen_flg = 1
		column = line.split ","
		name = column[1]
    type = column[2]
    
    if( type == MAPTYPESTR_GRID )
      wbbin_alltbl_need_file += file_write_grid( name,
        file_output_list, file_temp_list,
        file_depend_list, file_make_depend, file_wbbin_tbl_depend, "WB" )
    elsif( type == MAPTYPESTR_CROSS )
      wbbin_alltbl_need_file += file_write_cross( name,
        file_output_list, file_temp_list,
        file_depend_list, file_make_depend, file_wbbin_tbl_depend )
    elsif( type == MAPTYPESTR_NOGRID )
      file_write_nogrid( name,
        file_output_list, file_temp_list,
        file_depend_list, file_make_depend )
    elsif( type == MAPTYPESTR_RANDOM )
      file_write_random( name,
        file_output_list, file_temp_list,
        file_depend_list, file_make_depend, "RD" )
    else
      printf( "%s:land_list.rb ERROR UNKNOWN MAPTYPE(%s)\n", file_name, type )
      printf( "%s\n", line )
      error_end( file_output_list, FNAME_OUTPUT_LIST,
          file_temp_list, FNAME_TEMP_LIST,
          file_depend_list, FNAME_DEPEND_LIST,
          file_make_depend, FNAME_MAKE_DEPEND,
          file_wbbin_tbl_depend, FNAME_DEPEND_WBBIN_TBL_LIST )
      exit 1
    end
	end
	file_csv.close


end

#最後にすべてをまとめたalltblを作る
file_make_depend.printf( "\t@echo %s\n", STR_WBBIN_ALLTBL )
file_make_depend.printf( "%s:%s\n", STR_WBBIN_ALLTBL, wbbin_alltbl_need_file )
file_make_depend.printf( "\t@%s CDAT %s\n", STR_WBBIN_CONVERTER, STR_WBBIN_CDAT )

file_make_depend.printf( "\t@echo %s\n", STR_WBBIN_TBLCHNAGE_LOG )
file_make_depend.printf( "%s:%s\n", STR_WBBIN_TBLCHNAGE_LOG, STR_WBBIN_ALLTBL )
file_make_depend.printf( "\t@%s %s ./\n", STR_DIFFCOPY_CONVERTER, STR_WBBIN_ALLTBL )


file_wbbin_tbl_depend.close
file_make_depend.close
file_depend_list.close
file_temp_list.close
file_output_list.close
