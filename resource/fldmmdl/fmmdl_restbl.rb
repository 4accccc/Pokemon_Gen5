#=======================================================================
# fmmdl_restbl.rb
# フィールド動作モデル リソーステーブル作成
# 
# 引数 fmmdl_restbl.rb
# xlstxt_filename restbl_filename res_dir select_res dmyfile
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

#=======================================================================
#	異常終了
#=======================================================================
def error_end( path_delfile, file0, file1 )
	file0.close
	file1.close
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
#	指定番号のリソースファイル名を取得
#	戻り値 nil=無し,STR_ERROR=エラー,STR_END=終了
#=======================================================================
def xlstxt_get_resfile_name( xlstxt_file, no, sel_res )
	xlstxt_file.pos = 0
  
  xlsline = RBDEF_NUM_RESFILE_NAME_0

  if( sel_res != "0" )
    xlsline = RBDEF_NUM_RESFILE_NAME_1
  end
  
	line = xlstxt_file.gets #一行目飛ばし
	
	if( line == nil )
		return STR_ERROR
	end
	
	while line = xlstxt_file.gets
		str = line.split( "," )
		
		if( str[0] == STR_END )
			return str[0]
		end
		
		if( no <= 0 )	#指定位置
			if( str[xlsline] != "" && str[xlsline] != nil )
				return str[xlsline]
			else
				break
			end
		end
		
		no = no - 1
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
  
	line = xlstxt_file.gets #一行目飛ばし

	if( line == nil )
		return RET_FALSE
	end
	
	while line = xlstxt_file.gets
		str = line.split( "," )
		
		if( str[0] == STR_END )
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

dmyfile = nil

if( ARGV[4] != nil )
	dmyfile = sprintf( "%s\/%s", resdir_path, ARGV[4] )
	if( FileTest.exist?(dmyfile) != true )
		printf( "ERROR!! fmmdl_restbl.rb dmyfile\n" )
		exit 1
	end
end

xlstxt_file = File.open( xlstxt_filename, "r" )
restbl_file = File.open( restbl_filename, "w" )

no = 0
flag = 0
restbl_file.printf( "FMMDL_RESLIST =" )

loop{
	str = xlstxt_get_resfile_name( xlstxt_file, no, sel_res )
	
	if( str == STR_ERROR )
		printf( "ERROR!! fmmdl_restbl.rb %sが異常です\n", xlstxt_filename )
		error_end( restbl_filename, xlstxt_file, restbl_file )
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
					error_end( restbl_filename, xlstxt_file, restbl_file )
					exit 1
				end
			end
			
			restbl_file.printf( "\t%s", str )
			flag = 1
		end
	end
	
	no = no + 1
}

if( flag == 0 )
	printf( "fmmdl_restbl.rb リソース対象がありません\n" )
	error_end( restbl_filename, xlstxt_file, restbl_file )
	exit 1
end

xlstxt_file.close
restbl_file.close
