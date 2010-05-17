####################################################################################
#
# @brief  存在しないゾーンのスクリプトメッセージデータをリストアップする
# @file   needless_message_listup.rb
# @author obata
# @date   2010.05.17
#
####################################################################################

ev_file_list = Array.new
gmm_file_list = Array.new
needless_file_list = Array.new 

Dir[ "../../../*.ev" ].each do |path|
  filename = File::basename( path )
  ev_file_list << filename
end

Dir[ "../../../../../message/src/script_message/*.gmm" ].each do |path|
  filename = File::basename( path )
  gmm_file_list << filename
end

gmm_file_list.each do |gmm_filename|
  gmm_zone_id = File::basename( gmm_filename, ".gmm" )
  target_filename = gmm_zone_id + ".ev"
  if ev_file_list.include?( target_filename ) == false then
    needless_file_list << gmm_filename
  end
end

puts needless_file_list
