#//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#/**
# *
# *	@file		zone_group_hash_cnv.rb
# *	@brief  ゾーングループハッシュを作成
# *	@author	Masayuki Onoue
# *	@date		2011.09.02
# *
# */
#//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

# 定数
ZONE_ID_INDEX = 0
ZONE_GROUP_INDEX = 25

# 引数の取得
hash_name   = ARGV[0]
table_name  = ARGV[1]

# ファイルオープン
fp_table = open(table_name,"r")
fp_hash  = open(hash_name,"w")

# ゾーングループを抜き出す
fp_table.gets # 読み飛ばし
zone_hash = Hash.new

fp_table.each{ |line|

  # 終了判定
  if line.include?("END") then break end

  # データ分割
  data = line.split(/\s*\s\s*/)
  zone_id    = data[ZONE_ID_INDEX]
  zone_group = data[ZONE_GROUP_INDEX]

  # エラーチェック
  if zone_hash.key?(zone_id) then
    raise "zone_idが重複しています<=%s"[zone_id]
  end

  # ゾーングループを保存
  zone_hash[zone_id] = zone_group
}

header = <<EOS
#//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#/**
# *
# *	@file		%s
# *	@brief  ゾーン名をゾーングループに変換する
# *	@author	Masayuki Onoue
# *	@date		2011.09.02
# * @note   コンバータが出力したファイルです
# *
# */
#//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$zohe_id_to_zone_group = {
EOS
fp_hash.puts header%[hash_name]

fp_hash

# テーブル出力
zone_hash.each_key{|zone_id|
  fp_hash.puts '  "%s"=>"%s",'%[zone_id.to_s,zone_hash[zone_id].to_s]
}
fp_hash.puts "}"

fp_table.close
fp_hash.close
