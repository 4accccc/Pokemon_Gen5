##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
##
##
##	@file		height_data_cnv.rb
##	@brief  ドラゴンジム高さデータコンバータ
##	@author	Masayuki Onoue
##	@date		2011.06.17
##
##
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"

NO_INDEX  = 0
X_INDEX   = 1
Y_INDEX   = 2
XW_INDEX  = 3
ZW_INDEX  = 4
H_INDEX   = 5

DATA_HEADER = <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//   @file		%s
//   @brief   ドラゴンジム用拡張高さデータテーブル
//   @author	Masayuki Onoue
//   @date		2011.06.17
//   @note    /resource/fldmapdata/gimmick/gym_dragon/height_data_cnv.rbが出力しています
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//-------------------------------------
///	拡張高さデータ型
//=====================================
typedef struct {
  u8 x;
  u8 z;
  u8 xw;
  u8 zw;
  fx32 h;
}GYM_DRAGON_EX_HEIGHT_DATA;

static const GYM_DRAGON_EX_HEIGHT_DATA sc_ex_height_data[] =
{
EOS

##================================
# main
##================================
data_name = ARGV[0]
out_data_name = ARGV[1]

data = CSV.readlines(data_name)
out_file = open( out_data_name ,"w" )

# データテーブルの作成
out_file.puts DATA_HEADER%[out_data_name]

data.each_with_index{|line,i|
  if i == 0 then next end
  if line.include?("#END") == TRUE then break end
  x = line[X_INDEX].to_i
  y = line[Y_INDEX].to_i
  xw = line[XW_INDEX].to_i
  zw = line[ZW_INDEX].to_i
  h = line[H_INDEX].to_i*16
  out_file.puts "  { %d ,%d ,%d ,%d ,FX32_CONST(%d) },"%[x,y,xw,zw,h]
}
out_file.puts "};"
out_file.puts "#define EX_HEIGHT_DATA_NUM  (NELEMS(sc_ex_height_data))"

out_file.close


