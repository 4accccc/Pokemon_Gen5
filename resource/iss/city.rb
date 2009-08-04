# データ・インデックス
INDEX_ZONE = 0
INDEX_POS_X = 1
INDEX_POS_Y = 2
INDEX_POS_Z = 3
INDEX_VOLUME_1 = 4
INDEX_VOLUME_2 = 5
INDEX_VOLUME_3 = 6
INDEX_VOLUME_4 = 7
INDEX_RANGE_X_1 = 8
INDEX_RANGE_X_2 = 9
INDEX_RANGE_X_3 = 10
INDEX_RANGE_X_4 = 11
INDEX_RANGE_Y_1 = 12
INDEX_RANGE_Y_2 = 13
INDEX_RANGE_Y_3 = 14
INDEX_RANGE_Y_4 = 15
INDEX_RANGE_Z_1 = 16
INDEX_RANGE_Z_2 = 17
INDEX_RANGE_Z_3 = 18
INDEX_RANGE_Z_4 = 19

# テーブル1要素分のデータサイズ
ONE_TABLE_DATA_SIZE = 4

# 1ユニット分のデータサイズ
ONE_UNIT_DATA_SIZE = 24


#---------------------------------------------------------------------------------- 
# ゾーン名から, ゾーンIDを取得する
def GetZoneID( name )

	zone_id = 0;

	# 検索する定数名を作成
	def_name = "ZONE_ID_" + name

	# ファイルを開く
	file_def = File.open( "../../prog/arc/fieldmap/zone_id.h" )

	# 1行ずつ見ていく
	file_def.each do |line|

		# 検索定数名が見つかった場合
		if( line.index( def_name ) != nil ) then

			# ()内の文字列を数値に変換する
			i0 = line.index( "(" ) + 1
			i1 = line.index( ")" ) - 1
			str_num = line[ i0..i1 ]
			zone_id = str_num.to_i 
			break
			
		end
		
	end
	
	# ファイルを閉じる
	file_def.close

	return zone_id
end

#----------------------------------------------------------------------------------
# ゾーンIDの最大値を取得する
def GetMaxZoneID()

	max_id = 0;

	# ファイルを開く
	file_def = File.open( "../../prog/arc/fieldmap/zone_id.h" )

	# 1行ずつ見ていく
	file_def.each do |line|

		# 検索定数名が見つかった場合
		if( line.index( "ZONE_ID_MAX" ) != nil ) then

			# ()内の文字列を数値に変換する
			i0 = line.index( "(" ) + 1
			i1 = line.index( ")" ) - 1
			str_num = line[ i0..i1 ]
			max_id = str_num.to_i 
			break
			
		end
		
	end
	
	# ファイルを閉じる
	file_def.close

	return max_id
end

#----------------------------------------------------------------------------------
# データ・テーブルを出力する
def WriteDataTable( src_filename, dst_filename, open_mode )

	# ファイルを開く
	file  = File.open( src_filename )
	wfile = File.open( dst_filename, open_mode )

	# データ数(テーブルサイズ)をカウント
	data_num = 0;
	file.each do |line|
		line = line.gsub( /\s/, "" )
		if( line != "" ) then
			data_num += 1;
		end
	end
	data_num -= 1;	# 1行目は無視するので-1
	array = [ data_num ]
	wfile.write( array.pack( "C" ) )


	# データテーブル作成
	line_index = 0
	data_index = 0			# 出力したデータの数
	file.pos   = 0
	file.each do |line|

		# 空データが来たら, 終了
		empty_test = line.gsub( /\s/, "" )
		if( empty_test == "" ) then
			break;
		end

		# 1行目は無視する
		if( line_index != 0 ) then

			# タブで区切る
			line_split = line.split( /\t/ ) 

			# ゾーンIDを取得
			zone_id = GetZoneID( line_split[ INDEX_ZONE ] )

			# データへのオフセットを計算
			offset  = 1 + ONE_TABLE_DATA_SIZE * data_num   # オフセット = データ先頭までのオフセット + 
			offset += ONE_UNIT_DATA_SIZE * data_index	   #              データ先頭からのオフセット

			# 出力したデータの数をカウント
			data_index += 1 

			# テーブル・データを出力
			array = [ zone_id, offset ]
			wfile.write( array.pack( "SS" ) )

		end

		line_index += 1

	end
	
	# ファイルを閉じる
	file.close
	wfile.close
end

#----------------------------------------------------------------------------------

src_filename = "city.txt"
dst_filename = "city.bin"


# ヘッダ(データ・テーブル)を作成
WriteDataTable( src_filename, dst_filename, "wb" )

# ファイルを開く
file  = File.open( src_filename )
wfile = File.open( dst_filename, "ab" );	# 追記する

# 行インデックス
line_index = 0;

# すべての行について処理する
file.each do |line|

	# 空データが来たら, 終了
	empty_test = line.gsub( /\s/, "" )
	if( empty_test == "" ) then
		break;
	end

	# 1行目は無視する
	if( line_index != 0 ) then

		# タブで区切る
		line_split = line.split( /\t/ )
		
		# 区切った各データを数値配列に変換
		array      = Array.new 
		data_index = 0
		line_split.each do |s|

			# 街指定はゾーンIDに変換する
			if( data_index == INDEX_ZONE ) then
				array << GetZoneID( s )

			# それ以外は そのまま数値に変換
			else 
				array << s.to_i
			end

			# データ番号をカウント
			data_index += 1

		end

		# ファイルに出力
		line_pack  = array.pack( "SSSSCCCCCCCCCCCCCCCC" );
		wfile.write( line_pack )

	end

	# 行数をカウント
	line_index += 1
end

# ファイルを閉じる
wfile.close
file.close
