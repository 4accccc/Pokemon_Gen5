# @brief  レベルアップで覚えるのに同技の技マシンが使えない不整合を検出する
# @file   wazamachine_check.rb
# @author obata
# @date   2010.06.28

require "mons_fullname_list.rb"
require "levelwaza_hash.rb"
require "machinewaza_hash.rb"
require "machine_wazaname_hash.rb"

machine_wazaname_list = $machine_wazaname_hash.values

out_data = Array.new
$mons_fullname_list.each do |mons_name|
  levelwaza_list = $levelwaza_hash[ mons_name ]
  machinewaza_list = $machinewaza_hash[ mons_name ]
  levelwaza_list.each do |levelwaza|
    if machine_wazaname_list.include?( levelwaza ) == true && 
       machinewaza_list.include?( levelwaza ) == false then
      puts "#{mons_name}: #{levelwaza}"
      out_data << "#{mons_name}: #{levelwaza}"
    end
  end
end

# 出力
out_file = File::open( "../result_wazamachine.txt", "w" )
out_file.puts( "！==========================================================!")
out_file.puts( "！このファイルはwazamachine_check.rbが作成したファイルです" )
out_file.puts( "！レベルアップで覚えるのに同技の技マシンが使えない不整合をチェックして出力しています" )
out_file.puts( "！==========================================================!")
out_file.puts( out_data )
out_file.close 
