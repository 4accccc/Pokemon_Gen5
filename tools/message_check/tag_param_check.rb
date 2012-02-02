#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
# 	@file		tag_param_check.rb
# 	@brief  タグのパラメータを修正
# 	@author	Masayuki Onoue
# 	@date		2012.02.01
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../mstxt_purser/mstxt_purser"

require 'csv'

# 文字コード指定
$KCODE = 'SJIS'

# 検索対象パラメータ名
PARAM_NAME = "Idx"

# NGデータ
class NG_DATA
  attr_accessor :file_name,:text_data 

  # データ初期化
  def initialize( file_name, data  )
    @file_name  = file_name
    @text_data  = data
  end
end


# 全タグを調べてエラーがあったかどうかを返す
def search_error_tag( tag_list )

  # 見つかったか
  is_find = false

  # タグが含まれていない場合は調査しない
  if tag_list.size > 0 then
    # 指定パラメータが含まれていないラベルを調べる
    tag_list.each{|tag|
      # 他のグループタグは調べる必要が無い
      case tag.group
      when "Name"
        unless tag.param.key?(PARAM_NAME) then
          is_find = true
          break
        end
      when "Digit"
        unless tag.param.key?(PARAM_NAME) then
          is_find = true
          break
        end
      end
    }
  end

  return is_find
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

    # ひらがな、漢字ともにチェックする
    if search_error_tag( data.jpn_tag ) then
      ng_data_array << NG_DATA.new(file_name,data)
    elsif search_error_tag( data.kanji_tag ) then
      ng_data_array << NG_DATA.new(file_name,data)
    end

  }
}

puts "NGデータ数:%d"%ng_data_array.size

# データ出力
CSV.open("tag_param_check_result.csv", 'w') do |writer|
  writer << [ "ファイル名", "ラベル" ,"テキスト（ひらがな）","テキスト（漢字）","パス" ]
  # 検索結果出力
  ng_data_array.each{|data|
    writer << [ File.basename(data.file_name), data.text_data.label ,data.text_data.jpn ,data.text_data.jpn_kanji ,data.file_name ]
  }
end

