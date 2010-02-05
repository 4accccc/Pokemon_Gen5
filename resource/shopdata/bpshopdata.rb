#===================================================================
#
# @brief  BPショップデータコンバーター
#
# itemym.hとbp_item.csvを読み込んでBPショップデータを作成します。
#
# ex)
# ruby bpshopdata.rb bp_item.csv itemsym.h > bp_item.cdat bp_item_inde.h
#
# @data   10.02.05
# @author mori
#
#===================================================================

# ITEMSYM.hの定義取得を開始する行
ITEMSYM_H_START_LINE = 10
FIXSHOP_START_LINE	 = 7

# 文字コード指定
$KCODE = 'SJIS'

#CSV読み込み
require "csv"

#ワーク初期化
hash       = Hash.new	# どうぐリスト
namelist   = Array.new	# 配列名
maxlist    = Array.new	# 配列MAX名
outbuf	   = ""			# 出力ショップデータ群
headerbuf  = ""			# スクリプトで指定するためのヘッダ
lastnamebuf = ""		# ショップ定義名を一回分覚えておく
line_count = 0
shop_count = 0
closeflag  = 0			# 初回だけは配列定義を括る必要が無いのでフラグ



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

# 1行読み飛ばし
reader.shift

# ショップデータヘッダ部分出力
outbuf += sprintf( "//====================================================================\n")
outbuf += sprintf( "// @file  %s\n", ARGV[2])
outbuf += sprintf( "// @brief BPショップデータ\n")
outbuf += sprintf( "//\n")
outbuf += sprintf( "// ●bp_item.xlsをresource/shopdata/でコンバートして出力しています\n")
outbuf += sprintf( "//\n")
outbuf += sprintf( "//====================================================================\n\n")

# ヘッダファイルヘッダ部分出力
headerbuf += sprintf( "//====================================================================\n")
headerbuf += sprintf( "// @file  %s\n",ARGV[3])
headerbuf += sprintf( "// @brief BPショップデータ指定用ヘッダ\n")
headerbuf += sprintf( "//\n")
headerbuf += sprintf( "// ●bp_item.xlsをresource/shopdata/でコンバートして出力しています\n")
headerbuf += sprintf( "//\n")
headerbuf += sprintf( "//====================================================================\n\n")
headerbuf += sprintf( "#ifndef __%s__\n",   ARGV[3].to_s.upcase.tr(".","_"))
headerbuf += sprintf( "#define __%s__\n\n", ARGV[3].to_s.upcase.tr(".","_"))


# CSVデータから出力
reader.each { |row|
	# 1列目に名前が入っている場合はBPショップデータ定義開始
	if row[0].to_s!="" then
	
		#前の行までのデータ定義を終了する
		if closeflag==1 then
#			outbuf += sprintf("\t\t{0xffff, 0xffff, },\n")
			outbuf += sprintf("};\n")
			outbuf += sprintf("#define %s_MAX	(NELEMS(%s))\n\n", lastnamebuf.upcase,lastnamebuf)
			maxlist << sprintf( "%s_MAX",lastnamebuf.upcase,lastnamebuf )
		end

		#新しいデータ定義の宣言
		outbuf += sprintf( "// %s\n", row[0] )
		outbuf += sprintf( "static const SHOP_ITEM %s[][2] = {\n", row[1].to_s )
		namelist << row[1].to_s
		closeflag = 1
		lastnamebuf = row[1].to_s
		headerbuf += sprintf("#define %s    ( %d )  // %s\n", 
							 row[1].to_s.upcase,shop_count,row[0].to_s)
		shop_count=shop_count+1
					     		

	end

	if row[3].to_s!="" then
		if row[4].to_s=="" then
			STDERR.print( ""+row[3]+"の横に値段が書いてありません\n")
			exit(1)
		end
		if hash[row[3].to_s]!=nil then
			outbuf += sprintf("\t\t{ %s,\t%s, },\n", hash[row[3].to_s],row[4].to_s)
		else
			STDERR.print( "「"+row[3].to_s+"」と言うどうぐは存在しません\n")
			exit(1)
		end

		
	end
}
reader.close

#最後の定義は終端が無いのでココで括る
#outbuf += sprintf("\t\t{0xffff, 0xffff, },\n")
outbuf += sprintf("};\n")
outbuf += sprintf("#define %s_MAX	(NELEMS(%s))\n\n", lastnamebuf.upcase,lastnamebuf)
maxlist << sprintf( "%s_MAX",lastnamebuf.upcase,lastnamebuf )


# 一括アクセス用に配列ポインタの配列を定義
outbuf += sprintf("\n\n// 一括アクセス用配列定義\n")
outbuf += sprintf("static const SHOP_ITEM *bp_shop_data_table[]={\n")
namelist.each{ |name|
	outbuf += sprintf("\t%s,\n", name)
}
outbuf += sprintf("};\n")


# ヘッダファイルの終端処理
headerbuf += sprintf( "\n\n#endif\n" )

# 各ショップのアイテム数の配列化
outbuf += sprintf("\n\n// 各ショップ販売最大数取得用\n")
outbuf += sprintf("static const u8 bp_shop_itemnum_table[]={\n")
maxlist.each{ |maxdef|
	outbuf += sprintf("\t%s,\n", maxdef)
}
outbuf += sprintf("};\n")


#溜めたショプデータを出力(.cdat)
file = File.open(ARGV[2],"w")
file.printf(outbuf)
file.close

#溜めたヘッダデータを出力(.h)
file = File.open(ARGV[3],"w")
file.printf(headerbuf)
file.close

exit(0)