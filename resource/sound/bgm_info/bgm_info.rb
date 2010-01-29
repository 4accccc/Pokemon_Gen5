###############################################################################
#
# @brief  [BGMのシーケンス番号 : ISSタイプ] のテーブルを作成する
# @author obata_tosihiro
# @date   2009.07.17
# 
# [出力バイナリ・データのフォーマット]
# [0] データ数
# [1] シーケンス番号
# [2]
# [3] ISSタイプ
# 以下、[1]-[3]がデータの数だけ存在
###############################################################################


###############################################################################
#
# @brief 指定された文字列が空かどうかを判定する
#
# @param str 判定対象の文字列
# 
###############################################################################
def IsEmpty( str )

	# 空白文字を削除
	str = str.gsub( /\s/, "" )

	# 何らかの文字が存在すれば, 空ではない
	if str != "" then
		return false;
	end

	# 空
	return true;
	
end


###############################################################################
#
# @brief 文字列に該当するシーケンス番号を取得する
#
# @param seq_name シーケンス番号の define 定義名
# @param file     シーケンス番号が定義されているファイル
# 
###############################################################################
def GetSeqIndex( seq_name, file )

	# ファイル先頭位置へシーク
	file.seek( 0, IO::SEEK_SET )

	# すべての行を検索する
	file.each do | line |
		# 発見
		if( line.index( seq_name ) != nil ) then
			items = line.split( "\t" )
			return items[1].to_i
		end
	end

	# 見つからなかった場合
	abort( "シーケンスラベル#{seq_name}は定義されていません。" )
end


###############################################################################
#
# @brief 文字列に該当するISSタイプ番号を取得する
#
# @param iss_type ISSタイプ( "通常", "街", "道路" など )
# @param file     ISSタイプが定義されているファイル
# 
###############################################################################
def GetISSType( iss_type, file )

	# ファイル先頭位置へシーク
	file.seek( 0, IO::SEEK_SET )

	# すべての行を検索する
	file.each do | line |
		# 発見
		if( line.index( iss_type ) != nil ) then

			# "(", ")" の間にある数値を取得
			i0 = line.index( "(" ) + 1
			i1 = line.index( ")" ) - 1
			str = line[ i0..i1 ]
			return str.to_i
		end
	end

  # 見つからず
  abort( "ISSタイプ#{iss_type}は定義されていません。" )
end


###############################################################################
#
# @brief メイン
# 
###############################################################################
# 出力データ
data_num = 0	     # データ数
table    = Array.new # テーブルデータ[シーケンス番号, ISSタイプ]

#------------------------------------------------------------
# テーブル作成
#------------------------------------------------------------

# 各ファイル名
SRC_FILENAME      = ARGV[0]
DEF_SEQ_FILENAME  = "../wb_sound_data.sadl"
DEF_TYPE_FILENAME = "iss_type.h"
DST_FILENAME      = "bgm_info.bin"

# ファイルを開く
src_file      = File.open( SRC_FILENAME )
def_seq_file  = File.open( DEF_SEQ_FILENAME )
def_type_file = File.open( DEF_TYPE_FILENAME )

# 1,2行目をスキップ
src_file.gets
src_file.gets

# 3行目以降をすべて見る
src_file.each do | line |

	# 項目別に分割
	items = line.split( "\t" )

	# ISSタイプを取得
	iss_type = items[5];

	# ISSタイプが設定されている場合, シーケンス番号とISSタイプをテーブルに登録
	if iss_type!=nil && ( IsEmpty( iss_type ) != true ) then
		table[ data_num ] = [ GetSeqIndex( items[2], def_seq_file ), GetISSType( iss_type, def_type_file ) ];
		data_num          = data_num + 1;
	end
	
end

# ファイルを閉じる
src_file.close
def_seq_file.close
def_type_file.close


#------------------------------------------------------------
# データを出力
#------------------------------------------------------------
# ファイルを開く
dst_file = File.open( DST_FILENAME, "wb" )

# データ数を出力
dst_file.write( [ data_num ].pack( "C" ) )

# テーブルを出力
table.each { |items|
	dst_file.write( items.pack( "SC" ) )
}

# ファイルを閉じる
dst_file.close


#=begin
#------------------------------------------------------------
# デバッグ出力
#------------------------------------------------------------
# ファイルを開く
dst_file = File.open( "debug.txt", "w" )

# データ数を出力
str = "data_num = " + data_num.to_s() + "\n"
dst_file.write( str )

# テーブルを出力
table.each { |items|
	str = "seq = " + items[0].to_s() + "  type = " + items[1].to_s() + "\n"
	dst_file.write( str )
}

# ファイルを閉じる
dst_file.close

#=end
