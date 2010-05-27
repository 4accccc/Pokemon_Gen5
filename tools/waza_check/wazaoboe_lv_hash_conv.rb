# @brief  パーソナルデータから, ポケモン名_技名 ==> 習得Lv の変換ハッシュテーブルを作成する
# @file   wazaoboe_lv_hash_conv.rb
# @author obata
# @date   2010.05.26 

require "personal_index.rb" 


file = File::open( "personal_wb.csv", "r" )
file_lines = file.readlines
file.close

output_file = File::open( "wazaoboe_lv_hash.rb", "w" )
output_file.puts( "$wazaoboe_lv_hash = {" )

file_lines.each do |line|
  line.gsub!( "\"", "" )
  line.gsub!( /\s/, "" )
  items = line.split( "," )
  mons_name = items[ PARA::POKENAME ]
  0.upto( 24 ) do |waza_idx|
    waza_name = items[ PARA::WAZA1 + waza_idx ]
    waza_lv = items[ PARA::WAZA_LV1 + waza_idx ]
    if waza_name == "" then 
      break 
    end # 空欄を発見
    output_file.puts( "  \"#{mons_name}_#{waza_name}\" => #{waza_lv}," )
  end
end

output_file.puts( "}" )
