#--------------------------------------------------------------------------
# CountryList.xlsの５０音順のリストから全世界用の登録可能国リストを
# .cdatファイルで出力するコンバーター
#
# 2010.06.02 by mori
#--------------------------------------------------------------------------
require "csv"
require 'jcode'
$KCODE = "SJIS"

countrylist=[]	# 国番号を保存していく配列
countryname=[]	# 国名を保存していく配列

#引数チェック
if (ARGV[0]==nil||ARGV[1]==nil) then
	printf("引数を指定してください\nruby sort_listmake.rb <csvファイル> <出力cdatファイル>\n")
	exit
end

#csvファイルオープン
csv = CSV.open(ARGV[0],"r")

# 国名gmmリストの集計。頭文字をひらがな化して先頭の文字だけ取得する
csv.each{|row|
	if (row[0].to_a.index("Floor")==nil) then
		countrylist << row[2].to_a
		countryname << row[1].to_a
	end
}

File.open(ARGV[1],"w"){|file|

file.printf("//-----------------------------------------------------------------\n")
file.printf("/*\n")
file.printf("/* @file %s\n",ARGV[1])
file.printf(" */\n")
file.printf("//-----------------------------------------------------------------\n")
file.printf("#ifndef __%s__\n",ARGV[1].tr("a-z\.","A-Z_"))
file.printf("#define __%s__\n",ARGV[1].tr("a-z\.","A-Z_"))

file.printf("\n/// 登録可能国リスト（全ての国ではないことに注意）増やしたい場合は
///CountryList.xlsの５０音順の欄を変更してください。
ALIGN4 static const int EnableCountryList[] = {\n");
countrylist.zip(countryname).each{|index,name|
	file.printf("\t%s,\t// %s\n",index, name)
}
file.printf("};\n\n")

file.printf("static const COUNTRY_ENABLE_MAX	= NELEMS(EnableCountryList);\n")


file.printf("#endif")
}