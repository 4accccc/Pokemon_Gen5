#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
# 	@file		msg_ng_data.rb
# 	@brief  メッセージのチェック用NGデータクラス
# 	@author	Masayuki Onoue
# 	@date		2012.01.16
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../mstxt_purser/mstxt_purser"

# 文字コード指定
$KCODE = 'SJIS'

# NGデータ
class NG_DATA
  attr_accessor :file_name,:text_data ,:error_info ,:inc_okri

  # データ初期化
  def initialize( file_name, data ,error_info )
    @file_name    = file_name
    @text_data    = data
    @error_info   = error_info

    # メッセージ送りありなし
    if (data.jpn.include?("▼") ||  data.jpn.include?("▽") ||
        data.jpn_kanji.include?("▼") ||  data.jpn_kanji.include?("▽")) then
      @inc_okri     = "○"
    else
      @inc_okri     = "×"
    end
  end
end

