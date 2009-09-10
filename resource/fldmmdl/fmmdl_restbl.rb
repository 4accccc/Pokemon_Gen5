#=======================================================================
# fmmdl_restbl.rb
# フィールド動作モデル リソーステーブル作成
# 
# 引数 fmmdl_restbl.rb
# xlstxt_filename restbl_filename res_dir select_res 
# xlstxt_ncgimd_filename restbl_ncgimd_filename mkobj_ncgimd_filename
# res_ncg_dir ncgimd_conv_path
# dmyfile
# 
# xlstxt_filename
# 動作モデルリスト表 テキスト変換済みファイル名
# 
# restbl_filename
# 動作モデルリソースファイル名記述先ファイル名
#
# res_dir
# リソースファイルが置かれているディレクトリ
# 
# select_res
# xlstxt_filenameで指定されている複数のリソースファイルの内、どちらを使うか
# 0=製品反映モデルファイルを使用 1=テスト用を使用
#
# xlstxt_ncgimd_filename
# 動作モデルリスト表 ncg変換対応imd テキスト変換済みファイル名
#
# restbl_ncgimd_filename
# 動作モデル ncg変換imd一覧記述先ファイル名
#
# mkobj_ncgimd_filename
# 動作モデル ncg変換対象make記述先ファイル名
#
# res_ncg_dir
# ncgファイルが置かれているディレクトリ
#
# dmyfile　※任意
# ダミーファイル　dmyfileが指定されていて
# リソースファイルが無い場合、dmyfileをリソースファイルに置き換えます。
#=======================================================================
$KCODE = "SJIS"
load "rbdefine"

#=======================================================================
#	定数
#=======================================================================
STR_ERROR = "ERROR"
STR_END = "#END"

RET_FALSE = (0)
RET_TRUE = (1)
RET_ERROR = (-1)

#=======================================================================
#	異常終了
#=======================================================================
def error_end( path_delfile, file0, file1, file2, file3 )
	file0.close
	file1.close
	file2.close
	file3.close
	File.delete( path_delfile )
end

#=======================================================================
#	ファイルコピー
#=======================================================================
def file_copy( srcpath, copypath )
	open( srcpath ){ |input|
		open( copypath, "w" ){ |output|
			output.write( input.read )
		}
	}
end

#=======================================================================
#	指定番号の","区切り文字列を取得
#	戻り値 nil=無し,STR_ERROR=エラー,STR_END=終了
#=======================================================================
def xlstxt_getlinestr( xlstxt_file, get_no )
	no = 0
	xlstxt_file.pos = 0
	
	line = xlstxt_file.gets #一行目飛ばし
  
	if( line == nil )
		return RET_FALSE
	end
	
	while no < get_no
    line = xlstxt_file.gets

    if( line == nil )
      return STR_ERROR
    end
  
    str = line.split( "," )
  
	  if( str[0] == STR_END )
      return STR_END
    end
    
    no = no + 1
  end
  
  line = xlstxt_file.gets
  line = line.strip

  if( line == nil )
    return STR_ERROR
  end
  
  str = line.split( "," )
  
	if( str[0] == STR_END )
    return STR_END
  end
	
	return str
end

#=======================================================================
#	指定番号のリソースファイル名を取得
#	戻り値 nil=無し,STR_ERROR=エラー,STR_END=終了
#=======================================================================
def xlstxt_get_resfile_name( xlstxt_file, no, sel_res )
  xlsline = RBDEF_NUM_RESFILE_NAME_0

  if( sel_res != "0" )
    xlsline = RBDEF_NUM_RESFILE_NAME_1
  end
  
  str = xlstxt_getlinestr( xlstxt_file, no )
	
	if( str == STR_ERROR )
		return STR_ERROR
	end
	
  if( str == STR_END )
    return STR_END
  end
  
	if( str[xlsline] != "" && str[xlsline] != nil )
		return str[xlsline]
  end
	
	return nil
end

#=======================================================================
#	指定番号の表示タイプを取得
#	戻り値 nil=無し,STR_ERROR=エラー,STR_END=終了
#=======================================================================
def xlstxt_get_drawtype( xlstxt_file, no )
  str = xlstxt_getlinestr( xlstxt_file, no )
  
  if( str == STR_ERROR )
		return STR_ERROR
	end
	
  if( str == STR_END )
    return STR_END
  end

  type = str[RBDEF_NUM_DRAWTYPE]
			
  if( type != "" && type != nil )
		return type
	end
	
	return nil
end

#=======================================================================
#	指定番号より前リソースファイル名を検索し重複チェック
#	戻り値 RET_TRUE=指定ファイル名が存在している RET_FALSE=無し
#	戻り値 nil=無し,STR_ERROR=エラー,STR_END=終了
#=======================================================================
def xlstxt_check_resfile_name( xlstxt_file, check_no, check_str, sel_res )
	no = 0
	xlstxt_file.pos = 0
	
  xlsline = RBDEF_NUM_RESFILE_NAME_0
  
  if( sel_res != "0" )
    xlsline = RBDEF_NUM_RESFILE_NAME_1
  end
  
	while no < check_no
    str = xlstxt_getlinestr( xlstxt_file, no )
		
		if( str == STR_ERROR )
      break
	  end
	
    if( str == STR_END )
      break
    end
    
		if( no < check_no )	#指定位置より前
			if( str[xlsline] != "" && str[xlsline] != nil )
				if( str[xlsline] == check_str )
					return RET_TRUE
				end
			end
		end
		
		no = no + 1
	end
	
	return RET_FALSE
end

#=======================================================================
# 指定されたモデルサイズ、アニメーションIDから
# NCG変換時に必要なパラメタをハッシュで返す
# listfile ncg対応表txtファイル
# mdlstr モデルサイズ文字列 MDLSIZE_32x32等
# anmstr アニメID文字列 ANMID_NON等
# 戻り値 nil=なし
#=======================================================================
def ncgparam_get( listfile, mdlstr, anmstr )
  listfile.pos = 0
  listfile.gets #一行抜かし
  listfile.gets #一行抜かし
  
  while line = listfile.gets
    line = line.strip
    str = line.split( "," )
    
    if( str[0] == STR_END )
      break
    end
    
    if( str[0] != nil && str[0] == anmstr )
      i = 1
      while data = str[i]
        param = data.split( "/" )
        
        if( param[0] != nil && param[0] == mdlstr )
          h = Hash.new
          h.store( "str_width", param[1] )
          h.store( "str_height", param[2] )
          h.store( "str_count", param[3] )
          h.store( "str_imdfile", param[4] )
          h.store( "str_dmyfile", param[5] )
          return h
        end
        
        i = i + 1
      end
    end
  end
  
  return nil
end

#=======================================================================
# DRAWTYPE_BLACT処理
# 戻り値 RET_TRUE=リソースを追加した。
#=======================================================================
def conv_drawtype_blact(
  xlstxt_file, restbl_file, no, resdir_path, sel_res, dmyfile, 
  xlstxt_ncgimd_file, restbl_ncgimd_file,
  mkobj_ncgimd_file, resdir_ncgimd_path, ncgimd_conv_path )
	
  imdname = xlstxt_get_resfile_name( xlstxt_file, no, sel_res )
  
	if( imdname == STR_ERROR )
    return RET_ERROR
  end
  
  if( imdname == nil )
    return RET_FALSE
  end
  
  if( xlstxt_check_resfile_name(xlstxt_file,no,imdname,sel_res) == RET_TRUE )
    return RET_FALSE
  end
  
	restbl_file.printf( " \\\n" )
  
  str = xlstxt_getlinestr( xlstxt_file, no )
  mdltype = str[RBDEF_NUM_MDLSIZE]
  anmtype = str[RBDEF_NUM_ANMID]
  param = ncgparam_get( xlstxt_ncgimd_file, mdltype, anmtype )

  imdpath = sprintf( "%s\/%s", resdir_path, imdname )
  
  if( param == nil ) #ncg変換対象ではない
		if( FileTest.exist?(imdpath) != true )
		  if( dmyfile == nil )
				printf( "ERROR %s がありません\n", imdname )
        return RET_ERROR
      end
      
		  file_copy( dmyfile, imdpath )
		  printf( "%sをダミーファイルから作成しました\n", imdpath )
    end
    
		restbl_file.printf( "\t%s", str )
    return RET_TRUE
  end
  
  #ncg変換対象
  restbl_ncgimd_file.printf( " \\\n" )
  
  name = imdname.gsub( "\.imd", "" )
  ncgname = name + "\.ncg"
  nclname = name + "\.ncl"
  
  ncgpath = sprintf( "%s\/%s", resdir_ncgimd_path, ncgname )
  
	if( FileTest.exist?(ncgpath) != true )
    if( dmyfile == nil )
		  printf( "ERROR %s がありません\n", ncgpath )
      return RET_ERROR
    end
    
    dmyfile = sprintf( "%s/%s.ncg", resdir_ncgimd_path, param['str_dmyfile'] )
    file_copy( dmyfile, ncgpath )
		printf( "%sをダミーファイルから作成しました\n", ncgpath )
  end
	
  nclpath = sprintf( "%s\/%s", resdir_ncgimd_path, nclname )

  if( FileTest.exist?(nclpath) != true )
    if( dmyfile == nil )
		  printf( "ERROR %s がありません\n", nclpath )
      return RET_ERROR
    end
    
    dmyfile = sprintf( "%s/%s.ncl", resdir_ncgimd_path, param['str_dmyfile'] )
    file_copy( dmyfile, nclpath )
		printf( "%sをダミーファイルから作成しました\n", nclpath )
  end
  
  restbl_file.printf( "\t%s", imdname )
  restbl_ncgimd_file.printf( "\t%s", imdname )
  
  #make object
  dmyimdpath = sprintf( "%s\/%s", resdir_path, param['str_imdfile'] )
  
  mkobj_ncgimd_file.printf( "#%s\n", imdname )
  mkobj_ncgimd_file.printf(
    "%s: %s %s\n\t@echo convert ncg_imd %s\n\t@ruby %s %s %s %s %s %s %s\n\n",
    imdpath, ncgpath, nclpath, name,
    ncgimd_conv_path, ncgpath, resdir_path, dmyimdpath, 
    param['str_width'], param['str_height'], param['str_count'] )
  
  return RET_TRUE
end

#=======================================================================
#	リソーステーブル作成
#=======================================================================
xlstxt_filename = ARGV[0]

if( xlstxt_filename == nil )
	printf( "ERROR!! fmmdl_restbl.rb restbl_filename\n" )
	exit 1
end

restbl_filename = ARGV[1]

if( restbl_filename == nil )
	printf( "ERROR!! fmmdl_restbl.rb restbl_filename\n" )
	exit 1
end

resdir_path = ARGV[2] 

if( resdir_path == nil )
	printf( "ERROR!! fmmdl_restbl.rb resdir\n" )
	exit 1
end

if( FileTest.directory?(resdir_path) != true )
	printf( "ERROR!! fmmdl_restbl.rb resdir\n" )
	exit 1
end

sel_res = ARGV[3]

if( sel_res != "0" && sel_res != "1" )
  printf( "ERROR!! fmmdl_restbl.rb sel_res\n" )
  exit 1
end

xlstxt_ncgimd_filename = ARGV[4]

if( xlstxt_ncgimd_filename == nil )
	printf( "ERROR!! fmmdl_restbl.rb xlstxt_ncgimd_filename\n" )
	exit 1
end

restbl_ncgimd_filename = ARGV[5]

if( restbl_ncgimd_filename == nil )
	printf( "ERROR!! fmmdl_restbl.rb restbl_ncgimd_filename\n" )
	exit 1
end

mkobj_ncgimd_filename = ARGV[6]

if( mkobj_ncgimd_filename == nil )
	printf( "ERROR!! fmmdl_restbl.rb mkobj_ncgimd_filename\n" )
	exit 1
end

resdir_ncgimd_path = ARGV[7]

if( resdir_ncgimd_path == nil )
	printf( "ERROR!! fmmdl_restbl.rb resdir_ncgimd_path\n" )
	exit 1
end

ncgimd_conv_path = ARGV[8]

if( ncgimd_conv_path == nil )
	printf( "ERROR!! fmmdl_restbl.rb ncgimd_conv_path\n" )
	exit 1
end


dmyfile = nil

if( ARGV[9] != nil )
	dmyfile = sprintf( "%s\/%s", resdir_path, ARGV[9] )
	if( FileTest.exist?(dmyfile) != true )
		printf( "ERROR!! fmmdl_restbl.rb dmyfile\n" )
		exit 1
	end
end

xlstxt_file = File.open( xlstxt_filename, "r" )
restbl_file = File.open( restbl_filename, "w" )
xlstxt_ncgimd_file = File.open( xlstxt_ncgimd_filename, "r" )
restbl_ncgimd_file = File.open( restbl_ncgimd_filename, "w" )
mkobj_ncgimd_file = File.open( mkobj_ncgimd_filename, "w" )

no = 0
flag = 0
ret = RET_FALSE

restbl_file.printf( "FMMDL_RESLIST =" )
restbl_ncgimd_file.printf( "FMMDL_RESLIST_NCGIMD =" )

loop{
  type = xlstxt_get_drawtype( xlstxt_file, no )
  
  case type
  when "DRAWTYPE_NON"
    #現状無し
  when "DRAWTYPE_BLACT"
    ret = conv_drawtype_blact(
      xlstxt_file, restbl_file, no, resdir_path, sel_res, dmyfile,
      xlstxt_ncgimd_file, restbl_ncgimd_file,
      mkobj_ncgimd_file, resdir_ncgimd_path, ncgimd_conv_path )
  when "DRAWTYPE_MDL"
    #現状無し
  when STR_END
    break
  else
    ret = RET_ERROR
  end
  
  if( ret == RET_TRUE ) #リソース存在
    flag = 1
  elsif( ret == RET_ERROR ) #エラー
		printf( "ERROR: fmmdl_restbl.rb %s error text\n", xlstxt_filename )
		error_end( restbl_filename, xlstxt_file, restbl_file,
              xlstxt_ncgimd_file, restbl_ncgimd_file )
		exit 1
  end
  
	no = no + 1
}

if( flag == 0 ) #リソースが一つも存在していない
	printf( "fmmdl_restbl.rb リソース対象がありません\n" )
  error_end( restbl_filename, xlstxt_file, restbl_file,
            xlstxt_ncgimd_file, restbl_ncgimd_file )
	exit 1
end

xlstxt_file.close
restbl_file.close
xlstxt_ncgimd_file.close
restbl_ncgimd_file.close
mkobj_ncgimd_file.close

=begin
	str = xlstxt_get_resfile_name( xlstxt_file, no, sel_res )
	
	if( str == STR_ERROR )
		printf( "ERROR!! fmmdl_restbl.rb %sが異常です\n", xlstxt_filename )
		error_end( restbl_filename,
       xlstxt_file, restbl_file, xlstxt_ncgimd_file, restbl_ncgimd_file )
		exit 1
	end
	
	if( str == STR_END )
		break
	end
	
	if( str != nil )
		if( xlstxt_check_resfile_name(xlstxt_file,no,str,sel_res) == RET_FALSE )
			restbl_file.printf( " \\\n" )
			path = sprintf( "%s\/%s", resdir_path, str )
		  
			if( FileTest.exist?(path) != true )
				if( dmyfile != nil )
					file_copy( dmyfile, path )
					printf( "%sをダミーファイルから作成しました\n", path )
				else
					printf( "ERROR!! %s がありません\n", str )
		      error_end( restbl_filename, xlstxt_file, restbl_file,
              xlstxt_ncgimd_file, restbl_ncgimd_file )
					exit 1
				end
			end
			
			restbl_file.printf( "\t%s", str )
			flag = 1
		end
	end
	

if( flag == 0 )
	printf( "fmmdl_restbl.rb リソース対象がありません\n" )
  error_end( restbl_filename, xlstxt_file, restbl_file,
      xlstxt_ncgimd_file, restbl_ncgimd_file )
	exit 1
end
=end
