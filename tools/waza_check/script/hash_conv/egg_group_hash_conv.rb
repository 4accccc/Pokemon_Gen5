# @brief  ポケモン名 → タマゴグループ のハッシュテーブルを生成する
# @file   egg_group_hash_conv.rb
# @author obata
# @date   2010.06.01

require "personal_parser.rb"
require "monsno_hash.rb"


def CreateEggGroupHash( personal_filename, output_path )

  personal_parser = PersonalDataParser.new
  personal_parser.load_personal_file( personal_filename )

  out_data = Array.new
  out_data << "$egg_group_hash = {"

  $monsname.each do |mons_name|
    egg_group1 = personal_parser.get_egg_group1( mons_name )
    egg_group2 = personal_parser.get_egg_group2( mons_name )
    out_data << "\t\"#{mons_name}\" => [ \"#{egg_group1}\", \"#{egg_group2}\" ],"
  end

  out_data << "}"

  # 出力
  output_filename = output_path + "egg_group_hash.rb"
  output_file = File.open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end
