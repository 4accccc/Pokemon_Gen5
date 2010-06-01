# @brief  パーソナルデータから, 逆進化ハッシュテーブルを生成する
# @file   reverse_evolve_hash_conv.rb
# @author obata
# @date   2010.05.27

require "personal_index.rb"


# main #######################################################

# パーソナルデータ取得
personal_file = File::open( "personal_wb.csv", "r" )
personal_lines = personal_file.readlines
personal_file.close

# 空テーブルを作成
reverse_evolve_hash = Hash.new
mons_name_list = Array.new
personal_lines.each do |line|
  line.gsub!( "\"", "" )
  line.gsub!( /\s/, "" )
  items = line.split( "," )
  mons_name = items[ PARA::POKENAME ]
  reverse_evolve_hash[ mons_name ] = Array.new
  mons_name_list << mons_name
end

# テーブルを埋める
personal_lines.each do |line|
  line.gsub!( "\"", "" )
  line.gsub!( /\s/, "" )
  items = line.split( "," )
  mons_name = items[ PARA::POKENAME ]

  evolve_poke_list = Array.new
  PARA::SHINKA_POKE1.upto( PARA::SHINKA_POKE7 ) do |item_idx|
    ev_poke = items[ item_idx ]
    if ev_poke != nil && ev_poke != "" then
      evolve_poke_list << ev_poke
    end
  end 

  evolve_poke_list.each do |ev_poke|
    reverse_evolve_hash[ ev_poke ] << mons_name
  end
end

# 出力
output_file = File::open( "reverse_evolve_poke_hash.rb", "w" )
output_file.puts( "$reverse_evolve_poke_hash = {" )

mons_name_list.each do |mons_name|
  ev_list = reverse_evolve_hash[ mons_name ]
  output_file.puts( "\t\"#{mons_name}\"=>[" )

  if ev_list != nil then
    ev_list.each do |ev_poke|
      output_file.puts( "\t\t\"#{ev_poke}\"," )
    end
  end

  output_file.puts( "\t]," )
end

output_file.puts( "}" ) 


