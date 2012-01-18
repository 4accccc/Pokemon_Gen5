# @brief  BWで覚えていたレベル技をSWANでも覚えるかチェック
# @file   level_waza_check.rb
# @author onoue
# @date   2012.01.18

require "mons_fullname_list.rb"
require "levelwaza_hash.rb"
require "wb_levelwaza_hash.rb"
require "machinewaza_hash.rb"
require "machine_wazaname_hash.rb"

machine_wazaname_list = $machine_wazaname_hash.values

out_data = Array.new
$mons_fullname_list.each do |mons_name|

  # WBから存在するポケモンのみチェック
  if $wb_levelwaza_hash.key?( mons_name ) then
    wb_levelwaza_list = $wb_levelwaza_hash[ mons_name ]
    levelwaza_list = $levelwaza_hash[ mons_name ]
    wb_levelwaza_list.each do |wb_levelwaza|
      # SWANに含まれているかチェック
      unless levelwaza_list.include?(wb_levelwaza) then
        # 含まれていなかったらログを出力
        out_data << "#{mons_name} => #{wb_levelwaza} ×\n"
      end
    end
  end
end

# 出力
out_file = File::open( "../result_lv_waza.txt", "w" )
out_file.puts( "！==========================================================!")
out_file.puts( "！このファイルはlevel_waza_check.rbが作成したファイルです" )
out_file.puts( "！BWで覚えていたレベル技をSWANでも覚えるかチェックして出力しています" )
out_file.puts( "！==========================================================!")
out_file.puts( out_data )
out_file.close 
