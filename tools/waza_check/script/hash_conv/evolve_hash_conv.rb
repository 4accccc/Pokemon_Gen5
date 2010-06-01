# @brief  パーソナルデータから, 進化ハッシュテーブルを生成する
# @file   evolve_hash_conv.rb
# @author obata
# @date   2010.05.27

require "personal_index.rb"


def CreateEvolveHash( personal_filename, output_path )

  personal_parser = PersonalDataParser.new
  personal_parser.load_personal_file( personal_filename )

  out_data = Array.new
  out_data << "$evolve_poke_hash = {"

  $monsname.each do |mons_name| 
    if mons_name == "－－－－－" then
      next
    end 

    evolve_poke_list = personal_parser.get_evolve_poke_list( mons_name )

    out_data << "\t\"#{mons_name}\"=>["
    evolve_poke_list.each do |ev_poke|
      out_data << "\t\t\"#{ev_poke}\","
    end
    out_data << "\t],"
  end

  out_data << "}"

  # 出力
  output_filename = output_path + "evolve_poke_hash.rb"
  output_file = File::open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end
