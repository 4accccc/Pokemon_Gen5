# @brief  ポケモン名 → タマゴグループ のハッシュテーブルを生成する
# @file   egg_group_hash_conv.rb
# @author obata
# @date   2010.06.01

require "personal_parser.rb"
require "monsno_hash.rb"


def OutputTokuseiHash( personal_filename, output_path )

  personal_parser = PersonalDataParser.new( personal_filename )
  mons_fullname_list = personal_parser.get_mons_fullname_list

  out_data = Array.new
  out_data << "$tokusei_hash = {"

  mons_fullname_list.each do |mons_fullname|
    out_data << "\t\"#{mons_fullname}\" => ["
    tokusei_list = personal_parser.get_tokusei_list( mons_fullname )
    tokusei_list.each do |tokusei|
      out_data << "\t\t\"#{tokusei}\","
    end
    out_data << "\t],"
  end

  out_data << "}"

  # 出力
  output_filename = output_path + "tokusei_hash.rb"
  output_file = File.open( output_filename, "w" )
  out_data.each do |data|
    output_file.puts( data )
  end
  output_file.close

end
