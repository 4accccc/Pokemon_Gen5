#==================================================
# sortlist_make.rb
# エクセルファイルから取得したソート済国名を
# GTSで使用するcdatファイルに変換するスクリプト
#
# 2010/05/07 mori
#
# 引数１：参照するCSVファイル
# 引数２：出力するcdatファイル名
#
# 使用例）
# ruby sortlist_make.rb aiueo.txt gts_coutrylist.cdat
#
#==================================================
require "csv"
require 'jcode'
$KCODE = "SJIS"

tmpstr=[]		#一時処理用配列
coutrylist=[]	#gmm定義を保存していく配列
headlist=[]		#国名頭文字を保存していく配列
headcount=Array.new(50,0)	# 頭文字登場回数を数える配列
headstart=Array.new(50,0)	# headcountの数値を足しこんで指定の頭文字のスタート位置を取得する配列
searchstr='あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわ'
dakustr='がぎぐげござじずぜぞだぢづでどぱぴぷぺぽばびぶべぼ'	#濁点・半濁点を外すための
kanastr='かきくけこさしすせそたちつてとはひふへほはひふへほ'	#置換用文字列

#引数チェック
if (ARGV[0]==nil||ARGV[1]==nil) then
	printf("引数を指定してください\nruby sort_listmake.rb <csvファイル> <出力cdatファイル>\n")
	exit
end

#csvファイルオープン
csv = CSV.open(ARGV[0],"r")

# 国名gmmリストの集計。頭文字をひらがな化して先頭の文字だけ取得する
csv.each{|row|
	# 空行か「ID」の行は出力しないようにする
	if (row[0].to_a.index("Floor")==nil) then
		coutrylist << row[2].to_a
		tmpstr = row[1].tr('ァ-ン', 'ぁ-ん').tr(dakustr,kanastr)
		headlist<<tmpstr.split(//)[0]
#		printf("%s, %s\n", tmpstr.to_a, row[2].to_a)
	end
}

# 頭文字の出現回数を数え上げる
headlist.each{|head|
	a = searchstr.index(head)/2+1
	headcount[a] = headcount[a]+1
#	printf("%d,\n",searchstr.index(head))
}

# 数え上げた頭文字の出現回数をスタート位置に変換する
50.times{|i|
	0..i.times{|n|
		headstart[i] = headstart[i]+headcount[n];
	}
}

#----------------------------------------------
# .cdatにファイル出力
#----------------------------------------------

File.open(ARGV[1],"w"){|file|

file.printf("//-----------------------------------------------------------------\n")
file.printf("/*\n")
file.printf("/* @file %s\n",ARGV[1])
file.printf(" */\n")
file.printf("//-----------------------------------------------------------------\n")
file.printf("#ifndef __%s__\n",ARGV[1].tr("a-z\.","A-Z_"))
file.printf("#define __%s__\n",ARGV[1].tr("a-z\.","A-Z_"))

file.printf("\n///国検索：50音順リスト
ALIGN4 static const u16 CountryListTbl[] = {\n");
coutrylist.each{|name|
	file.printf("\t%s,\n",name)
}
file.printf("};\nconst u32 CountryListTblNum = NELEMS(CountryListTbl);\n")

searchstr = searchstr.split(//)

file.printf("\n// アイウエオ順で国名CountryListが何番目に出現するかを
// 格納しているリスト
// {  CountryListの添え字,   アイウエオの添え字（ア=0,カ=5,ヤ=35,ラ=38)
static const u8 CountrySortList[][2]={\n")

50.times{|i|
	if headcount[i]!=0 then
		file.printf("\t{ %2d, %2d }, // %s\n", headstart[i], i, searchstr[i-1])
	end
}
file.printf("\t{ %3d, %3d }, // 番兵\n", headstart[49], 255)

file.printf("};\n\n")

file.printf("static const COUNTRY_SORT_HEAD_MAX	= NELEMS(CountrySortList);\n")

file.printf("#endif")
}