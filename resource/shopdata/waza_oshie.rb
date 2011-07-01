#=======================================================================
#
# @brief  技教えデータコンバータ
#
# wazano_oshie.csvと技番号ハッシュを読み込んで技教えデータを作成します
#
# ex)
# ruby waza_oshie.rb waza_oshie.csv > waza_oshie.cdat
#
# @data   11/06/30
# @author ichiraku
#
#=======================================================================

# 文字コード指定
$KCODE = 'SJIS'

#CSV読み込み
require "csv"

#育てやのハッシュを使わせてもらう
require "../sodateya/hash.rb" 

#ワーク初期化
namelist   = Array.new	# 配列名
maxlist    = Array.new	# 配列MAX名
outbuf	   = ""			# 出力ショップデータ群
lastnamebuf = ""		# ショップ定義名を一回分覚えておく
closeflag  = 0			# 初回だけは配列定義を括る必要が無いのでフラグ


#-------------------------------------
# 技教えデータファイルオープン
#-------------------------------------
reader = CSV.open( ARGV[0], 'r')

# 1行読み飛ばし
reader.shift

# ショップデータヘッダ部分出力
outbuf += sprintf( "//====================================================================\n")
outbuf += sprintf( "// @file  %s\n", ARGV[1])
outbuf += sprintf( "// @brief 技教えデータ\n")
outbuf += sprintf( "//\n")
outbuf += sprintf( "// ●waza_oshie.xlsをresource/shopdata/でコンバートして出力しています\n")
outbuf += sprintf( "//\n")
outbuf += sprintf( "//====================================================================\n\n")

# CSVデータから出力
reader.each { |row|

  # 1行目に色が入ってる場合は技教えデータ定義開始
  if row[0].to_s != "" then

    # 前の行までのデータ定義を終了する
    if closeflag==1 then
			outbuf += sprintf("};\n")
			outbuf += sprintf("#define %s_MAX	(NELEMS(%s))\n\n", lastnamebuf.upcase,lastnamebuf)
			maxlist << sprintf( "%s_MAX",lastnamebuf.upcase,lastnamebuf )
		end

    #新しいデータ定義の宣言
    outbuf += sprintf( "// %s\n", row[0] )
		outbuf += sprintf( "static const SHOP_ITEM %s[] = {\n", row[1].to_s )
		namelist << row[1].to_s
		closeflag = 1
		lastnamebuf = row[1].to_s
  end

  if row[2].to_s!="" then
		if row[3].to_s=="" then
			STDERR.print( ""+row[2]+"の横にコストが書いてありません\n")
			exit(1)
		end
		if GetWazaNo( row[2].to_s ) != nil then
			outbuf += sprintf("\t\t{ %s,\t%s, },  // %s\n", GetWazaNo( row[2].to_s ),row[3].to_s, row[2].to_s)
		else
			STDERR.print( "「"+row[2].to_s+"」と言うどうぐは存在しません\n")
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
outbuf += sprintf("static const SHOP_ITEM *waza_oshie_data_table[]={\n")
namelist.each{ |name|
	outbuf += sprintf("\t%s,\n", name)
}
outbuf += sprintf("};\n")


# 各技教えのコスト数の配列化
outbuf += sprintf("\n\n// 各ショップ販売最大数取得用\n")
outbuf += sprintf("static const u8 waza_oshie_datanum_table[]={\n")
maxlist.each{ |maxdef|
	outbuf += sprintf("\t%s,\n", maxdef)
}
outbuf += sprintf("};\n")


#溜めたショプデータを出力(.cdat)
file = File.open(ARGV[1],"w")
file.printf(outbuf)
file.close

exit(0)
