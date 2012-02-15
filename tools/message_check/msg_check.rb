#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
# 	@file		msg_ocheck.rb
# 	@brief  メッセージのチェック
# 	@author	Masayuki Onoue
# 	@date		2012.01.16
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../mstxt_purser/mstxt_purser"
require "msg_rule_check.rb"
require "msg_ng_data.rb"

require 'csv'

# 文字コード指定
$KCODE = 'SJIS'

# メッセージファイルの位置
MSG_FILES = "C:/home/swan/resource/message/msproj/src/**/*.mstxt" 
mstxt_list = Dir[ MSG_FILES ]


# NGリストの生成
ng_data_array = Array.new

mstxt_list.each_with_index{|file_name,index|

  # デバッグ関連メッセージを読み飛ばす
  if File.basename(file_name) =~ /^d_/ then
    next
  end
  if File.basename(file_name) =~ /^debug_/ then
    next
  end

  #file_name = "C:/home/swan/resource/message/msproj/src/zkn_comment_ita.mstxt"
  #file_name = 'c:\home\swan\resource\message\msproj\src\debug\d_linkfest.mstxt'
  #file_name = 'c:\home\swan\resource\message\msproj\src\script_message\c01r0301.mstxt'

  # 進行状態出力
  puts "%s(%d/%d)"%[File.basename(file_name),index,mstxt_list.size]

  mstxt_data = MSTXT::Document.new(file_name)
  mstxt_data.values.each{|data|
begin
    # 行を考慮したチェック(平仮名で見つかった場合は、漢字はチェックしない)
    if line_msg_okuri_check( file_name, data, data.jpn, ng_data_array ) == false then
      line_msg_okuri_check( file_name, data, data.jpn_kanji, ng_data_array )
    end

    # トレーナー戦闘に移行する前のメッセージなのに、文末に▼がついていない
    tr_last_key_input_check( file_name, data, ng_data_array )

    # 日本語と漢字で最後キー送りするかどうかが変わっていないかチェック
    kana_kanji_last_input_check( file_name, data, ng_data_array )

    # かな欄に、漢字が含まれている
    kana_check( file_name, data, ng_data_array )
    
    # 空白文字の連続をチェック
    space_check( file_name, data, ng_data_array )

    # 半角文字チェック
    half_width_word_check( file_name, data, ng_data_array )
end

    # キー送りの後に、文字が書かれていないかチェック
    last_input_after_word( file_name, data, ng_data_array )
  }

  #break
}

error_cnt = 0
ng_data_array.each{|data|
  if data.inc_okri == "○" then
    error_cnt += 1
  end
}
puts "NGデータ数（総数）　:%d"%ng_data_array.size
puts "NGデータ数(送りあり):%d"%error_cnt

# データ出力
CSV.open("msg_check_result.csv", 'w') do |writer|
  
  # エラー一覧
  writer << [ "ファイル名", "ラベル", "送り","平仮名", "漢字","エラー詳細","パス" ]
  # 検索結果出力
  ng_data_array.each{|data|
    writer << [ File.basename(data.file_name), data.text_data.label,data.inc_okri,data.text_data.jpn,data.text_data.jpn_kanji ,data.error_info,data.file_name ]
  }

end

