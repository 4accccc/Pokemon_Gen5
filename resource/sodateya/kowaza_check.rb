#! ruby -Ks

###################################################################################
#
# @brief  子技テーブルの整合性チェック
# @file   kowaza_check.rb
# @auther obata
# @date   2009.10.09
#
###################################################################################

# ハッシュの取り込み
require "C:/home/pokemon_wb/tools/hash/monsno_hash"   # モンスターNo.
require "C:/home/pokemon_wb/tools/hash/wazaoboe_hash" # 技覚え
require "kowaza_table"
require "potential_table"


#-----------------------------------------------------------------------------------
# メイン
#-----------------------------------------------------------------------------------

# エラーリスト
error_list = Array.new

# 全ポケモンをチェック
1.upto( $kowaza_table_data.size-1 ) do |i|
  data = $kowaza_table_data[i].split(/\s/)
  no   = data[DATA_INDEX_MONS_NO].to_i
  name = data[DATA_INDEX_MONS_NAME]
  group1 = data[DATA_INDEX_EGG_GROUP_1].to_i
  group2 = data[DATA_INDEX_EGG_GROUP_2].to_i
  if 0<no then
    puts "子技チェック: #{name}"
    # 子技リストを取得
    kowaza_list = GetKowazaList( name )
    # 全ての子技をチェック
    kowaza_list.each do |waza|
      print "-#{waza}"
      # チェック対象の子技を自力で覚えるポケモンをリストアップ
      poke_list = GetPokemonListByWaza( waza )
      # リストアップしたポケモンの中から, チェック対象ポケモンの先祖となり得るポケモンを探す
      ok = false
      poke_list.each do |father_name|
        father_monsno = $monsno_hash[father_name]
        father_data = $kowaza_table_data[father_monsno].split(/\s/)
        g1 = father_data[DATA_INDEX_EGG_GROUP_1].to_i
        g2 = father_data[DATA_INDEX_EGG_GROUP_2].to_i
        if $potential_table[group1][g1]==true ||
           $potential_table[group1][g2]==true ||
           $potential_table[group2][g1]==true ||
           $potential_table[group2][g2]==true then
          ok = true
        end
      end
      if ok==true then
        puts "○"
      else
        puts "×"
        if group1!=15 && group2!=15 then  # 無生殖グループ(15)の警告は出さない！
          error_list << "#{name} は #{waza} を覚えられません!!"
        end
      end
    end
  end
end

# エラーリストを表示
error_list.each do |message|
  puts message
end
