##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
##
##
##	@file		def_data_cnv.rb
##	@brief  ドラゴンジム定義データコンバータ
##	@author	Masayuki Onoue
##	@date		2011.06.23
##
##
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"

DEF_NAME_INDEX  = 0
DATA_INDEX      = 1
COMMENT_INDEX   = 2

HEADER = <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//   @file		%s
//   @brief   ドラゴンジム定義データ
//   @author	Masayuki Onoue
//   @date		2011.06.23
//   @note    /resource/fldmapdata/gimmick/gym_dragon/data_cnv.rbが出力しています
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

EOS

##================================
# main
##================================
data_name = ARGV[0]
out_data_name = ARGV[1]

data = CSV.readlines(data_name)
out_file = open( out_data_name ,"w" )

out_file.puts HEADER%[out_data_name]

data.each_with_index{|data,index|
  if index == 0 then next end
  out_file.puts "#define  %s (%d) // %s"%[data[DEF_NAME_INDEX].to_s,data[DATA_INDEX].to_i,data[COMMENT_INDEX]]
}

out_file.close



