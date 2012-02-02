#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
# 	@file		tag_cnt_check.rb
# 	@brief  タグの数が漢字と平仮名で異なっていないかチェック
# 	@author	Masayuki Onoue
# 	@date		2012.02.01
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../mstxt_purser/mstxt_purser"

require 'csv'

# 文字コード指定
$KCODE = 'SJIS'

# NGデータ
class NG_DATA
  attr_accessor :file_name,:text_data 

  # データ初期化
  def initialize( file_name, data  )
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

    # カウンタ(ひらがな)
    n_cnt_j = 0
    d_cnt_j = 0
    # カウンタ（漢字）
    n_cnt_k = 0
    d_cnt_k = 0

    # 指定パラメータが含まれていないラベルを調べる
    data.jpn_tag.each{|tag|
      # 他のグループタグは調べる必要が無い
      case tag.group
      when "Name"
        n_cnt_j += 1
      when "Digit"
        d_cnt_j += 1
      end
    }

    # 指定パラメータが含まれていないラベルを調べる
    data.kanji_tag.each{|tag|
      # 他のグループタグは調べる必要が無い
      case tag.group
      when "Name"
        n_cnt_k += 1
      when "Digit"
        d_cnt_k += 1
      end
    } 

    # 個数を比較
    if (n_cnt_j != n_cnt_k) || (d_cnt_j != d_cnt_k) then
      ng_data_array << NG_DATA.new(file_name,data)
    end
  }
}

puts "NGデータ数:%d"%ng_data_array.size

# データ出力
CSV.open("tag_cnt_check_result.csv", 'w') do |writer|
  writer << [ "ファイル名", "ラベル" ,"テキスト（ひらがな）","テキスト（漢字）","パス" ]
  # 検索結果出力
  ng_data_array.each{|data|
    writer << [ File.basename(data.file_name), data.text_data.label ,data.text_data.jpn ,data.text_data.jpn_kanji ,data.file_name ]
  }
end

