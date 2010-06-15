# @brief  ポケモン名 → 性別タイプ のハッシュテーブルを生成する
# @file   egg_group_hash_conv.rb
# @author obata
# @date   2010.06.01

require "personal_parser.rb"
require "monsno_hash.rb"


def OutputGenderHash( personal_filename, output_path )

  personal_parser = PersonalDataParser.new( personal_filename )
  mons_fullname_list = personal_parser.get_mons_fullname_list

  out_data = Array.new
  out_data << "$gender_hash = {"

  mons_fullname_list.each do |mons_fullname|
    gender_type = personal_parser.get_gender_type( mons_fullname )
    gender_name = case
                  when gender_type == GENDER_TYPE_MALE_ONLY   then "GENDER_TYPE_MALE_ONLY"
                  when gender_type == GENDER_TYPE_NORMAL      then "GENDER_TYPE_NORMAL"
                  when gender_type == GENDER_TYPE_FEMALE_ONLY then "GENDER_TYPE_FEMALE_ONLY"
                  when gender_type == GENDER_TYPE_UNKNOWN     then "GENDER_TYPE_UNKNOWN"
                  end
    out_data << "\t\"#{mons_fullname}\" => \"#{gender_name}\","
  end

  out_data << "}"

  # 出力
  output_filename = output_path + "gender_hash.rb"
  output_file = File.open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end
