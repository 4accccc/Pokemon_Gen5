# @brief  パーソナルデータから, 教え技のハッシュテーブルを生成する
# @file   teachwaza_hash_conv.rb
# @author obata
# @date   2010.05.27

require "personal_parser.rb"
require "monsno_hash.rb"



# main #######################################################


def OutputTeachWazaHash_from_PersonalData( personal_filename, output_path )

  personal_parser = PersonalDataParser.new( personal_filename )
  mons_fullname_list = personal_parser.get_mons_fullname_list

  out_data = Array.new
  out_data << "$teachwaza_hash = {"

  mons_fullname_list.each do |mons_fullname| 

    out_data << "\t\"#{mons_fullname}\"=>["

    teach_waza_list = personal_parser.get_teach_waza_list( mons_fullname )
    teach_waza_list.each do |waza_name|
      out_data << "\t\t\"#{waza_name}\","
    end

    out_data << "\t],"

  end

  out_data << "}"

  # 出力
  output_filename = output_path + "teachwaza_hash.rb"
  output_file = File::open( output_filename, "w" )
  output_file.puts( out_data )
  output_file.close

end
