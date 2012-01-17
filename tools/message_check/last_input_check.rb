#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
# 	@file		last_input_check.rb
# 	@brief  キー送りのチェック
# 	@author	Masayuki Onoue
# 	@date		2012.01.16
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../mstxt_purser/mstxt_purser"

require 'csv'

# 文字コード指定
$KCODE = 'SJIS'

# 最後の文字を取得する
def get_last_word( word )
  size = word.split(//s).size
  last_word = word.strip.split(//s)[size-1..size].to_s
  return last_word
end

# NGデータ
class NG_DATA
  attr_accessor :file_name,:text_data

  # データ初期化
  def initialize( file_name, data )
    @file_name  = file_name
    @text_data  = data
  end
end

# メッセージファイルの位置
MSG_FILES = "C:/home/swan/resource/message/msproj/src/**/*.mstxt" 
mstxt_list = Dir[ MSG_FILES ]


# NGリストの生成
ng_data_array = Array.new

mstxt_list.each_with_index{|file_name,index|

  # 進行状態出力
  puts "%s(%d/%d)"%[File.basename(file_name),index,mstxt_list.size]

  mstxt_data = MSTXT::Document.new(file_name)

  # 日本語と漢字で最後キー送りするかどうかが変わっていないかチェック
  mstxt_data.values.each{|data|

    # 初期化
    is_jpn_last_key       = false
    is_jpn_kanji_last_key = false

    # ひらがなは最後キー送りありかどうか
    last_word = get_last_word( data.jpn )
    if last_word.to_s == '▼' then
      is_jpn_last_key = true
    end

    # 漢字は最後キー送りありかどうか
    last_word = get_last_word( data.jpn_kanji )
    if last_word.to_s == '▼' then
      is_jpn_kanji_last_key = true
    end 

    # キー送りの状態が異なっていた場合はデータをリスト化
    if is_jpn_last_key != is_jpn_kanji_last_key then
      ng_data_array << NG_DATA.new(file_name,data)
    end
  }
}

puts "NGデータ数:%d"%ng_data_array.size

# データ出力
CSV.open("result.csv", 'w') do |writer|
  writer << [ "ファイル名", "ラベル" ]
  # 検索結果出力
  ng_data_array.each{|data|
    writer << [ data.file_name, data.text_data.label ]
  }
end

