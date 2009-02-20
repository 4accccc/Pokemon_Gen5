#=======================================================================
# fmmdl_restbl.rb
# フィールド動作モデル リソーステーブル作成
# 
# 引数 fmmdl_restbl.rb xlstxt_filename restbl_filename res_dir dmyfile
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
# dmyfile　※任意
# ダミーファイル　dmyfileが指定されていて
# リソースファイルが無い場合、dmyfileをリソースファイルに置き換えます。
#=======================================================================
$KCODE = "SJIS"

#=======================================================================
#	定数
#=======================================================================
RESFILE_NAME_NO = (8) #リストファイル　リソースファイル名記述箇所

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

dmyfile = nil

if( ARGV[3] != nil )
	dmyfile = sprintf( "%s\/%s", resdir_path, ARGV[3] )
	if( FileTest.exist?(dmyfile) != true )
		printf( "ERROR!! fmmdl_restbl.rb dmyfile\n" )
		exit 1
	end
end

xlstxt_file = File.open( xlstxt_filename, "r" )
restbl_file = File.open( restbl_filename, "w" )

line = xlstxt_file.gets #一行目飛ばし

if( line == nil )
	printf( "ERROR!! fmmdl_restbl.rb %sが異常です\n", xlstxt_filename )
	error_end( restbl_filename, xlstxt_file, restbl_file )
	exit 1
end

flag = 0
restbl_file.printf( "FMMDL_RESLIST =" )

loop{
	line = xlstxt_file.gets
	str = line.split( "," )
	
	if( str[0] == "#END" )
		break
	end
	
	restbl_file.printf( " \\\n" )
	
	if( str[RESFILE_NAME_NO] != "" && str[RESFILE_NAME_NO] != nil )
		path = sprintf( "%s\/%s", resdir_path, str[RESFILE_NAME_NO] )
		
		if( FileTest.exist?(path) != true )
			if( dmyfile != nil )
				file_copy( dmyfile, path )
				printf( "%sをダミーファイルから作成しました\n", path )
			else
				printf( "ERROR!! %s がありません\n", str[RESFILE_NAME_NO] )
				error_end( restbl_filename, xlstxt_file, restbl_file )
				exit 1
			end
		end
		
		restbl_file.printf( "\t%s", str[RESFILE_NAME_NO] )
		flag = 1
	end
}

if( flag == 0 )
	printf( "fmmdl_restbl.rb リソース対象がありません\n" )
	error_end( restbl_filename, xlstxt_file, restbl_file )
	exit 1
end

xlstxt_file.close
restbl_file.close
