# @brief  トレーナーデータの解析
# @file   trdata_parser.rb
# @author obata
# @date   2010.05.28 

require "constant.rb"
require "trainer.rb"


class PARA
  enum_const_set %w[
    FIGHT_TYPE
    DATA_TYPE
    GOLD
    GENDER
    TR_ID
    TR_TYPE
    TR_NAME
    POKE1_POW
    POKE1_LV
    POKE1_GENDER
    POKE1_FORM
    POKE2_POW
    POKE2_LV
    POKE2_GENDER
    POKE2_FORM
    POKE3_POW
    POKE3_LV
    POKE3_GENDER
    POKE3_FORM
    POKE4_POW
    POKE4_LV
    POKE4_GENDER
    POKE4_FORM
    POKE5_POW
    POKE5_LV
    POKE5_GENDER
    POKE5_FORM
    POKE6_POW
    POKE6_LV
    POKE6_GENDER
    POKE6_FORM
    AI
    POKE1_ITEM
    POKE2_ITEM
    POKE3_ITEM
    POKE4_ITEM
    POKE5_ITEM
    POKE6_ITEM
    USE_ITEM1
    USE_ITEM2
    USE_ITEM3
    USE_ITEM4
    POKE1_WAZA1
    POKE1_WAZA2
    POKE1_WAZA3
    POKE1_WAZA4
    POKE2_WAZA1
    POKE2_WAZA2
    POKE2_WAZA3
    POKE2_WAZA4
    POKE3_WAZA1
    POKE3_WAZA2
    POKE3_WAZA3
    POKE3_WAZA4
    POKE4_WAZA1
    POKE4_WAZA2
    POKE4_WAZA3
    POKE4_WAZA4
    POKE5_WAZA1
    POKE5_WAZA2
    POKE5_WAZA3
    POKE5_WAZA4
    POKE6_WAZA1
    POKE6_WAZA2
    POKE6_WAZA3
    POKE6_WAZA4
    POKE1_NAME
    POKE2_NAME
    POKE3_NAME
    POKE4_NAME
    POKE5_NAME
    POKE6_NAME
    HP_RECOVER
    GIFT_ITEM
    POKE1_SPEABI
    POKE2_SPEABI
    POKE3_SPEABI
    POKE4_SPEABI
    POKE5_SPEABI
    POKE6_SPEABI
    MAX
  ]
end


MAX_POKEMON_NUM = 6
MAX_WAZA_NUM = 4

# 技が指定されているデータタイプ
$waza_data_type_list = [
  "技",
  "マルチ",
]


# module ###################################################################

def GetTrainerDataItems( trdata )
  trdata.gsub!( "\"", "" )
  trdata.gsub!( /\s/, "" )
  trdata_items = trdata.split( "," ) 
  return trdata_items
end

def GetDataType( trdata_items )
  data_type = trdata_items[ PARA::DATA_TYPE ]
  return data_type
end

def GetTrainer( trdata_items ) 
  trainer = Trainer.new
  trainer.set_name( trdata_items[ PARA::TR_NAME ] )
  trainer.set_type( trdata_items[ PARA::TR_TYPE ] )
  return trainer
end

def GetPokemonNum( trdata_items )
  poke_num = 0
  0.upto( MAX_POKEMON_NUM - 1 ) do |poke_idx|
    mons_name = trdata_items[ PARA::POKE1_NAME + poke_idx ]
    if mons_name != nil then poke_num = poke_num + 1 end
  end
  return poke_num
end

def GetMonsName( trdata_items, poke_idx )
  item_idx = PARA::POKE1_NAME + poke_idx
  mons_name = trdata_items[ item_idx ]
  return mons_name
end

def GetMonsLevel( trdata_items, poke_idx )
  item_idx = PARA::POKE1_LV + ( poke_idx * 4 )
  level = trdata_items[ item_idx ].to_i
  return level
end

def GetWazaName( trdata_items, poke_idx, waza_idx )
  item_idx = PARA::POKE1_WAZA1 + ( poke_idx * 4 ) + waza_idx
  waza_name = trdata_items[ item_idx ]
  return waza_name
end

def GetWazaList( trdata_items, poke_idx )
  waza_list = Array.new
  0.upto( MAX_WAZA_NUM - 1 ) do |waza_idx|
    waza_name = GetWazaName( trdata_items, poke_idx, waza_idx )
    if waza_name != nil then waza_list << waza_name end
  end
  return waza_list
end

def GetPokemon( trdata_items, poke_idx )
  data_type = GetDataType( trdata_items ) 
  mons_name = GetMonsName( trdata_items, poke_idx )
  mons_level = GetMonsLevel( trdata_items, poke_idx )

  pokemon = Pokemon.new
  pokemon.set_mons_name( mons_name )
  pokemon.set_level( mons_level )

  if $waza_data_type_list.include?( data_type ) then
    waza_list = GetWazaList( trdata_items, poke_idx )
    pokemon.set_waza_list( waza_list )
  else
    pokemon.setup_default_waza
  end
  return pokemon
end


# main #####################################################################

def GetTrainerList

  trdata_file = File::open( "trainer_wb.csv", "r" )
  trdata_lines = trdata_file.readlines
  trdata_file.close

  trainer_list = Array.new

  trdata_lines.each do |trdata|
    trdata_items = GetTrainerDataItems( trdata )
    trainer = GetTrainer( trdata_items )
    pokemon_num = GetPokemonNum( trdata_items )

    0.upto( pokemon_num - 1 ) do |poke_idx| 
      pokemon = GetPokemon( trdata_items, poke_idx )
      trainer.add_pokemon( pokemon )
    end

    trainer_list << trainer
  end

  return trainer_list

end
