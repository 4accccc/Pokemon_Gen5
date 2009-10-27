#===================================================================
#
# @brief  ショップデータコンバーター
#
# itemym.hとfs_item.csvを読み込んでショップデータを作成します。
#
# ex)
# ruby shopdata.rb fs_item.csv itemsym.h > fs_item.cdat
#
# @data   09.10.19
# @author mori
#
#===================================================================

# ITEMSYM.hの定義取得を開始する行
ITEMSYM_H_START_LINE = 10


# 文字コード指定
$KCODE = 'SJIS'

#CSV読み込み
require "csv"

#ワーク初期化
hash       = Hash.new	# どうぐリスト
namelist   = Array.new	# 配列名
maxlist    = Array.new	# 配列MAX名
line_count = 0



#---------------------------------------
# WBアイテム実定義ファイルオープン
#---------------------------------------
file = File.open(ARGV[1])


file.each{|line|
	# 10行目から読み込む
	if line_count>ITEMSYM_H_START_LINE then
		if line.to_s.slice(/\/\/\s\w*/)!=nil then
			hash[line.to_s.slice(/\/\/\s\w*/).sub(/\/\/\s/,"")] = line.to_s.slice(/ITEM_\w*/)
		end
	end
	line_count = line_count+1
}

file.close

#---------------------------------------
# ショップデータファイルオープン
#---------------------------------------
reader = CSV.open( ARGV[0], 'r')

# 3行読み飛ばし
reader.shift
reader.shift
reader.shift

# ファイルヘッダ出力
printf( "//====================================================================\n")
printf( "//    ショップデータ\n")
printf( "//\n")
printf( "//\n")
printf( "// ●fs_item.xlsをresource/shopdata/でコンバートして出力しています\n")
printf( "//\n")
printf( "//====================================================================\n\n")

# CSVデータから出力
reader.each { |row|
	printf( "// %s\n", row[0] )
	printf( "static const u16 %s[] = {\n", row[1].to_s )
	namelist << row[1].to_s

	# どうぐデータ出力
	(row.length-2).times{|i|
		if row[i+2]==nil then
			printf("\t\t0xffff,\n")
		else
			printf("\t\t%s,\n", hash[row[i+2].to_s])
		end
	}
	printf("};\n")
	printf("#define %s_MAX	(NELEMS(%s))\n\n", row[1].to_s.upcase,row[1].to_s)
	maxlist << sprintf( "%s_MAX",row[1].to_s.upcase,row[1].to_s )
}
reader.close

# 一括アクセス用に配列ポインタの配列を定義
printf("\n\n// 一括アクセス用配列定義\n")
printf("static const u16 *shop_data_table[]={\n")
namelist.each{ |name|
	printf("\t%s,\n", name)
}
printf("};\n")

# 各ショップのアイテム数の配列化
printf("\n\n// 各ショップ販売最大数取得用\n")
printf("static const u8 shop_itemnum_table[]={\n")
maxlist.each{ |maxdef|
	printf("\t%s,\n", maxdef)
}
printf("};\n")
