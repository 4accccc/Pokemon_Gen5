require "form_name_hash.rb"
require "levelwaza_hash.rb"
require "levelwaza_learninglevel_hash.rb"
require "machinewaza_hash.rb"
require "eggwaza_hash.rb"
require "teachwaza_hash.rb"
require "reverse_evolve_poke_hash.rb"
require "egg_group_hash.rb"
require "egg_group_poke_list_hash.rb"
require "gender_hash.rb"


class PersonalAccessor 
  def get_levelwaza_list( mons_fullname )
    waza_list = $levelwaza_hash[ mons_fullname ]
  end

  def get_levelwaza_learninglevel_list( mons_fullname )
    level_list = $levelwaza_learninglevel_hash[ mons_fullname ]
    return waza_level_list
  end

  def get_wazalevel_list_of( mons_fullname, waza_name ) 
    waza_list = get_levelwaza_list( mons_fullname )
    wazalevel_list = get_levelwaza_learninglevel_list( mons_fullname )
    level_list = Array.new
    0.upto( waza_list.size - 1 ) do |waza_idx|
      if waza_list[ waza_idx ] == waza_name then
        level_list << wazalevel_list[ waza_idx ]
      end
    end
    return level_list
  end

  def check_waza_learning_by_levelup( mons_fullname, waza_name )
    waza_list = $levelwaza_hash[ mons_fullname ]
    if waza_list && waza_list.include?( waza_name ) then
      return true
    else
      return false
    end
  end

  def check_waza_learning_by_levelup_at( mons_fullname, mons_level, waza_name )
    waza_list = $levelwaza_hash[ mons_fullname ]
    wazalevel_list = $levelwaza_learninglevel_hash[ mons_fullname ]
    0.upto( wazalevel_list.size - 1 ) do |waza_idx|
      check_waza_name = waza_list[ waza_idx ]
      check_waza_level = wazalevel_list[ waza_idx ]
      if check_waza_name == waza_name && check_waza_level <= mons_level then
        return true
      end
    end
    return false
  end

  def get_default_waza_list( mons_fullname, mons_level )
    puts mons_fullname
    default_waza_list = Array.new
    levelup_waza_list = $levelwaza_hash[ mons_fullname ]
    levelup_wazalevel_list = $levelwaza_learninglevel_hash[ mons_fullname ]
    0.upto( levelup_wazalevel_list.size - 1 ) do |waza_idx|
      waza_name = levelup_waza_list[ waza_idx ]
      waza_level = levelup_wazalevel_list[ waza_idx ]
      if waza_level <= mons_level then
        default_waza_list << waza_name
      end
    end
    # 習得レベルの昇順で並んでいる, という前提で後ろの4つを残す
    while 4 < default_waza_list.size
      default_waza_list.slice!(0)
    end
    return default_waza_list
  end

  def check_waza_learning_by_machine( mons_fullname, waza_name )
    machine_waza_list = $machinewaza_hash[ mons_fullname ]
    if machine_waza_list && machine_waza_list.include?( waza_name ) then
      return true
    else
      return false
    end
  end

  def check_waza_learning_by_egg( mons_fullname, waza_name )
    egg_waza_list = $eggwaza_hash[ mons_fullname ]
    if egg_waza_list && egg_waza_list.include?( waza_name ) then
      return true
    else
      return false
    end
  end

  def check_waza_learning_by_teach( mons_fullname, waza_name )
    teach_waza_list = $teachwaza_hash[ mons_fullname ]
    if teach_waza_list && teach_waza_list.include?( waza_name ) then
      return true
    else
      return false
    end
  end

  def check_waza_learning( mons_fullname, mons_level, waza_name )
    # 自身のチェック
    if mons_fullname == "ドーブル" then
      return true 
    elsif check_waza_learning_by_teach( mons_fullname, waza_name ) then
      return true
    elsif check_waza_learning_by_egg( mons_fullname, waza_name ) then
      return true
    elsif check_waza_learning_by_machine( mons_fullname, waza_name ) then
      return true
    elsif check_waza_learning_by_levelup_at( mons_fullname, mons_level, waza_name ) then
      return true
    end 
    # 進化前ポケモンを再帰チェック
    reverse_evolve_poke_list = get_reverse_evolve_poke_list( mons_fullname )
    if reverse_evolve_poke_list != nil then
      reverse_evolve_poke_list.each do |prev_poke|
        if check_waza_learning( prev_poke, mons_level, waza_name ) then
          return true
        end
      end
    end 
    return false
  end

  # タマゴ技以外の方法で覚えるかどうか？
  def check_waza_learning_except_eggwaza( mons_fullname, mons_level, waza_name )
    # 自身のチェック
    if mons_fullname == "ドーブル" then
      return true 
    elsif check_waza_learning_by_teach( mons_fullname, waza_name ) then
      return true
    elsif check_waza_learning_by_machine( mons_fullname, waza_name ) then
      return true
    elsif check_waza_learning_by_levelup_at( mons_fullname, mons_level, waza_name ) then
      return true
    end 
    # 進化前ポケモンを再帰チェック
    # ( 技を覚えるまで進化をキャンセルした場合への対応 )
    reverse_evolve_poke_list = get_reverse_evolve_poke_list( mons_fullname )
    if reverse_evolve_poke_list != nil then
      reverse_evolve_poke_list.each do |prev_poke|
        if check_waza_learning_except_eggwaza( prev_poke, mons_level, waza_name ) then
          return true
        end
      end
    end 
    return false
  end

  def get_reverse_evolve_poke_list( mons_fullname )
    reverse_evolve_poke_list = $reverse_evolve_poke_hash[ mons_fullname ]
    return reverse_evolve_poke_list
  end

  def get_egg_group_list( mons_fullname )
    egg_group_list = $egg_group_hash[ mons_fullname ]
    return egg_group_list
  end

  def get_egg_group1( mons_fullname )
    egg_group_list = get_egg_group_list( mons_fullname )
    return egg_group_list[0]
  end

  def get_egg_group2( mons_fullname )
    egg_group_list = get_egg_group_list( mons_fullname )
    return egg_group_list[1]
  end

  def get_poke_list_belong_to_egg_group( egg_group_name )
    poke_list = $egg_group_poke_list_hash[ egg_group_name ]
    return poke_list
  end

  def get_eggwaza_list( mons_fullname )
    eggwaza_list = $eggwaza_hash[ mons_fullname ]
    return eggwaza_list
  end

  def check_eggwaza_exist( mons_fullname )
    egg_waza_list = get_eggwaza_list( mons_fullname )
    if egg_waza_list == nil then
      return false
    else
      return true
    end
  end

  def get_gender_type( mons_fullname )
    gender_type = $gender_hash[ mons_fullname ]
    return gender_type
  end

  def get_mons_fullname_list( mons_name )
    fullname_list = Array.new
    form_list = $form_name_hash[ mons_name ]
    form_list.each do |form_name|
      fullname = mons_name + form_name
      fullname_list << fullname
    end
    return fullname_list
  end

  def get_all_mons_fullname_list
    return $mons_fullname_list
  end
end
