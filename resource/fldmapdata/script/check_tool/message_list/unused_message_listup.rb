#################################################################################
# @brief  使用していないメッセージのリストアップ
# @file   unused_message_listup.rb
# @author onoue
# @date   2010.10.8
#
# 調査するファイルタイプを以下の文字列で渡してください
# c = Cファイル , data = cdatファイル , ev = evファイル , h = ヘッダファイル
#
################################################################################# 
require "message.rb"
require "benchmark"
require "csv"

RESULT_FILE     = "../all_message_check_result.txt" # 解析結果の出力ファイル
EV_FILES        = "../../*.ev" # チェック対象の .ev ファイル
H_FILES         = "../../../../../prog/include/**/*.h"  # チェック対象の .c ファイル
LOCAL_H_FILES   = "../../../../../prog/src/**/*.h"  # チェック対象の .c ファイル
C_FILES         = "../../../../../prog/src/**/*.c"  # チェック対象の .c ファイル
CDAT_FILES      = "../../../../../prog/src/**/*.cdat"  # チェック対象の .c ファイル
MSG_ID_DEF_FILES= "../../../../../prog/include/msg/**/*.h" # メッセージIDの定義ファイル
NO_SEARCH_DATA  = "nosearch_gmm.csv" # メッセージIDの定義ファイル

puts "未使用メッセージを調査します"

puts "メッセージIDを取得しています・・・"

#=============================
# 使用しないGMMリストを取得
no_search_gmm_list = Array.new
no_search_list = CSV.open(NO_SEARCH_DATA,"r")
no_search_list.each{|line|
  unless line[1].nil? then
    if line[1].include?("×") then
      # ヘッダ名形式に変換
      no_search_gmm_list << "msg_"+line[0].gsub("\.gmm",".\h").strip
    end
  end
}
no_search_list.close

#--------------------------------------------------------------------------------
# ■すべての使用されているメッセージIDを取得
#--------------------------------------------------------------------------------
ev_list = Array.new

cmd = ARGV[0].to_s + ARGV[1].to_s + ARGV[2].to_s + ARGV[3].to_s

#=============================
# 調査する項目を選択

if cmd =~ /ev/ then
  ev_list = Dir[ EV_FILES ] 
  print "evファイル "
end

if cmd =~ /c/ then
  ev_list = ev_list  + Dir.glob( C_FILES )
  print "cファイル "
end

if cmd =~ /h/ then
  ev_list = ev_list + Dir.glob( H_FILES ) + Dir.glob( LOCAL_H_FILES )
  print "hファイル "
end

if cmd =~ /data/ then
  ev_list = ev_list + Dir.glob( CDAT_FILES )
  print "cdatファイル "
end

print "を調査しています\n\n"

# メッセージIDが定義してあるhファイルは調査しない
ev_list -= Dir[ MSG_ID_DEF_FILES ]
  
out_file = File::open( RESULT_FILE, "w" )
out_file.puts Time.now
out_file.close

#--------------------------------------------------------------------------------
# ■すべてのメッセージIDを取得
#--------------------------------------------------------------------------------
header_list = Dir.glob( MSG_ID_DEF_FILES )

#全てのメッセージファイル出力
#header_list.each{|filename|
#  fname = File.basename(filename).strip.gsub(/^msg_/,"").gsub(/\.h/,"\.gmm")
#  puts fname
#}
#exit

all_message_cnt = 0
header_list.each_with_index do |filename,file_index|

  fname = File.basename(filename).strip
  print fname.ljust(40)
  if no_search_gmm_list.index(fname) != nil then
    puts ": %3.2f%%skip(%4d\/%4d)\n"%[ file_index.to_f/header_list.size*100 ,file_index,header_list.size]
    next
  end

  # 出力先ファイル
  out_file = File::open( RESULT_FILE, "a" )
  
  # メッセージリスト
  all_message_list = Array.new

  file = File.open( filename, "r" )

  # 個数定義ヘッダ
  invalid_id = File::basename( filename, ".*" ) + "_max"
  # インクルードガード定義
  unique_def_id = "__" + File::basename( filename, ".*" ).upcase + "_H__"

  # ファイルない全てを検索
  file.reverse_each do |line|
    if line =~ /#define\s+(\w*)\s*/ then
      str_id = $1.strip
      if (str_id != invalid_id) && ( str_id != unique_def_id ) then
        all_message_list << Message.new( filename, str_id )
      end
    end
  end
  file.close
  all_message_cnt += all_message_list.size
  
  # ラベル数を数えるときだけにコメントアウト
  #next

  ev_list.each do |data_name|
    file = File.open( data_name, "r" )
    in_comment = false

    file.each do |line|
      if in_comment then
        if line.include?( "*/" ) then 
          in_comment = false # コメント終了
        end 
      else
        # C コメント 除外
        line.gsub!( /\/\*.*?\*\//, "" )
        # C コメント部分を除外
        if line.include?( "/*" ) then 
          comment_head = line.index( "/*" )
          line         = line[0...comment_head] 
          in_comment = true
        end
        # C++ コメント部分を除外
        if line.include?( "//" ) then 
          comment_head = line.index( "//" )
          line         = line[0...comment_head] 
        end

        # メッセージが使用されているかチェック
        all_message_list.each do |check_message| 
          if line.include?( check_message.str_id ) then
            all_message_list.delete(check_message)
          end
        end
      end
    end
    file.close
  end


  #--------------------------------------------------------------------------------
  # ■解析結果を出力
  #--------------------------------------------------------------------------------
  gmmname_old = ""
  gmmname = "" 
  all_message_list.each do |message|
    # gmmファイル名に変換
    if message.filename =~ /msg_(\w*)/ then
      gmmname = $1.strip + ".gmm"
    end

    # コメントを生成
    if gmmname_old != gmmname then
      head = "\n//=============================================\n" + "//\t" + gmmname + "\n" + "//============================================="
      out_file.puts( head )
      gmmname_old = gmmname
    end

    # str_idを出力
    out_file.puts( message.str_id )
  end
  
  puts ": %3.2f%%完了(%4d\/%4d)\n"%[ file_index.to_f/header_list.size*100 ,file_index,header_list.size]
  out_file.close
end

puts "メッセージラベル総数:%d"%[all_message_cnt]

