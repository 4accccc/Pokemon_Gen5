require "kowaza_table"

#===================================================================================
# 定数
#===================================================================================
# タマゴグループの数
EGG_GROUP_NUM = 15


#===================================================================================
# 先祖可能性テーブル
#===================================================================================
# 初期化
$potential_table = Array.new( EGG_GROUP_NUM+1 )
0.upto( EGG_GROUP_NUM ) do |i|
  $potential_table[i] = Array.new( EGG_GROUP_NUM+1 )
  0.upto( EGG_GROUP_NUM ) do |j|
    $potential_table[i][j] = false
  end
end


# 先祖可能性テーブルを作成
1.upto( EGG_GROUP_NUM ) do |my_group|
  # チェックリスト
  check = Array.new( EGG_GROUP_NUM+1 )
  0.upto( EGG_GROUP_NUM ) do |i|
    check[i] = false
  end 
  
  # 同グループからチェックを開始する
  $potential_table[my_group][my_group] = true
  check_group = my_group

  # チェック対象グループがなくなるまで繰り返す
  while check_group!=nil
    # チェック対象グループを持つポケモンをリストアップ
    poke_list = GetPokemonListByEggGroup( check_group ) 
    # リストアップしたポケモンの持つグループを先祖テーブルに登録
    poke_list.each do |poke_name|
      monsno = GetMonsNo( poke_name )
      data = $kowaza_table_data[ monsno ].split(/\s/)
      g1 = data[DATA_INDEX_EGG_GROUP_1].to_i
      g2 = data[DATA_INDEX_EGG_GROUP_2].to_i
      $potential_table[my_group][g1] = true
      $potential_table[my_group][g2] = true
    end
    # チェックリストを更新
    check[check_group] = true
    # 次のチェック対象グループを探す
    check_group = nil
    1.upto( EGG_GROUP_NUM ) do |g|
      if $potential_table[my_group][g]==true &&
         check[g]==false then
        check_group = g
        break
      end
    end
  end

  puts "先祖可能性テーブル作成: 子作りグループ[#{my_group}]完了"
end


# 先祖可能性テーブルを表示
1.upto( EGG_GROUP_NUM ) do |my_group|
  1.upto( EGG_GROUP_NUM ) do |your_group|
    if $potential_table[my_group][your_group]==true then
      print "○"
    else
      print "×"
    end
  end
  puts
end
