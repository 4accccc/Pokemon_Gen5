# @brief  パーソナルデータから, 各ポケモンが習得可能なおしえ技のハッシュテーブルを作成する
# @file   oshiewaza_hash_conv.rb
# @author Masayuki Onoue
# @date   2011.10.26 

require "personal_parser.rb"


# main ######################################################

def OutputOshieWazaHash_from_PersonalDat( personal_filename, output_path )

  personal_parser = PersonalDataParser.new( personal_filename )
  mons_fullname_list = personal_parser.get_mons_fullname_list

  out_data = Array.new
  out_data << "$oshiewaza_hash = {"

  mons_fullname_list.each do |mons_fullname|

    oshie_list = personal_parser.get_usable_waza_oshie_list( mons_fullname )

    # 出力データ作成
    out_data << "\t\"#{mons_fullname}\"=>["
    oshie_list.each do |waza_name|
      out_data << "\t\t\"#{waza_name}\","
    end
    out_data << "\t],"

  end

  out_data << "}" 

  # 出力
  output_filename = output_path + "oshiewaza_hash.rb"
  output_file = File::open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end

