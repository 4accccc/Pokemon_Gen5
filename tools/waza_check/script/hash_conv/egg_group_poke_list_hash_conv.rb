# @brief  子作りグループ → ポケモンリスト のハッシュテーブルを生成する
# @file   egg_group_poke_list_hash_conv.rb
# @author obata
# @date   2010.06.01

require "personal_parser.rb"
require "monsno_hash.rb"


def OutputEggGroupPokeListHash( personal_filename, output_path )

  personal_parser = PersonalDataParser.new( personal_filename )
  mons_fullname_list = personal_parser.get_mons_fullname_list

  # 空のハッシュを作成
  egg_group_poke_list_hash = Hash.new
  $egg_group_list.each do |egg_group_name|
    egg_group_poke_list_hash[ egg_group_name ] = Array.new
  end

  # ポケモン名をハッシュに割り振る
  mons_fullname_list.each do |mons_fullname| 
    egg_group1 = personal_parser.get_egg_group1( mons_fullname )
    egg_group2 = personal_parser.get_egg_group2( mons_fullname ) 
    egg_group_poke_list_hash[ egg_group1 ] << mons_fullname
    if egg_group1 != egg_group2 then
      egg_group_poke_list_hash[ egg_group2 ] << mons_fullname
    end
  end

  # 出力データを作成
  out_data = Array.new
  out_data << "$egg_group_poke_list_hash = {"
  $egg_group_list.each do |egg_group_name|
    poke_list = egg_group_poke_list_hash[ egg_group_name ]
    out_data << "\t\"#{egg_group_name}\" => ["
    poke_list.each do |mons_fullname|
      out_data << "\t\t\"#{mons_fullname}\","
    end
    out_data << "\t],"
  end
  out_data << "}"
  
  # 出力
  output_filename = output_path + "egg_group_poke_list_hash.rb"
  output_file = File.open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end
