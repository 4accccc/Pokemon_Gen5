# @brief  パーソナルデータから, ポケモン名 => [習得技] のハッシュを作成する
# @file   levelwaza_hash_conv.rb
# @author obata
# @date   2010.05.26 

require "personal_parser.rb" 
require "monsno_hash.rb"


def OutputLevelWazaHash_form_PersonalData( personal_filename, output_path )

  personal_parser = PersonalDataParser.new( personal_filename )
  mons_fullname_list = personal_parser.get_mons_fullname_list

  out_data = Array.new 
  out_data << "$levelwaza_hash = {"

  mons_fullname_list.each do |mons_fullname| 
    out_data << "\t\"#{mons_fullname}\" => [" 
    waza_list = personal_parser.get_level_waza_list( mons_fullname )
    waza_list.each do |waza_name|
      out_data << "\t\t\"#{waza_name}\","
    end
    out_data << "\t],"
  end

  out_data << "}" 

  # 出力
  output_filename = output_path + "levelwaza_hash.rb"
  output_file = File::open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end

