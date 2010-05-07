#==================================================
# wifiearth_gmm.rb
# エクセルファイルから取得した国名・地域名を
# gmmファイルに変換するスクリプト
#
# 2010/05/04 mori
#
# 引数１：参照するCSVファイル
# 引数２：出力するGMMファイル
# 同じフォルダに「template.gmm」ファイルが存在している必要があります。
#
# 使用例）
# ruby wifiearth_gmm.rb world.csv wifi_earth_msg_world.gmm
#
#==================================================
require "../../tools/gmm_make/gmm_make"   # gmm生成クラス
require "csv"

#GMMクラス初期化
gmm = GMM::new
gmm.open_gmm( "template.gmm", ARGV[1] )

#csvファイルオープン
csv = CSV.open(ARGV[0],"r")
csv.each{|row|
	# 空行か「ID」の行は出力しないようにする
	if (row[0]!=nil && row[0].to_a.index("ID")==nil) then
		gmm.make_row_kanji( row[0], row[1], row[1] )
#		printf("%s, %s\n", row[0].to_a, row[1].to_a)
	end
}

#GMM出力
gmm.close_gmm
