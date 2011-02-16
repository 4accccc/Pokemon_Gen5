#! ruby -Ks
#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#**
#*
#*	@file		makelist_sort_pocket.rb
#*	@brief  バッグ内で使用するソートデータを取得
#*	@author	Masayuki Onoue
#*	@date		2011.02.09
#*
#*/
#/]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

require "csv"

#/=============================================================================
#**
#*					定数宣言
#/
#/=============================================================================
CONVERT_FILE_NAME = "swan_item_sort.csv"
SORT1_FILE_NAME = "itemsort_absolute_def.cdat"
SORT2_FILE_NAME = "itemsort_jp_def.cdat"

# ファイルを開く
in_file = CSV.open(CONVERT_FILE_NAME, 'r' )
sort1_file = open(SORT1_FILE_NAME, 'w')
sort2_file = open(SORT2_FILE_NAME, 'w')

# ヘッダ行を飛ばす
in_file.shift

# ヘッダ部分を作成する
header = <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		itemsort_absolute_def.cdat
 *	@brief  全アイテム統一並び順(あいうえお)
 *	@author	Masayuki Onoue
 *	@date		2011.02.09
 *  
 *  /resource/itemconvmakelist_sort_pocket.rbから出力されたデータです。
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// 添字:item_id 
static const u16 ItemSortByAbsolute[] = {
  0,  // ダミーデータ
EOS
sort1_file << header

header = <<EOS
//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		itemsort_jp_def.cdat
 *	@brief  分類別あいうえお順データ
 *	@author	Masayuki Onoue
 *	@date		2011.02.09
 *  
 *  resource/itemconvmakelist_sort_pocket.rbから出力されたデータです。
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// 添字:item_id 
static const u16 ItemSortByJp[] = {
  0,  // ダミーデータ
EOS
sort2_file << header

# ソーとデータを作成
in_file.each do |line|

  unless line[0] == nil then
    # アイテム名を見て使用しないアイテムを決める
    unless line[0].include?("？？？") then
      unless line[1] == nil then
        sort1_file.puts "  "+line[2].to_s+",  // " + line[0]
      end
      unless line[2] == nil then
        sort2_file.puts "  "+line[1].to_s+",  // " + line[0]
      end
    end
  end
end

sort1_file.puts "};"
sort2_file.puts "};"

# ファイルクローズ
sort1_file.close
sort2_file.close

