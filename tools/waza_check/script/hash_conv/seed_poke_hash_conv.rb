# @brief  種ポケモンハッシュを生成する
# @file   seed_poke_hash_conv.rb
# @author obata
# @date   2010.06.01

require "personal_parser.rb"
require "monsno_hash.rb"
require "evolve_hash_conv.rb"


def OutputSeedPokeHash_from_PersonalData( personal_filename, output_path )
  personal_parser = PersonalDataParser.new( personal_filename )
  seed_poke_hash = CreateSeedPokeHash_from_PersonalData( personal_parser )
  out_data = CreateOutputData_of_SeedPokeHash( personal_parser, seed_poke_hash )
  OutputSeedPokeHashData_to_File( out_data, output_path, "seed_poke_hash.rb" )
end

def CreateSeedPokeHash_from_PersonalData( personal_parser )
  mons_fullname_list = personal_parser.get_mons_fullname_list
  reverse_evolve_poke_hash = CreateReverseEvolvePokeHash_from_PersonalData( personal_parser )

  seed_poke_hash = Hash.new
  mons_fullname_list.each do |mons_fullname|
    seed_poke_hash[ mons_fullname ] = Array.new
  end

  # 第一進化
  mons_fullname_list.each do |mons_fullname|
    prev_mons_list = reverse_evolve_poke_hash[ mons_fullname ]
    prev_mons_list.each do |prev_mons_name|
      if reverse_evolve_poke_hash[ prev_mons_name ].size == 0 then
        seed_poke_hash[ mons_fullname ] << prev_mons_name
      end
    end
    seed_poke_hash[ mons_fullname ].uniq! # 重複を排除
  end

  # 第二進化
  mons_fullname_list.each do |mons_fullname|
    prev_mons_list = reverse_evolve_poke_hash[ mons_fullname ]
    prev_mons_list.each do |prev_mons_name|
      prev_prev_mons_list = reverse_evolve_poke_hash[ prev_mons_name ]
      prev_prev_mons_list.each do |prev_prev_mons_name| 
        if reverse_evolve_poke_hash[ prev_prev_mons_name ].size == 0 then
          seed_poke_hash[ mons_fullname ] << prev_prev_mons_name
        end
      end
    end
    seed_poke_hash[ mons_fullname ].uniq! # 重複を排除
  end

  # 自分自身
  mons_fullname_list.each do |mons_fullname|
    prev_mons_list = reverse_evolve_poke_hash[ mons_fullname ]
    if prev_mons_list.size == 0 then
      mons_name = personal_parser.get_mons_name( mons_fullname )
      fullname_list = personal_parser.convert_to_mons_fullname_list( mons_name )
      fullname_list.each do |seed_poke|
        seed_poke_hash[ mons_fullname ] << seed_poke
      end
    end
  end

  # 特殊ケース
  seed_poke_hash[ "バルビート" ] << "イルミーゼ"
  seed_poke_hash[ "イルミーゼ" ] << "バルビート"
  seed_poke_hash[ "ニドラン♀" ] << "ニドラン♂"
  seed_poke_hash[ "ニドラン♂" ] << "ニドラン♀"
  seed_poke_hash[ "ニドリーノ" ] << "ニドラン♀"
  seed_poke_hash[ "ニドキング" ] << "ニドラン♀"
  seed_poke_hash[ "マナフィ" ] << "フィオネ"

  return seed_poke_hash
end

def CreateOutputData_of_SeedPokeHash( personal_parser, seed_poke_hash )
  out_data = Array.new
  out_data << "$seed_poke_hash = {"
  mons_fullname_list = personal_parser.get_mons_fullname_list
  mons_fullname_list.each do |mons_fullname|
    out_data << "\t\"#{mons_fullname}\" => ["
    seed_mons_list = seed_poke_hash[ mons_fullname ]
    seed_mons_list.each do |seed_mons_name|
      out_data << "\t\t\"#{seed_mons_name}\","
    end
    out_data << "\t],"
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
