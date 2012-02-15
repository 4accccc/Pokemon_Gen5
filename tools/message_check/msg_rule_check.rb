#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
# 	@file		msg_rule_check.rb
# 	@brief  メッセージルールチェック
# 	@author	Masayuki Onoue
# 	@date		2012.02.10
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
require "../mstxt_purser/mstxt_purser"
require "msg_ng_data.rb"

require 'csv'

# 文字コード指定
$KCODE = 'SJIS'

# 最後の文字を取得する
def get_last_word( word )
  size = word.split(//s).size
  last_word = word.strip.split(//s)[size-1..size].to_s
  return last_word
end

# タグを除いたメッセージを作成
def get_remove_tag_msg( msg )
  return msg.gsub(/\[.+?\]/,"")
end

# 行数を考慮した、行送りチェック
def line_msg_okuri_check( file_name, data, msg, ng_data_array )

  line_index = 0          # ▼のみでリセット
  line_index_reverse = 0  # ▼と▽でリセットするカウンタ

  # 送りが含まれていない場合はスキップ
  if (msg.include?("▼") == false) && (msg.include?("▽")==false) then
    return false
  end

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

# トレーナー戦闘に移行する前のメッセージなのに、文末に▼がついていない
def tr_last_key_input_check( file_name, data, ng_data_array )

  # トレーナー戦闘に移行する前のメッセージなのに、文末に▼がついていない
  if data.label.include?( "TRMSG_FIGHT_START" ) then
    if (( get_last_word( data.jpn ).to_s != "▼" ) ||
        ( get_last_word( data.jpn_kanji ).to_s != "▼" )) then
      ng_data_array << NG_DATA.new(file_name,data,"トレーナー戦闘に移行する前のメッセージなのに、文末に▼がついていない")
    end
  end
end


# 日本語と漢字で最後キー送りするかどうかが変わっていないかチェック
def kana_kanji_last_input_check( file_name, data, ng_data_array )

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
    ng_data_array << NG_DATA.new(file_name,data,"漢字とひらがなでキー送りが違います")
  end

end

# かな欄に、漢字が含まれている
def kana_check( file_name, data, ng_data_array )

  # 漢字以外を削除
  kanji_list = data.jpn.gsub(/[^亜-煕]/,'')
  # 円を削除
  kanji_list = kanji_list.gsub(/[円]/,'')
  if kanji_list.size != 0 then
    ng_data_array << NG_DATA.new(file_name,data,"平仮名欄に漢字が含まれている")
  end

end

# 空白スペースの連続をチェック
def space_check( file_name, data, ng_data_array )

  # 空白の連続をチェック(全角スペース２連続)
  if data.jpn =~ /　　/ then
    ng_data_array << NG_DATA.new(file_name,data,"空白文字の連続が含まれています")
    return
  end
  if data.jpn_kanji =~ /　　/ then
    ng_data_array << NG_DATA.new(file_name,data,"空白文字の連続が含まれています")
    return
  end

  # 空白の連続をチェック(半角スペース４連続)
  if data.jpn =~ /    / then
    ng_data_array << NG_DATA.new(file_name,data,"空白文字の連続が含まれています")
    return
  end
  if data.jpn_kanji =~ /    / then
    ng_data_array << NG_DATA.new(file_name,data,"空白文字の連続が含まれています")
    return
  end

end

# 半角文字チェック(漢字と平仮名両方から調べる)
def half_width_word_check( file_name, data, ng_data_array )
  # 半角だけの文字以外をチェック
  if half_width_word_check_msg(data.jpn) == true then
    ng_data_array << NG_DATA.new(file_name,data,"半角スペースが含まれています")
    return
  end
  if half_width_word_check_msg(data.jpn_kanji) == true then
    ng_data_array << NG_DATA.new(file_name,data,"半角スペースが含まれています")
    return
  end
end

# 半角だけの文字以外をチェック
def half_width_word_check_msg( msg )
  str = get_remove_tag_msg(msg).gsub("▼","").gsub("▽","")
  unless ((str =~/^[ -~｡-ﾟ\s“°]*\z/) &&  ( str.size > 0 )) then
    if get_remove_tag_msg(msg).include?(" ") then
      return true
    end
  end
  return false
end

# キー送りの後に文字が書かれていないかチェック
def last_input_after_word( file_name, data, ng_data_array )
  if last_input_after_word_msg( data.jpn ) then
    ng_data_array << NG_DATA.new(file_name,data,"キー送りの後に文字が存在します")
  elsif last_input_after_word_msg( data.jpn_kanji ) then
    ng_data_array << NG_DATA.new(file_name,data,"キー送りの後に文字が存在します")
  end
end

def last_input_after_word_msg( msg )
  line_list = msg.split("\n")
  line_list.each{|line|
    input_after = ""
    if line.include?("▼") then
      input_after = line.gsub(/.*▼/,"")
    elsif line.include?("▽") then
      input_after = line.gsub(/.*▽/,"")
    end
    if input_after.size > 0 then
      puts line
      return true
    end
  }
  return false
end


