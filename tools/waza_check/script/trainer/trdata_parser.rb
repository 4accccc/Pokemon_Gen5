# @brief  トレーナーデータの解析
# @file   trdata_parser.rb
# @author obata
# @date   2010.05.28 

require "trdata_index.rb"
require "trainer.rb"
require "monsname_converter.rb"

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
    if mons_name != nil && mons_name != "" then poke_num = poke_num + 1 end
  end
  return poke_num
end

def GetMonsName( trdata_items, poke_idx )
  item_idx = PARA::POKE1_NAME + poke_idx
  mons_name = trdata_items[ item_idx ]
  mons_name = ConvertMonsName( mons_name )
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
    if waza_name != nil && waza_name != "" then waza_list << waza_name end
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

def GetTrainerList( filename )

  trdata_file = File::open( filename, "r" )
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
