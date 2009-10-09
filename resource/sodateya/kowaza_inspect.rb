#! ruby -Ks
#############################################################################################
#
# @brief  子技が習得可能かどうかを検証する
# @file   kowaza_inspect.rb
# @author obata
# @date   2009.10.09
#
############################################################################################# 
require "C:/home/pokemon_wb/tools/hash/monsno_hash"   # モンスターNo.
require "C:/home/pokemon_wb/tools/hash/wazaoboe_hash" # 技覚え
require "kowaza_table"
require "potential_table"

#============================================================================================
# 家計図
#============================================================================================
$family_tree = Array.new

#--------------------------------------------------------------------------------------------
# 家系図を表示する
#--------------------------------------------------------------------------------------------
def DispFamilyTree
  $family_tree.each do |name|
    print "#{name}--"
  end
  puts ""
end


#============================================================================================
# チェック済みリスト
#============================================================================================
$check = Array.new( GetMaxMonsNo() )
0.upto( $check.size-1 ) do |i|
  $check[i] = false
end


#--------------------------------------------------------------------------------------------
# 指定した技を覚えたモンスターの家系図を探る
#--------------------------------------------------------------------------------------------
def SearchOrigin( monsname, wazaname ) 
  monsno = GetMonsNo( monsname )
  data = $kowaza_table_data[monsno].split(/\s/)
  group1 = data[DATA_INDEX_EGG_GROUP_1].to_i
  group2 = data[DATA_INDEX_EGG_GROUP_2].to_i

  $family_tree.push( monsname )
  $check[monsno] = true
  DispFamilyTree()

  # 父になる可能性のある全てのポケモンを検証
  father_list = GetPokemonListByEggGroup(group1) | GetPokemonListByEggGroup(group2)
  father_list.each do |father|
    if $check[ GetMonsNo(father) ]==false then
      waza_list = $wazaoboe_hash[father]
      waza_list.each do |waza|
        if wazaname==waza then  # 自力で覚える父候補を発見
          $family_tree.push( father )
          return true
        end
      end
    end
  end

  # 父候補に自力で覚えるものがいなかったら, 縦型探索を行う
  father_list.each do |father|
    if $check[ GetMonsNo(father) ]==false then
      result = SearchOrigin( father, wazaname )
      if result==true then
        return true
      end
    end
  end

  # 見つからなかった場合
  file_tree.pop
  return false 
end


#--------------------------------------------------------------------------------------------
# メイン
#--------------------------------------------------------------------------------------------
result = SearchOrigin( ARGV[0], ARGV[1] )
puts "================================================="
if result==true then DispFamilyTree() end
