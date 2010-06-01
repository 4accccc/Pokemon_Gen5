# @brief  パーソナルデータから, ポケモン名 ==> [習得Lv] のハッシュを作成する
# @file   wazaoboe_lv_hash_conv.rb
# @author obata
# @date   2010.05.26 

require "personal_parser.rb" 
require "monsno_hash.rb"
require "wazaoboe_hash.rb"


def CreateWazaOboeLvHash( personal_filename, output_path )

  personal_parser = PersonalDataParser.new
  personal_parser.load_personal_file( personal_filename )

  out_data = Array.new 
  out_data << "$wazaoboe_lv_hash = {"

  $monsname.each do |mons_name| 
    if mons_name == "－－－－－" then
      next
    end 

    waza_list = personal_parser.get_levelup_waza_list( mons_name )
    level_list = personal_parser.get_levelup_wazalevel_list( mons_name )

    out_data << "\t\"#{mons_name}\" => ["
    level_list.each do |level|
      out_data << "\t\t#{level},"
    end
    out_data << "\t],"
  end

  out_data << "}" 

  # 出力
  output_filename = output_path + "wazaoboe_lv_hash.rb"
  output_file = File::open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end
