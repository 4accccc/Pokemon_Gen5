# @brief  パーソナルデータから, ポケモン名 => [フォーム名] のハッシュを作成する
# @file   form_name_hash_conv.rb
# @author obata
# @date   2010.06.02

require "personal_parser.rb" 
require "monsno_hash.rb"

def OutputFormNameHash( personal_filename, output_path )

  personal_parser = PersonalDataParser.new( personal_filename )

  out_data = Array.new 
  out_data << "$form_name_hash = {"

  $monsname.each do |mons_name|
    if mons_name == "－－－－－" then next end
    out_data << "\t\"#{mons_name}\" => [" 
    form_list = personal_parser.get_form_name_list( mons_name )
    form_list.each do |form_name|
      out_data << "\t\t\"#{form_name}\","
    end
    out_data << "\t],"
  end

  out_data << "}" 

  # 出力
  output_filename = output_path + "form_name_hash.rb"
  output_file = File::open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end 
