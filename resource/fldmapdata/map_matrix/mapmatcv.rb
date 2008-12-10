###############################################################
#	mapmatcv.rb
#	マップマトリクスコンバート
#
#	呼び出し mapmapcv.rb BlockPath FilePath LandPath BinPath
#	FilePath マップマトリクスファイルパス
#	BlockPath 地形ブロック番号ファイルパス
#	LandPath 地形モデルアーカイブインデックスファイルパス
#	BinPath  バイナリファイル作成先パス
#
###############################################################
$KCODE = "SJIS"

#==============================================================
#	※マトリクスフォーマット バイト単位
#	0-1 マップ存在
#	2-3 マップ種類
#	4-5	Hブロックサイズ
#	6-7 Vブロックサイズ
#	8-ブロックサイズ(HxV)x4byte分の配置配列
#
#	※地形データテキストフォーマット
#	format: tabspace
#	title
#    0 1 2 3	#dial X
#    1 9 0 0
#    2 8 0 2
#    3 1 0 6
#==============================================================

#==============================================================
#	定数
#==============================================================
#ファイル書き込み位置
WPOS_FLAG	= (0)
WPOS_KIND	= (2)
WPOS_SIZE	= (4)
WPOS_TABLE	= (8)

#配列なし
IDX_NON		= (0xffffffff)

#関数エラー
RET_ERROR	= (-1)
RET_TRUE	= (1)
RET_FALSE	= (0)

#固定文字列
STR_NULL	= ""
STR_END		= "#END"

#==============================================================
#	表データ
#==============================================================
#--------------------------------------------------------------
#	表データの指定位置の数値を取得
#	listfile	リストファイル
#	x	X位置
#	y	Y位置
#--------------------------------------------------------------
def table_posdata_get( listfile, x, y )
	listfile.pos = 0			#先頭に
	
	listfile.gets				#タイトル飛ばし
	listfile.gets				#X目盛り飛ばし
	
	while y > 0					#Y合わせ
		line = listfile.gets
		
		if( line == @nil || line.include?(STR_END) )
			return RET_ERROR
		end
		
		y -= 1
	end
	
	line = listfile.gets		#目当ての行

	if( line == @nil || line.include?(STR_END) )
		return RET_ERROR
	end
	
	str = split( "\t" )			#タブ単位
	x += 1						#Y目盛り飛ばし

	if( str[x] == @nil || str[x] == STR_END )
		return RET_ERROR
	end
	
	num = str[x].to_i
	return num
end

#==============================================================
#	地形ブロックnaix
#==============================================================
#--------------------------------------------------------------
#	地形ブロックnaixファイルから指定インデックスの数値を取得
#--------------------------------------------------------------
def landnaix_number_get( naix_file, str )
	naix_file.pos = 0			#先頭に
	
	loop{
		line = naix_file.gets
		
		if( line == @nil || line.include?("}") )
			return RET_ERROR
		end

		if( line.include?("enum") )
			break
		end
	}
	
	num = 0

	loop{
		line = naix_file.gets
		
		if( line == @nil || line.include?("}") )
			return RET_ERROR
		end
		
		if( line.include?(str) )
			return num
		end
		
		num += 1
	}
	
	return RET_ERROR
end

#==============================================================
#	地形ブロック番号ファイル
#==============================================================
#--------------------------------------------------------------
#	指定ブロック番号から地形データインデックス番号を取得
#--------------------------------------------------------------
def blocknum_landidx_get( idx, blocknum_file, naix_file )
	naix_file.pos = 0			#先頭に
	blocknum_file.pos = 0
	line = blocknum_file.gets	#一行飛ばし
	
	loop{
		line = blocknum_file.gets
		
		if( line == @nil || line.include?(STR_END) )
			return RET_ERROR
		end
		
		str = split( "\t" )		#タブ単位
		num = str[0].to_i		#番号
		
		if( idx == num )		#一致
			ret = landnaix_number_get( naix_file, str[1] )
			
			if( ret == RET_ERROR )
				printf( "mapmatcv ERROR " )
				printf( "%sのブロックが見つかりません\n", str[1] )
			end
			
			return ret
		end
	}
end

#==============================================================
#	マトリクスファイル
#==============================================================
#--------------------------------------------------------------
#	マトリクス文字列からマトリックスID取得
#--------------------------------------------------------------
def matstr_matid_get( str )
	return str[0]
end

#--------------------------------------------------------------
#	マトリクス文字列からXサイズを取得
#--------------------------------------------------------------
def matstr_x_size_get( str )
	num = str[1].to_i
	return num
end

#--------------------------------------------------------------
#	マトリクス文字列からYサイズを取得
#--------------------------------------------------------------
def matstr_y_size_get( str )
	num = str[2].to_i
	return num
end

#--------------------------------------------------------------
#	マトリクス文字列からプレフィックスを取得
#--------------------------------------------------------------
def matstr_prefix_get( str )
	return str[3]
end

#--------------------------------------------------------------
#	マトリクス文字列から地形ファイル名を取得
#--------------------------------------------------------------
def matstr_landfile_name_get( str )
	return str[4]
end

#--------------------------------------------------------------
#	マトリクス文字列からZONE_ID管理表名取得
#--------------------------------------------------------------
def matstr_zonefile_name_get( str )
	return str[5]
end

#--------------------------------------------------------------
#	プレフィックス名にx,yの値を足した文字列を返す
#--------------------------------------------------------------
def prefixname_get( prefix, x, y )
	name = sprintf( "%s%02d_%02d", prefix, x, y )
	return name
end

#--------------------------------------------------------------
#	マトリクスIDをバイナリファイル名に変換した文字列を返す
#--------------------------------------------------------------
def	matid_binname_get( id )
	name = sprintf( "%s_mat.bin", id )
	return name
end

#--------------------------------------------------------------
#	存在フラグ書き込み
#--------------------------------------------------------------
def filewrite_flag( wfile, flag )
	pos = wfile.pos
	wfile.pos = WPOS_FLAG
	ary = Array( flag )
	wfile.write( ary.pack("S*") )
	wfile.pos = pos
end

#--------------------------------------------------------------
#	マップ種類書き込み
#--------------------------------------------------------------
def filewrite_kind( wfile, kind )
	pos = wfile.pos
	wfile.pos = WPOS_KIND
	ary = Array( kind )
	wfile.write( ary.pack("S*") )
	wfile.pos = pos
end

#--------------------------------------------------------------
#	X,Y書き込み
#--------------------------------------------------------------
def filewrite_xy_size( wfile, x, y )
	pos = wfile.pos
	wfile.pos = WPOS_SIZE
	ary = Array( x )
	wfile.write( ary.pack("S*") )
	ary = Array( y )
	wfile.write( ary.pack("S*") )
	wfile.pos = pos
end

#--------------------------------------------------------------
#	一行コンバート
#	return RET_TRUE == 終端
#--------------------------------------------------------------
def mtxline_convert( line, cr_dir_path, naix_file, blocknum_file )
	if( line == @nil || line.include?(STR_END) )
		return RET_TRUE
	end
	
	landfile = @nil
	str = line.split( "\t" )
	str_id = matstr_matid_get( str )
	x_size = matstr_x_size_get( str )
	y_size = matstr_y_size_get( str )
	prefix = matstr_prefix_get( str )
	landname = matstr_landfile_name_get( str )
	zonename = matstr_zonefile_name_get( str )
	
	printf( "mapmatcv convert %s\n", str_id )
	
	str_wfile = matid_binname_get( str_id )
	str_wfile = sprintf( "%s/%s", cr_dir_path, str_wfile )
	wfile = File.open( str_wfile, "wb" )
	
	if( wfile == @nil )
		printf( "mapmatcv ERROR バイナリファイル作成失敗\n" )
		exit 1
	end
	
	if( landname != "X" )		#ゾーンファイル指定無し
		landname = sprintf( "%s\.txt", landname )
		landfile = File.open( landname )
	end
	
	filewrite_flag( wfile, 1 )
	filewrite_kind( wfile, 1 )
	filewrite_xy_size( wfile, x_size, y_size )
	
	wfile.pos = WPOS_TABLE		#書き込み位置をテーブルへ
	y = 0
	
	while y < y_size
		x = 0
		while x < x_size
			idx = 1
			
			if( landname != "X" )	#テーブル指定あり
				idx = table_posdata_get( landfile, x, y )
				
				if( idx == RET_ERROR )
					printf( "map_matconv ERROR 地形テーブル異常\n" )
					exit 1
				end
			end
			
			case idx
			when 0					#指定なし
				idx = IDX_NON
			when 1					#マップ存在有り
				binname = prefixname_get( prefix, x, y )
				idx = landnaix_number_get( naix_file, binname )
				
				if( idx == RET_ERROR )
					printf( "map_matconv ERROR 地形%sが無い\n", binname )
					exit 1
				end
			else					#ブロック番号指定
				idx = blocknum_landidx_get( idx, blocknum_file, naix_file )
				
				if( idx == RET_ERROR )
					printf( "map_matconv ERROR ブロック番号異常 X=%d,Y=%d\n",
						  x, y )
					exit 1
				end
			end

			ary = Array( idx )
			wfile.write( ary.pack("I*") )
			x += 1
		end
		y += 1
	end
	
	wfile.close
	
	if( landfile != @nil )
		landfile.close
	end

	return RET_FALSE
end

#==============================================================
#	コンバートメイン
#==============================================================
#--------------------------------------------------------------
#	マップマトリクスコンバート
#--------------------------------------------------------------
mtx_path = ARGV[0]		#マップマトリクスファイルパス
blocknum_path = ARGV[1]		#ブロック番号ファイルパス
landnaix_path = ARGV[2]		#地形モデルアーカイブインデックスファイルパス
dirbin_path = ARGV[3]		#バイナリファイル作成先パス

if( mtx_path == @nil || landnaix_path == @nil || dirbin_path == @nil )
	printf( "mat_matconv ERROR 引数異常\n" )
	exit 1
end

printf( "mat_matconv %s %s %s\n", mtx_path, landnaix_path, dirbin_path )

mtx_file = File.open( mtx_path, "r" )
if( mtx_file == @nil )
	printf( "mat_matconv ERROR %sが開けません\n", mtx_path )
	exit 1
end

blocknum_file = File.open( blocknum_path, "r" )
if( blocknum_file == @nil )
	printf( "mat_matconv ERROR %sが開けません\n", blocknum_path )
	exit 1
end

naix_file = File.open( landnaix_path, "r" )
if( naix_file == @nil )
	printf( "mat_matconv ERROR %sが開けません\n", landnaix_path )
	exit 1
end

mtx_file.gets				#一行飛ばし

loop{
	line = mtx_file.gets
	ret = mtxline_convert( line, dirbin_path, naix_file, blocknum_file )
	if( ret == RET_TRUE )
		break
	end
}

mtx_file.close
blocknum_file.close
naix_file.close
