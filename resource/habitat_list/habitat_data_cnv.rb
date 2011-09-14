#//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#/**
# *
# *	@file		habitat_data_cnv.rb
# *	@brief  生息地リストデータコンバート
# *	@author	Masayuki Onoue
# *	@date		2011.09.04
# *
# */
#//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require File.dirname(__FILE__) + '/habitat_data_index.rb'
require "csv"

# 半角文字チェック
def string_check( str ,check_w )
  if check_w =~ str
    raise "不正文字が含まれれています->%s"%[str]
  end
end

OUT_FNAME = ARGV[0]
OUT_H_FNAME = ARGV[1]
DATA_FNAME = ARGV[2]

SYS_FLAG_INDEX    = 1
ZONE_GROUP_INDEX  = 2
COMMENT_INDEX     = 3

out_file = open(OUT_FNAME,"w")
out_h_file = open(OUT_H_FNAME,"w")
data_file = CSV.open(DATA_FNAME,"r")

header =<<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  生息地リスト用エンカウント解析データアーカイブインデックス
 *	@author	Masayuki Onoue
 *	@date		2011.09.04
 *	@note		コンバータが出力したファイルです。（図鑑データコンバート時に出力）
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

//-------------------------------------
///	生息地リスト用データ構造体
//=====================================
typedef struct {
  u16 sys_flag_id;
  u16 zone_group_id;
  u8  arc_idx;
}HABITAT_LIST_DATA;

//-------------------------------------
///	生息地リスト用データ
//=====================================
static const HABITAT_LIST_DATA sc_habitat_list_data[] =
{
EOS

# ヘッダ部分出力
out_file.puts header%[OUT_FNAME]

# テーブルサイズ初期化
table_lng = 0

# データの読み込み
data_file.shift
data_file.each{|line|
  # 終端文字検索
  if line.include?("#END") then break end
  flg_name    = line[SYS_FLAG_INDEX]
  group_name  = line[ZONE_GROUP_INDEX]

  # 不正文字チェック
  string_check( flg_name ,/[^0-9a-zA-Z_]/ )
  string_check( group_name ,/[^0-9a-zA-Z_]/ )

  # アーカイブID取得
  if $habitat_arc_idx_table.key?(group_name) then
    arc_idx     = $habitat_arc_idx_table[group_name]
  else
    raise puts "不明なゾーングループ=>%s\n"%[group_name]
  end

  $habitat_arc_idx_table.delete(group_name)

  out_file.puts "  {%s,%s,%d},  // %s"%[ flg_name,group_name,arc_idx,line[COMMENT_INDEX]]
  table_lng += 1
}

# 生息地リストに載っていない可能性のあるものリスト
$habitat_arc_idx_table.each_key{|group_name|
  puts group_name
}

out_file.puts "};"



header =<<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		%s
 *	@brief  生息地リストデータファイルヘッダ
 *	@author	Masayuki Onoue
 *	@date		2011.09.14
 *	@note		コンバータが出力したファイルです。
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
EOS

out_h_file.puts header%[OUT_H_FNAME]
out_h_file.puts "\n// 生息地リストデータ数"
out_h_file.puts "#define HABITAT_LIST_DATA_MAX (%d)"%[table_lng]

out_h_file.close
out_file.close
