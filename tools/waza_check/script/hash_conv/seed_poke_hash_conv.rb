# @brief  種ポケモンハッシュを生成する
# @file   seed_poke_hash_conv.rb
# @author obata
# @date   2010.06.01

require "monsno_hash.rb"
require "evolve_hash_conv.rb"


def UpdateSeedPokeHash( personal_filename, output_path )
  seed_poke_hash = CreateSeedPokeHash_from_PersonalData( personal_filename )
  out_data = CreateOutputData_of_SeedPokeHash( seed_poke_hash )
  OutputSeedPokeHashData_to_File( out_data, output_path, "seed_poke_hash.rb" )
end

def CreateSeedPokeHash_from_PersonalData( personal_filename )
  seed_poke_hash = CreateNullHash_for_SeedPokeHash()
  reverse_evolve_hash = CreateReverseEvolvePokeHash_from_PersonalData( personal_filename )
  $monsname.each do |mons_name|
    seed_mons_name = GetSeedPokemon( reverse_evolve_hash, mons_name )
    seed_poke_hash[ mons_name ] = seed_mons_name
  end
  return seed_poke_hash
end

def GetSeedPokemon( reverse_evolve_hash, mons_name )
  seed_mons_name = GetPrevEvolvePokemon( reverse_evolve_hash, mons_name )
  if seed_mons_name == nil || seed_mons_name == mons_name then
    return mons_name
  else
    return GetSeedPokemon( reverse_evolve_hash, seed_mons_name )
  end
end

def GetPrevEvolvePokemon( reverse_evolve_hash, mons_name ) 
  prev_mons_list = reverse_evolve_hash[ mons_name ]
  if prev_mons_list == nil then
    return mons_name
  else 
    return prev_mons_list[0]
  end
end

def CreateNullHash_for_SeedPokeHash
  hash = Hash.new
  $monsname.each do |mons_name|
    hash[ mons_name ] = nil
  end
  return hash
end

def CreateOutputData_of_SeedPokeHash( seed_poke_hash )
  out_data = Array.new
  out_data << "$seed_poke_hash = {"
  $monsname.each do |mons_name|
    seed_mons_name = seed_poke_hash[ mons_name ]
    out_data << "\t\"#{mons_name}\" => \"#{seed_mons_name}\","
  end
  out_data << "}"
  return out_data
end

def OutputSeedPokeHashData_to_File( out_data, output_path, filename )
  output_filename = output_path + filename
  output_file = File.open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close
end
