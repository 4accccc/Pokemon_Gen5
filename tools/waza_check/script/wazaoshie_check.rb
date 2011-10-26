# @brief  レベルアップで覚えるのに技教えで教えられない不整合を検出する
# @file   wazaoshie_check.rb
# @author Masayuki Onoue
# @date   2011.10.26

require "mons_fullname_list.rb"
require "levelwaza_hash.rb"
require "oshiewaza_hash.rb"
require "waza_oshie_hash.rb"

# 教え技か調べる
def is_oshiewaza( waza_name )
  if $oshie_waza_hash_red.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_blue.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_yellow.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_green.values.include?( waza_name ) then return true end
  return false
end

# メイン
out_data = Array.new
$mons_fullname_list.each do |mons_name|
  levelwaza_list = $levelwaza_hash[ mons_name ]
  oshiewaza_list = $oshiewaza_hash[ mons_name ]
  levelwaza_list.each do |levelwaza|
    if is_oshiewaza( levelwaza ) == true then
      if oshiewaza_list.include?( levelwaza ) == false then
        puts "#{mons_name}: #{levelwaza}"
        out_data << "#{mons_name}: #{levelwaza}"
      end
    end
  end
end

# 出力
out_file = File::open( "../result_wazaoshie.txt", "w" )
out_file.puts( out_data )
out_file.close 

