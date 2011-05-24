##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
##
##
##	@file		animal_trail_pos_data_cnv.rb
##	@brief  けものみち位置データコンバータ
##	@author	Masayuki Onoue
##	@data		2011.05.24
##
##
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require File.dirname(__FILE__) + '/rbdef.rb'
require "csv"

##================================
# main
##================================
dir_name = ARGV[0]
in_data_name = ARGV[1]
data_cnt = ARGV[2].to_i
out_data_name = ARGV[3]

puts dir_name+"/"+in_data_name
data = CSV.readlines(dir_name+"/"+in_data_name)

out_file = open( out_data_name ,"w" )
out_file.puts $pos_data_header % [out_data_name,data_cnt]
data_cnt.times{|index|
  out_file.printf "  { "
  out_file.printf data[index+TITLE_CNT][ZONE_ID_INDEX].to_s + " ,"
  out_file.printf data[index+TITLE_CNT][X_INDEX].to_s + " ,"
  out_file.printf data[index+TITLE_CNT][Y_INDEX].to_s + " ,"
  out_file.printf data[index+TITLE_CNT][Z_INDEX].to_s + " ,"
  out_file.puts "}, // %d" % (index + 1)
}
out_file.puts "};"
out_file.close


