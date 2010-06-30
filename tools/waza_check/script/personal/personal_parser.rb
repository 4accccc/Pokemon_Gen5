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

# アナザーフォルムの定義開始行番号
ROW_FIRST_ANOTHER_FORM = 650


class PersonalDataParser
  def initialize( personal_filename )
    @data = nil
    @data_index_hash = Hash.new
    @form_name_hash = Hash.new
    self.load_personal_file( personal_filename )
    self.create_data_index_hash
    self.create_form_name_hash
  end

  def load_personal_file( filename )
    file = File::open( filename, "r" )
    @data = file.readlines
    file.close 
  end

  def create_data_index_hash
    data_idx = 0
    @data.each do |line|
      personal_data = line.gsub( "\"", "" )
      personal_data.gsub!( /\s/, "" )
      personal_items = personal_data.split( "," )
      mons_name = personal_items[ PERSONAL_PARAM::POKENAME ]
      form_name = personal_items[ PERSONAL_PARAM::FORM_NAME ]
      if form_name == nil then
        form_name = ""
      end
      mons_fullname = mons_name + form_name
      @data_index_hash[ mons_fullname ] = data_idx
      data_idx += 1
    end
  end 

  def create_form_name_hash
    @data.each do |line|
      personal_data = line.gsub( "\"", "" )
      personal_data.gsub!( /\s/, "" )
      personal_items = personal_data.split( "," )
      mons_name = personal_items[ PERSONAL_PARAM::POKENAME ]
      form_name = personal_items[ PERSONAL_PARAM::FORM_NAME ]
      if form_name == nil then
        form_name = ""
      end
      if @form_name_hash.has_key?( mons_name ) == false then
        @form_name_hash[ mons_name ] = Array.new
      end
      @form_name_hash[ mons_name ] << form_name
    end
  end

  def get_data_index( mons_fullname )
    data_index = @data_index_hash[ mons_fullname ]
    return data_index
  end

  def get_personal_data( mons_fullname )
    data_index = get_data_index( mons_fullname )
    personal_data = @data[ data_index ]
    return personal_data
  end

  def get_personal_items( mons_fullname )
    personal_data = get_personal_data( mons_fullname )
    copy_data = personal_data.gsub( "\"", "" )
    copy_data.gsub!( /\s/, "" )
    personal_items = copy_data.split( "," )
    return personal_items
  end

  def get_form_name_list( mons_name )
    form_name_list = @form_name_hash[ mons_name ]
    return form_name_list
  end

  def get_mons_fullname_list
    fullname_list = Array.new 
    $monsname.each do |mons_name| 
      if mons_name == "－－－－－" then next end
      form_list = get_form_name_list( mons_name )
      form_list.each do |form_name|
        fullname = mons_name + form_name
        fullname_list << fullname
      end
    end
    return fullname_list
  end

  def convert_to_mons_fullname_list( mons_name )
    fullname_list = Array.new
    form_list = get_form_name_list( mons_name )
    if form_list == nil then
      fullname_list << mons_name
    else
      form_list.each do |form_name|
        fullname = mons_name + form_name
        fullname_list << fullname
      end
    end
    return fullname_list
  end

  def get_mons_name( mons_fullname )
    return mons_fullname.gsub( /[a-z]/, "" )
  end

  def get_gender_type( mons_fullname )
    personal_items = get_personal_items( mons_fullname )
    gender_type = personal_items[ PERSONAL_PARAM::GENDER ].to_i
    if gender_type != GENDER_TYPE_MALE_ONLY &&
       gender_type != GENDER_TYPE_FEMALE_ONLY &&
       gender_type != GENDER_TYPE_UNKNOWN then
      gender_type = GENDER_TYPE_NORMAL
    end
    return gender_type
  end

  def get_egg_group_list( mons_fullname )
    egg_group_list = Array.new
    egg_group_list << get_egg_group1( mons_fullname )
    egg_group_list << get_egg_group2( mons_fullname )
    return egg_group_list
  end

  def get_egg_group1( mons_fullname )
    personal_items = get_personal_items( mons_fullname )
    egg_group_idx = personal_items[ PERSONAL_PARAM::EGG_GROUP1 ].to_i
    egg_group = $egg_group_list[ egg_group_idx ]
    return egg_group
  end

  def get_egg_group2( mons_fullname )
    personal_items = get_personal_items( mons_fullname )
    egg_group_idx = personal_items[ PERSONAL_PARAM::EGG_GROUP2 ].to_i
    egg_group = $egg_group_list[ egg_group_idx ]
    return egg_group
  end

  def get_evolve_poke_list( mons_fullname )
    evolve_poke_list = Array.new
    personal_items = get_personal_items( mons_fullname )
    PERSONAL_PARAM::SHINKA_POKE1.upto( PERSONAL_PARAM::SHINKA_POKE7 ) do |item_idx|
      ev_poke_name = personal_items[ item_idx ]
      if ev_poke_name != "" then
        ev_poke_fullname_list = convert_to_mons_fullname_list( ev_poke_name )
        evolve_poke_list += ev_poke_fullname_list
      end
    end
    evolve_poke_list.uniq! # 重複を排除
    return evolve_poke_list
  end

  def get_level_waza_list( mons_fullname )
    personal_items = get_personal_items( mons_fullname )
    waza_list = Array.new
    PERSONAL_PARAM::WAZA1.upto( PERSONAL_PARAM::WAZA25 ) do |item_idx|
      waza_name = personal_items[ item_idx ]
      if waza_name != "" then 
        waza_list << waza_name
      end
    end
    return waza_list
  end

  def get_level_waza_learning_level_list( mons_fullname )
    personal_items = get_personal_items( mons_fullname )
    waza_level_list = Array.new
    PERSONAL_PARAM::WAZA_LV1.upto( PERSONAL_PARAM::WAZA_LV25 ) do |item_idx|
      waza_level = personal_items[ item_idx ]
      if waza_level != "" then
        waza_level_list << waza_level.to_i
      end
    end
    return waza_level_list
  end

  def get_usable_waza_machine_list( mons_fullname )
    usable_machine_list = Array.new
    personal_items = get_personal_items( mons_fullname )
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

  def get_teach_waza_list( mons_fullname )
    teach_waza_list = Array.new
    personal_items = get_personal_items( mons_fullname )
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

  def get_tokusei_list( mons_fullname )
    personal_items = get_personal_items( mons_fullname )
    tokusei_list = Array.new
    tokusei_list << personal_items[ PERSONAL_PARAM::SPEABI1 ]
    tokusei_list << personal_items[ PERSONAL_PARAM::SPEABI2 ]
    tokusei_list << personal_items[ PERSONAL_PARAM::SPEABI3 ]
    return tokusei_list
  end

end 
