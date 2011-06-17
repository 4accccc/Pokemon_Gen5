##[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
##
##
##	@file		data_cnv.rb
##	@brief  ドラゴンジムデータコンバータ
##	@author	Masayuki Onoue
##	@date		2011.06.16
##
##
##]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"

ID_INDEX        = 0
FILE_NAME_INDEX = 1
SOFS_INDEX      = 2
SFRAME_INDEX    = 3
EFRAME_INDEX    = 4

ID_HEADER = <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//   @file		%s
//   @brief   ドラゴンジムトレース用ICAテーブルID
//   @author	Masayuki Onoue
//   @date		2011.06.16
//   @note    /resource/fldmapdata/gimmick/gym_dragon/data_cnv.rbが出力しています
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
EOS

DATA_HEADER = <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
//
//   @file		%s
//   @brief   ドラゴンジムトレース用データ
//   @author	Masayuki Onoue
//   @date		2011.06.16
//   @note    /resource/fldmapdata/gimmick/gym_dragon/data_cnv.rbが出力しています
//
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//-------------------------------------
///	トレース移動用データ
//=====================================
typedef struct {
  u8  data_index;
  u32 start_frame;
  u32 end_frame;
}GYM_DRAGON_TRACE_DATA;

// =================================================
// 移動用データファイル一覧
// =================================================
static const ARCDATID sc_hero_move_data[] =
{
EOS


##================================
# main
##================================
data_name = ARGV[0]
out_data_name = ARGV[1]
id_data_name = ARGV[2]

data = CSV.readlines(data_name)
out_file = open( out_data_name ,"w" )
id_file = open( id_data_name ,"w" )


# IDHEADERの作成
id_file.puts ID_HEADER%[id_data_name]
data.each_with_index{|line,i|
  if i == 0 then next end
  if line.include?("#END") == TRUE then break end
  id_file.puts "#define "+line[ID_INDEX] + "  ("+(i-1).to_s+")"
}

# データテーブルの作成
arc_name_list = Array.new
out_file.puts DATA_HEADER%[out_data_name]
data.each_with_index{|line,i|
  if i == 0 then next end
  if line.include?("#END") == TRUE then break end
  arc_name_list << line[FILE_NAME_INDEX].gsub(/\.ica/,"")
}
# 書き込み
arc_name_list.uniq!
arc_name_list.each{|name|
  out_file.puts " NARC_gym_dragon_%s_bin,"%[name]
}
out_file.puts "};"
out_file.puts "#define HERO_MOVE_DATA_MAX  (NELEMS(sc_hero_move_data))"

out_file.puts <<EOS

// =================================================
// 移動用データ
// =================================================
static const GYM_DRAGON_TRACE_DATA sc_trace_data[] =
{
EOS
# データの書き込み
data.each_with_index{|line,i|
  if i == 0 then next end
  if line.include?("#END") == TRUE then break end
  ica_name = line[FILE_NAME_INDEX].gsub(/\.ica/,"")
  data_index = arc_name_list.index(ica_name)
  start_frame = line[SOFS_INDEX].to_i + line[SFRAME_INDEX].to_i
  end_frame   = line[SOFS_INDEX].to_i + line[EFRAME_INDEX].to_i
  out_file.puts " { %d ,%d ,%d },"%[data_index,start_frame,end_frame]
}
out_file.puts "};"


id_file.close
out_file.close


