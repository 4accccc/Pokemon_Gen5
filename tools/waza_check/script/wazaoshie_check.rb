# @brief  レベルアップで覚えるのに技教えで教えられない不整合を検出する
# @file   wazaoshie_check.rb
# @author Masayuki Onoue
# @date   2011.10.26

require "mons_fullname_list.rb"
require "levelwaza_hash.rb"
require "oshiewaza_hash.rb"
require "waza_oshie_hash.rb"
require "old_oshiewaza_hash.rb"

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

  # 旧バージョンでの教え技をチェック
  if $old_oshiewaza_hash.key?( mons_name ) then
    waza_array = $old_oshiewaza_hash[mons_name]
    waza_array.each{|waza_name|
      if is_oshiewaza( waza_name ) == true then
        if oshiewaza_list.include?( waza_name ) == false then
          puts "#{mons_name}: #{waza_name}"
          out_data << "#{mons_name}: #{waza_name}"
        end
      end
    }
  end

end

# 出力
out_file = File::open( "../result_wazaoshie.txt", "w" )
out_file.puts( "！==========================================================!")
out_file.puts( "！このファイルはwazaoshie_check.rbが作成したファイルです" )
out_file.puts( "！レベルアップで覚えるのに技教えで教えられない不整合をチェックして出力しています" )
out_file.puts( "！またか小作
out_file.puts( "！==========================================================!")
out_file.puts( out_data )
out_file.close 

