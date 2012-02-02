#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
# 	@file		msg_okuri_check.rb
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

# 最後の文字を取得する
def get_last_word( word )
  size = word.split(//s).size
  last_word = word.strip.split(//s)[size-1..size].to_s
  return last_word
end

# 行数を考慮した、行送りチェック
def line_msg_okuri_check( file_name, data, msg, ng_data_array )

  line_index = 0          # ▼のみでリセット
  line_index_reverse = 0  # ▼と▽でリセットするカウンタ

  msg_split = msg.split("\n")

  msg_split.each_with_index{|line,index|

    # 空白行は無視する
    if line.size <= 0 then
      next
    end
    
    # 条件A:1行目、あるいは▼で終わった次の行の文末に▽はつかない
    #       (BTSNo1199と同じパターン)
    if line.include?("▽") &&  (line_index == 0) then
      ng_data_array << NG_DATA.new(file_name,data,"1行目、あるいは▼で終わった次の行の文末に▽はつかない")
      return true
    end

    # 条件B:文章の頭から、あるいは▼で終わった次から3行以上続くのに
    #       2行続けて文末に送りマークがないと表示されない行が発生する。
    #       (BTSNo1223などのパターン)
    if line_index_reverse >= 2 then
      ng_data_array << NG_DATA.new(file_name,data,"文章の頭から、あるいは▼で終わった次から3行以上続くのに、2行続けて文末に送りマークがない")
      return true
    end

    # 区切りを調べる
    if line.include?("▼") then
      line_index = 0
    else
      line_index += 1
    end

    if line.include?("▼") || line.include?("▽") then
      line_index_reverse = 0
    else
      line_index_reverse += 1
    end
  }
      
  return false
end

# 例外ファイルリストを読み込む
exp_list = Array.new
exp_list_file = open("msg_okuri_exp.txt","r")
exp_list_file.each{|line|
  exp_list << line.strip
}
exp_list_file.close

# メッセージファイルの位置
MSG_FILES = "C:/home/swan/resource/message/msproj/src/**/*.mstxt" 
mstxt_list = Dir[ MSG_FILES ]

# 例外ファイルをチェック
exp_file_list = Marshal.load(Marshal.dump(mstxt_list))

exp_list.each{|exp|
  mstxt_list.delete_if {|file_name| File.basename(file_name) =~ /^#{exp}/}
}
exp_file_list = exp_file_list - mstxt_list

# NGリストの生成
ng_data_array = Array.new

mstxt_list.each_with_index{|file_name,index|

  #file_name = 'c:\home\swan\resource\message\msproj\src\script_message\c01r0301.mstxt'

  # 進行状態出力
  puts "%s(%d/%d)"%[File.basename(file_name),index,mstxt_list.size]

  mstxt_data = MSTXT::Document.new(file_name)

  # 日本語と漢字で最後キー送りするかどうかが変わっていないかチェック
  mstxt_data.values.each{|data|
   
    # 行を考慮したチェック(平仮名で見つかった場合は、漢字はチェックしない)
    if line_msg_okuri_check( file_name, data, data.jpn, ng_data_array ) == false then
      line_msg_okuri_check( file_name, data, data.jpn_kanji, ng_data_array )
    end
    
    # トレーナー戦闘に移行する前のメッセージなのに、文末に▼がついていない
    if data.label.include?( "TRMSG_FIGHT_START" ) then
      if (( get_last_word( data.jpn ).to_s != "▼" ) ||
          ( get_last_word( data.jpn_kanji ).to_s != "▼" )) then
        ng_data_array << NG_DATA.new(file_name,data,"トレーナー戦闘に移行する前のメッセージなのに、文末に▼がついていない")
      end
    end
 
  }
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
CSV.open("msg_okuri_result.csv", 'w') do |writer|
  
  # エラー一覧
  writer << [ "ファイル名", "ラベル", "送り","平仮名", "漢字","エラー詳細","パス" ]
  # 検索結果出力
  ng_data_array.each{|data|
    writer << [ File.basename(data.file_name), data.text_data.label,data.inc_okri,data.text_data.jpn,data.text_data.jpn_kanji ,data.error_info,data.file_name ]
  }

  # 例外一覧
  writer << [ "以下はチェックを行わなかった例外ファイル一覧です。" ]
  exp_file_list.each{|file_name|
    writer << [ file_name ]
  }
end

