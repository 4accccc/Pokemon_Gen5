# @brief  ポケウッドポケモンの覚えている技が正しいかチェックした結果を出力する
# @file   waza_check_for_pkwd_poke.rb
# @author Masayuki Onoue
# @date   2011.10.26

require "personal_accessor.rb"
require "oshiewaza_hash.rb"
require "waza_oshie_hash.rb"
require "form_name_hash.rb"
require "fr_old_owaza_hash.rb"
require "gs_old_owaza_hash.rb"
require "pl_old_owaza_hash.rb"
require "old_oshiewaza_hash.rb"

# 教え技か調べる
def is_oshiewaza( waza_name )
  if $oshie_waza_hash_red.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_blue.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_yellow.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_green.values.include?( waza_name ) then return true end
  return false
end

# 教え技か調べる
def is_old_oshiewaza( mons_name,waza_name )
  # 過去教え技
  if check_old_waza_hash($old_oshiewaza_hash,mons_name,waza_name,waza_name ) then
    return true
  end
  return false
end

# 過去技か調べる
def is_old_waza( mons_name ,waza_name )

  # fr
  if check_old_waza_hash($fr_old_owaza_hash ,mons_name ,waza_name , waza_name ) then
    return true
  end
  # gs
  if check_old_waza_hash($gs_old_owaza_hash ,mons_name ,waza_name , waza_name ) then 
    return true
  end
  # pl
  if check_old_waza_hash($pl_old_owaza_hash ,mons_name ,waza_name , waza_name ) then 
    return true 
  end

  return false
end

# ハッシュ内をチェック
def check_old_waza_hash( hash, mons_name ,waza_name , waza )
  if hash.key?(mons_name) then
    if hash[mons_name].include?(waza) then return true end
  end
  return false
end

# =======================================================
# メイン処理
# =======================================================

# 定数定義
PKWD_INFO_SCENARIO_NO_ROW   = 0
PKWD_INFO_POKE_TYPE_ROW     = 1
PKWD_INFO_POKE_INDEX_ROW    = 2
PKWD_INFO_POKE_NAME_ROW     = 3
PKWD_INFO_FORM_NO_ROW       = 4
PKWD_INFO_SKILL_NAME_1_ROW  = 5
PKWD_INFO_SKILL_NAME_2_ROW  = 6
PKWD_INFO_SKILL_NAME_3_ROW  = 7
PKWD_INFO_SKILL_NAME_4_ROW  = 8


# パーソナルデータ参照
personal_accessor = PersonalAccessor.new

# 技チェック
out_data = Array.new
out_file = File::open( "../result_pkwood_poke.txt", "w" )
out_file.puts( "！==========================================================!")
out_file.puts( "！このファイルはwaza_check_for_pkwd_poke.rbが作成したファイルです" )
out_file.puts( "！ポケウッドポケモンが覚えれない技を覚えていないかチェックしています" )
out_file.puts( "！レベル、技教え、技マシン技、教え技、タマゴ技、進化が対象です" )
out_file.puts( "！==========================================================!")


pkwd_file = File::open( "../data/pkwd_info.csv", "r" )

pkwd_file.each do |line|
  out_data = ""
  
  # 技データ配列
  skill_name_array = Array.new
  # データの分割
  split_line = line.split(",")

  # ポケウッドデータを取得
  scenario_no = split_line[PKWD_INFO_SCENARIO_NO_ROW].to_i
  poke_type   = split_line[PKWD_INFO_POKE_TYPE_ROW].to_i
  poke_index  = split_line[PKWD_INFO_POKE_INDEX_ROW].to_i
  poke_name   = split_line[PKWD_INFO_POKE_NAME_ROW].to_s.strip
  form_no     = split_line[PKWD_INFO_FORM_NO_ROW].to_i

  if poke_name.include?("セット") then
    next
  end
  
  # フォルムつき名を取得
  form_list = $form_name_hash[poke_name]
  full_name = poke_name + form_list[form_no]

  # スキル名を取得
  4.times{|index|
    skill_name = split_line[PKWD_INFO_SKILL_NAME_1_ROW+index].to_s
    
    # 数値が入っていた場合はそこで終了
    if skill_name == "0" then
      break
    end
    skill_name_array << skill_name
  }


  skill_name_array.each do |waza|
    oshiewaza_list = $oshiewaza_hash[ full_name ]
    
    # 技チェック
    unless (personal_accessor.check_waza_learning( full_name, 100 , waza )) ||
      ( is_oshiewaza( waza ) == true && oshiewaza_list.include?( waza ) == true ) ||
      (is_old_waza( full_name ,waza ) == true ) ||
      (is_old_oshiewaza( full_name,waza) == true) then

      if poke_type == 0 then
        out_data << "シナリオNo.%d:MY_POKE %d %s => %s ×\n"%[scenario_no,poke_index,poke_name,waza]
      else
        out_data << "シナリオNo.%d:ENEMY   %d %s => %s ×\n"%[scenario_no,poke_index,poke_name,waza]
      end

    end
  end
  unless out_data == "" then out_file.puts( out_data ) end
end

out_file.close 

