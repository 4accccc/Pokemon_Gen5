# @brief  パーソナルデータから, 進化ハッシュテーブルを生成する
# @file   evolve_hash_conv.rb
# @author obata
# @date   2010.05.27

require "personal_parser.rb"


# 順ハッシュ作成
def CreateEvolveHash( personal_filename, output_path ) 
  evolve_poke_hash = CreateEvolvePokeHash_from_PersonalData( personal_filename ) 
  out_data = CreateOutputData_of_EvolveHash( evolve_poke_hash, "evolve_poke_hash" ) 
  OutputHashData_to_File( out_data, output_path, "evolve_poke_hash.rb" ) 
end

# 逆ハッシュ作成
def CreateReverseEvolveHash( personal_filename, output_path ) 
  reverse_evolve_poke_hash = CreateReverseEvolvePokeHash_from_PersonalData( personal_filename )
  out_data = CreateOutputData_of_EvolveHash( reverse_evolve_poke_hash, "reverse_evolve_poke_hash" ) 
  OutputHashData_to_File( out_data, output_path, "reverse_evolve_poke_hash.rb" ) 
end


def CreateEvolvePokeHash_from_PersonalData( personal_filename ) 
  personal_parser = PersonalDataParser.new
  personal_parser.load_personal_file( personal_filename )

  evolve_poke_hash = CreateNullHash_for_EvolvePokeHash()

  $monsname.each do |mons_name| 
    evolve_poke_list = personal_parser.get_evolve_poke_list( mons_name )
    evolve_poke_list.each do |evolve_mons_name|
      evolve_poke_hash[ mons_name ] << evolve_mons_name
    end
  end

  return evolve_poke_hash
end

def CreateReverseEvolvePokeHash_from_PersonalData( personal_filename ) 
  personal_parser = PersonalDataParser.new
  personal_parser.load_personal_file( personal_filename )

  reverse_evolve_poke_hash = CreateNullHash_for_EvolvePokeHash()

  $monsname.each do |mons_name| 
    evolve_poke_list = personal_parser.get_evolve_poke_list( mons_name )
    evolve_poke_list.each do |evolve_mons_name|
      reverse_evolve_poke_hash[ evolve_mons_name ] << mons_name
    end
  end

  return reverse_evolve_poke_hash
end

def CreateNullHash_for_EvolvePokeHash
  hash = Hash.new
  $monsname.each do |mons_name| 
    hash[ mons_name ] = Array.new
  end
  return hash
end

def CreateOutputData_of_EvolveHash( hash, hash_name )
  out_data = Array.new
  out_data << "$#{hash_name} = {" 

  $monsname.each do |mons_name| 
    out_data << "\t\"#{mons_name}\"=>[" 

    evolve_poke_list = hash[ mons_name ] 
    evolve_poke_list.each do |evolve_mons_name|
      out_data << "\t\t\"#{evolve_mons_name}\","
    end
    out_data << "\t],"
  end 
  out_data << "}"

  return out_data
end

def OutputHashData_to_File( out_data, output_path, filename )
  output_filename = output_path + filename
  output_file = File::open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close
end
