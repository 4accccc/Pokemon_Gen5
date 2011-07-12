##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
##
##
##	@file		def_data_cnv.rb
##	@brief  バッジチェックゲート定義データ
##	@author	Masayuki Onoue
##	@date		2011.07.11
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
//   @brief   バッジチェックゲート定義データ
//   @author	Masayuki Onoue
//   @date		2011.07.11
//   @note    /resource/fldmapdata/gimmick/badge_gate/def_data_cnv.rbが出力しています
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
  if data.include?("#END") then break end
  out_file.puts "#define  %s (%d) // %s"%[data[DEF_NAME_INDEX].to_s,data[DATA_INDEX].to_i,data[COMMENT_INDEX]]
}

out_file.close



