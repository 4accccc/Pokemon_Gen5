require "monsno_hash.rb"
require "wazaoboe_hash.rb"
require "wazaoboe_lv_hash.rb"
require "machinewaza_hash.rb"
require "kowaza_hash.rb"
require "oshiewaza_hash.rb"
require "reverse_evolve_poke_hash.rb"


class PersonalAccessor 
  def get_levelup_waza_list( mons_name )
    waza_list = $wazaoboe_hash[ mons_name ]
  end

  def get_levelup_wazalevel_list( mons_name )
    waza_level_list = $wazaoboe_lv_hash[ mons_name ]
    return waza_level_list
  end

  def get_wazalevel_list_of( mons_name, waza_name ) 
    waza_list = get_levelup_waza_list( mons_name )
    wazalevel_list = get_levelup_wazalevel_list( mons_name )
    level_list = Array.new
    0.upto( waza_list.size - 1 ) do |waza_idx|
      if waza_list[ waza_idx ] == waza_name then
        level_list << wazalevel_list[ waza_idx ]
      end
    end
    return level_list
  end

  def check_waza_learning_by_levelup( mons_name, waza_name )
    waza_list = $wazaoboe_hash[ mons_name ]
    if waza_list && waza_list.include?( waza_name ) then
      return true
    else
      return false
    end
  end

  def check_waza_learning_by_levelup_at( mons_name, mons_level, waza_name )
    waza_list = $wazaoboe_hash[ mons_name ]
    wazalevel_list = $wazaoboe_lv_hash[ mons_name ]
    0.upto( wazalevel_list.size - 1 ) do |waza_idx|
      check_waza_name = waza_list[ waza_idx ]
      check_waza_level = wazalevel_list[ waza_idx ]
      if check_waza_name == waza_name && check_waza_level <= mons_level then
        return true
      end
    end
    return false
  end

  def get_default_waza_list( mons_name, mons_level )
    default_waza_list = Array.new
    levelup_waza_list = $wazaoboe_hash[ mons_name ]
    levelup_wazalevel_list = $wazaoboe_lv_hash[ mons_name ]
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

  def check_waza_learning_by_machine( mons_name, waza_name )
    machine_waza_list = $machinewaza_hash[ mons_name ]
    if machine_waza_list && machine_waza_list.include?( waza_name ) then
      return true
    else
      return false
    end
  end

  def check_waza_learning_by_egg( mons_name, waza_name )
    egg_waza_list = $kowaza_hash[ mons_name ]
    if egg_waza_list && egg_waza_list.include?( waza_name ) then
      return true
    else
      return false
    end
  end

  def check_waza_learning_by_teach( mons_name, waza_name )
    teach_waza_list = $oshiewaza_hash[ mons_name ]
    if teach_waza_list && teach_waza_list.include?( waza_name ) then
      return true
    else
      return false
    end
  end

  def check_waza_learning( mons_name, mons_level, waza_name )
    # 自身のチェック
    if mons_name == "ドーブル" then
      return true 
    elsif check_waza_learning_by_teach( mons_name, waza_name ) then
      return true
    elsif check_waza_learning_by_egg( mons_name, waza_name ) then
      return true
    elsif check_waza_learning_by_machine( mons_name, waza_name ) then
      return true
    elsif check_waza_learning_by_levelup_at( mons_name, mons_level, waza_name ) then
      return true
    end 
    # 進化前ポケモンを再帰チェック
    reverse_evolve_poke_list = get_reverse_evolve_poke_list( mons_name )
    if reverse_evolve_poke_list != nil then
      reverse_evolve_poke_list.each do |prev_poke|
        if check_waza_learning( prev_poke, mons_level, waza_name ) then
          return true
        end
      end
    end

    return false
  end

  def get_reverse_evolve_poke_list( mons_name )
    reverse_evolve_poke_list = $reverse_evolve_poke_hash[ mons_name ]
    return reverse_evolve_poke_list
  end
end
