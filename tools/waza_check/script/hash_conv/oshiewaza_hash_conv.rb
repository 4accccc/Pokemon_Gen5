# @brief  パーソナルデータから, 教え技のハッシュテーブルを生成する
# @file   oshiewaza_hash_conv.rb
# @author obata
# @date   2010.05.27

require "personal_parser.rb"
require "monsno_hash.rb"



# main #######################################################


def CreateOshieWazaHash( personal_filename, output_path )

  personal_parser = PersonalDataParser.new
  personal_parser.load_personal_file( personal_filename )

  out_data = Array.new
  out_data << "$oshiewaza_hash = {"

  $monsname.each do |mons_name| 
    if mons_name == "－－－－－" then
      next
    end 

    out_data << "\t\"#{mons_name}\"=>["

    teach_waza_list = personal_parser.get_teach_waza_list( mons_name )
    teach_waza_list.each do |waza_name|
      out_data << "\t\t\"#{waza_name}\","
    end

    out_data << "\t],"

  end

  out_data << "}"

  # 出力
  output_filename = output_path + "oshiewaza_hash.rb"
  output_file = File::open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( out_data )
  end
  output_file.close

end
