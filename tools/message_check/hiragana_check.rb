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

# NGデータ
class NG_DATA
  attr_accessor :file_name,:text_data ,:kanji

  # データ初期化
  def initialize( file_name, data ,kanji )
    @file_name  = file_name
    @text_data  = data
    @kanji      = kanji
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

    # 漢字以外を削除
    kanji_list = data.jpn.gsub(/[^亜-煕]/,'')
    # 円を削除
    kanji_list = kanji_list.gsub(/[円]/,'')
    if kanji_list.size != 0 then
      ng_data_array << NG_DATA.new(file_name,data,kanji_list)
    end
  }
}

puts "NGデータ数:%d"%ng_data_array.size

# データ出力
CSV.open("hiragana_result.csv", 'w') do |writer|
  writer << [ "ファイル名", "ラベル" ,"漢字","パス" ]
  # 検索結果出力
  ng_data_array.each{|data|
    writer << [ File.basename(data.file_name), data.text_data.label ,data.kanji,data.file_name ]
  }
end

