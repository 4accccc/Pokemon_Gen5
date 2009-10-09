#! ruby -Ks

###################################################################################
#
# @brief  子技テーブルをバイナリへコンバートする
# @file   kowaza_conv.rb
# @auther obata
# @date   2009.10.09
#
# 使用方法
#  ruby kowaza_conv.rb バイナリ出力先のパス
#
###################################################################################

# ハッシュの取り込み
require "C:/home/pokemon_wb/tools/hash/monsno_hash"   # モンスターNo.
require "C:/home/pokemon_wb/tools/hash/wazaoboe_hash" # 技覚え
require "kowaza_table"
require "wazano_hash.rb" 



#-----------------------------------------------------------------------------------
# メイン
#-----------------------------------------------------------------------------------

# バイナリファイルリスト
file_list = Array.new

# 最大モンスターナンバーを取得
monsno_max = GetMaxMonsNo()

# モンスターナンバー0用にダミーを出力
filename = "./#{ARGV[0]}/kowaza_data0.bin"
file = File.open( filename, "wb" )
file.close
file_list << filename

# 全ポケモンの子技データを出力
1.upto( monsno_max ) do |monsno|

  # 子技リストを取得
  kowaza_list = GetKowazaList_byMonsNo( monsno )

  # バイナリデータを出力
  data = Array.new
  data << kowaza_list.size
  kowaza_list.each do |waza_name|
    data << GetWazaNo( waza_name )
  end
  filename = "./#{ARGV[0]}/kowaza_data#{monsno}.bin"
  file = File.open( filename, "wb" )
  file.print( data.pack("S*") )
  file.close
  file_list << filename
  puts "子技バイナリデータ出力: #{filename}"
end


# バイナリファイルリストを出力
file = File.open( "conv_data.list", "w" ) 
file.print( "CONVERTDATA = " )
file_list.each do |filename|
  file.print( filename )
  file.print( " " )
end
file.close
