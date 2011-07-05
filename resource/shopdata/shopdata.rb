#===================================================================
#
# @brief  ショップデータコンバーター
#
# itemym.hとfs_item.csvを読み込んでショップデータを作成します。
#
# ex)
# ruby shopdata.rb fs_item.csv itemsym.h > fs_item.cdat fs_item_index.h
#
# @data   09.10.19
# @author mori
#
#===================================================================
require '../itemconv/item_hash.rb'

# ITEMSYM.hの定義取得を開始する行
ITEMSYM_H_START_LINE = 10
FIXSHOP_START_LINE	 = 7

# 文字コード指定
$KCODE = 'SJIS'

#CSV読み込み
require "csv"

#ワーク初期化
namelist   = Array.new	# 配列名
maxlist    = Array.new	# 配列MAX名
outbuf_num     = []     # 出力ショップ数
headerbuf  = ""			# スクリプトで指定するためのヘッダ
line_count = 0
shop_count = 0

#STDERR.print("エラー\n")
#exit(1)


#---------------------------------------
# ショップデータファイルオープン
#---------------------------------------
reader = CSV.open( ARGV[0], 'r')

# 3行読み飛ばし
reader.shift
reader.shift
reader.shift

# ヘッダファイルヘッダ部分出力
headerbuf += sprintf( "//====================================================================\n")
headerbuf += sprintf( "// @file  %s\n",ARGV[3])
headerbuf += sprintf( "// @brief ショップデータ指定用ヘッダ\n")
headerbuf += sprintf( "//\n")
headerbuf += sprintf( "// ●fs_item.xlsをresource/shopdata/でコンバートして出力しています\n")
headerbuf += sprintf( "//\n")
headerbuf += sprintf( "//====================================================================\n\n")
headerbuf += sprintf( "#ifndef __%s__\n",   ARGV[3].to_s.upcase.tr(".","_"))
headerbuf += sprintf( "#define __%s__\n\n", ARGV[3].to_s.upcase.tr(".","_"))


# CSVデータから出力
index = 0
reader.each { |row|
	if row[0].to_s=="" then
		break
  end
  outbuf_data = []
	namelist << row[1].to_s

	# どうぐデータ出力
  cnt = 0
	(row.length-2).times{|i|
		if row[i+2]!=nil then
			if $item_hash[row[i+2].to_s]!=nil then
				outbuf_data << [ $item_hash[row[i+2].to_s] ].pack('S')
        cnt += 1
			else
				STDERR.print( ""+row[0]+"の行、「"+row[i+2].to_s+"」と言うどうぐは存在しません\n")
				exit(1)
			end
		end
	}
  outbuf_num << [cnt.to_i].pack('C')

	maxlist << sprintf( "%s_MAX",row[1].to_s.upcase,row[1].to_s )
	shop_count=shop_count+1

	if shop_count>=FIXSHOP_START_LINE then
		headerbuf += sprintf("#define %s    ( %d )  // %s\n", row[1].to_s.upcase,
		shop_count-FIXSHOP_START_LINE,row[0].to_s)
  end


  #溜めたショプデータを出力(.dat)
  index_str = sprintf( "%03d", index )
  file = File.open("dat/#{index_str}_"+ARGV[2]+"_#{row[1]}.dat","wb")
  outbuf_data.join
  file.write(outbuf_data)
  file.close

  index+=1
}
reader.close

# ヘッダファイルの終端処理
headerbuf += sprintf( "\n\n#endif\n" )

#溜めたショプデータを出力(.dat)
file = File.open(ARGV[2]+"_num.dat","wb")
outbuf_num.join
file.write(outbuf_num)
file.close

#溜めたヘッダデータを出力(.h)
file = File.open(ARGV[3],"w")
file.printf(headerbuf)
file.close

exit(0)
