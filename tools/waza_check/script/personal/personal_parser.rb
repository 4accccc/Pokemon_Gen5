require "personal_index.rb" 
require "monsno_hash.rb"

# 教え技
$all_oshiewaza_list = [
  "くさのちかい",
  "ほのおのちかい",
  "みずのちかい",
  "ハードプラント",
  "ブラストバーン",
  "ハイドロカノン",
  "りゅうせいぐん",
] 

# タマゴグループ
$egg_group_list = [
"なし", # 不正値
"怪獣", 
"水棲", 
"虫",
"鳥",
"動物",
"妖精",
"植物",
"人型",
"甲殻",
"無機物",
"不定形",
"魚",
"メタモン",
"ドラゴン",
"無生殖",
]

# 性別
GENDER_TYPE_MALE_ONLY = 0
GENDER_TYPE_NORMAL = 127
GENDER_TYPE_FEMALE_ONLY = 254
GENDER_TYPE_UNKNOWN = 255


class PersonalDataParser
  def intialize
    @data = nil
  end

  def load_personal_file( filename )
    file = File::open( filename, "r" )
    @data = file.readlines
    file.close 
  end

  def get_personal_data( mons_name )
    monsno = $monsno_hash[ mons_name ].to_i
    personal_data = @data[ monsno - 1 ]
    return personal_data
  end

  def get_personal_items( mons_name )
    personal_data = get_personal_data( mons_name )
    copy_data = personal_data.gsub( "\"", "" )
    copy_data.gsub!( /\s/, "" )
    personal_items = copy_data.split( "," )
    return personal_items
  end


  def get_mons_name( personal_items )
    mons_name = personal_items[ PERSONAL_PARAM::POKENAME ] 
    return mons_name
  end

  def get_gender_type( mons_name )
    personal_items = get_personal_items( mons_name )
    gender_type = personal_items[ PERSONAL_PARAM::GENDER ].to_i
    if gender_type != GENDER_TYPE_MALE_ONLY &&
       gender_type != GENDER_TYPE_FEMALE_ONLY &&
       gender_type != GENDER_TYPE_UNKNOWN then
      gender_type = GENDER_TYPE_NORMAL
    end
    return gender_type
  end

  def get_egg_group_list( mons_name )
    egg_group_list = Array.new
    egg_group_list << get_egg_group1( mons_name )
    egg_group_list << get_egg_group2( mons_name )
    return egg_group_list
  end

  def get_egg_group1( mons_name )
    personal_items = get_personal_items( mons_name )
    egg_group_idx = personal_items[ PERSONAL_PARAM::EGG_GROUP1 ].to_i
    egg_group = $egg_group_list[ egg_group_idx ]
    return egg_group
  end

  def get_egg_group2( mons_name )
    personal_items = get_personal_items( mons_name )
    egg_group_idx = personal_items[ PERSONAL_PARAM::EGG_GROUP2 ].to_i
    egg_group = $egg_group_list[ egg_group_idx ]
    return egg_group
  end

  def get_evolve_poke_list( mons_name )
    evolve_poke_list = Array.new
    personal_items = get_personal_items( mons_name )
    PERSONAL_PARAM::SHINKA_POKE1.upto( PERSONAL_PARAM::SHINKA_POKE7 ) do |item_idx|
      ev_poke = personal_items[ item_idx ]
      if ev_poke != "" then
        evolve_poke_list << ev_poke
      end
    end
    return evolve_poke_list
  end

  def get_levelup_waza_list( mons_name )
    personal_items = get_personal_items( mons_name )
    waza_list = Array.new
    PERSONAL_PARAM::WAZA1.upto( PERSONAL_PARAM::WAZA25 ) do |item_idx|
      waza_name = personal_items[ item_idx ]
      if waza_name != "" then 
        waza_list << waza_name
      end
    end
    return waza_list
  end

  def get_levelup_wazalevel_list( mons_name )
    personal_items = get_personal_items( mons_name )
    waza_level_list = Array.new
    PERSONAL_PARAM::WAZA_LV1.upto( PERSONAL_PARAM::WAZA_LV25 ) do |item_idx|
      waza_level = personal_items[ item_idx ]
      if waza_level != "" then
        waza_level_list << waza_level.to_i
      end
    end
    return waza_level_list
  end

  def get_usable_waza_machine_list( mons_name )
    usable_machine_list = Array.new
    personal_items = get_personal_items( mons_name )
    machine_data = personal_items[ PERSONAL_PARAM::WAZA_MACHINE ]
    machine_data_split = machine_data.split( // )
    0.upto( machine_data_split.size - 1 ) do |machine_idx|
      if machine_data_split[ machine_idx ] == "●" then 
        machine_name = GetWazaMachineName( machine_idx + 1 )
        usable_machine_list << machine_name
      end
    end
    return usable_machine_list
  end

  def get_teach_waza_list( mons_name )
    teach_waza_list = Array.new
    personal_items = get_personal_items( mons_name )
    teach_waza_data = personal_items[ PERSONAL_PARAM::WAZA_OSHIE ]
    teach_waza_data_split = teach_waza_data.split( // )
    0.upto( teach_waza_data_split.size - 1 ) do |idx|
      if teach_waza_data_split[ idx ] == "●" then
        waza_name = $all_oshiewaza_list[ idx ]
        teach_waza_list << waza_name
      end
    end
    return teach_waza_list
  end

end

