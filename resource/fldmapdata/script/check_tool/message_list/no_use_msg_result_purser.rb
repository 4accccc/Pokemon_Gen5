#################################################################################
# @brief  使用していないメッセージのリストアップの結果を再度解析するツール
# @file   no_use_msg_result_purser.rb
# @author onoue
# @date   2011.12.2
################################################################################# 
GMMDATA =Struct.new("GMMDATA_",:name,:data_list)
require "csv"

result = open("../all_message_check_result.txt","r")

NO_SEARCH_DATA  = "nosearch_gmm.csv" # メッセージIDの定義ファイル
#=============================
# 使用しないGMMリストを取得
no_search_gmm_list = Array.new
no_search_list = CSV.open(NO_SEARCH_DATA,"r")
no_search_list.each{|line|
  unless line[1].nil? then
    if line[1].include?("×") then
      # ヘッダ名形式に変換
      no_search_gmm_list << line[0].strip
    end
  end
}
no_search_list.close

# リストデータ
gmm_list = Array.new

# trueの時ラベル解析中
data_state = false

# データ格納先
gmm_data = nil


result.each{|line|

  # 空白行の検索
  if line.size == 1 then
    data_state = false
  end

  # ラベルのカウント
  if data_state == true then
    # コメント行を含んでいない時のみ格納
    unless line.include?('//') then
      gmm_data.data_list << line
    end
  end

  # ラベルの発見
  if line.include?(".gmm") then
    gmm_data = GMMDATA.new
    gmm_data.name = line.to_s.gsub("//","").strip
    gmm_data.data_list = Array.new
    gmm_list << gmm_data
    data_state = true
  end
}

# 未発見ラベルの多い順にソート
gmm_list.sort!{|a, b| 
  (-1) *(a.data_list.size <=> b.data_list.size )
}

# ラベル総数
all_label_cnt = 0
data_cnt = 0
gmm_list.each{|gmm_data|
  if no_search_gmm_list.index(gmm_data.name).nil? then
    puts "%s\t%4d"%[gmm_data.name.ljust(27),gmm_data.data_list.size]
    data_cnt += 1
    all_label_cnt+=gmm_data.data_list.size
  end
}

puts "GMM数：%d"%[data_cnt]
puts "未発見ラベル総数：%d"%[all_label_cnt]

result.close
