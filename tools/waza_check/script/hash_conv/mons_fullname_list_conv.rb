# @brief  パーソナルデータから, ポケモン名+フォーム名のリストを生成する
# @file   mons_fullname_list_conv.rb
# @author obata
# @date   2010.06.02

require "personal_parser.rb" 
require "monsno_hash.rb"


def OutputMonsFullNameList( personal_filename, output_path )

  personal_parser = PersonalDataParser.new( personal_filename )
  fullname_list = personal_parser.get_mons_fullname_list

  out_data = Array.new 
  out_data << "$mons_fullname_list = ["
  fullname_list.each do |fullname|
    out_data << "\t\"#{fullname}\","
  end
  out_data << "]" 

  # 出力
  output_filename = output_path + "mons_fullname_list.rb"
  output_file = File::open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end

