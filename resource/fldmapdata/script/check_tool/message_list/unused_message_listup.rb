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

RESULT_FILE     = "../result.txt" # 解析結果の出力ファイル
EV_FILES        = "../../*.ev" # チェック対象の .ev ファイル
H_FILES         = "../../../../../prog/include/**/*.h"  # チェック対象の .c ファイル
LOCAL_H_FILES   = "../../../../../prog/src/**/*.h"  # チェック対象の .c ファイル
C_FILES         = "../../../../../prog/src/**/*.c"  # チェック対象の .c ファイル
CDAT_FILES      = "../../../../../prog/src/**/*.cdat"  # チェック対象の .c ファイル
MSG_ID_DEF_FILES= "../../../../../prog/include/msg/script/*.h" # メッセージIDの定義ファイル

puts Benchmark.measure{

  puts "未使用メッセージを調査します"

  puts "メッセージIDを取得しています・・・"
  # メッセージリスト
  all_message_list = Array.new
  used_message_list = Array.new
  unused_message_list = Array.new

  #--------------------------------------------------------------------------------
  # ■すべてのメッセージIDを取得
  #--------------------------------------------------------------------------------
  header_list = Dir.glob( MSG_ID_DEF_FILES )
  header_list.each do |filename|
    file = File.open( filename, "r" )
    invalid_id = File::basename( filename, ".*" ) + "_max"
    file.each do |line|
      if line =~ /#define\s+(msg_\w*)\s/ then
        str_id = $1.strip
        if str_id != invalid_id then
          all_message_list << Message.new( filename, str_id )
        end
      end
    end
    file.close
  end

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

  print "を調査しています\n"

  # メッセージIDが定義してあるhファイルは調査しない
  ev_list -= Dir[ MSG_ID_DEF_FILES ]

  ev_list.each do |filename| 
    file = File.open( filename, "r" )
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
        # メッセージIDの参照チェック
        while line =~ /(msg_\w*)/
          str_id = $1.strip
          used_message_list << Message.new( filename, str_id )
          line.gsub!( str_id, "" ) # 発見したメッセージIDは除外する
        end
      end
    end

    file.close
  end


  #--------------------------------------------------------------------------------
  # ■解析
  #--------------------------------------------------------------------------------
  print "解析結果を書き込んでいます\n"
  all_message_list.each do |check_message| 
    used = false
    used_message_list.each do |used_message|
      if check_message.str_id == used_message.str_id then
        used = true
        break
      end
    end

    # 使用されていない
    if used == false then
      unused_message_list << check_message
    end
  end

  print "\n//=================================\n"
  print "// 解析完了\n"
  print "//=================================\n"

  puts "メッセージ総数：" + all_message_list.size.to_s
  puts "使用メッセージ総数：" + used_message_list.size.to_s
  puts "未使用メッセージ総数：" + unused_message_list.size.to_s

  #--------------------------------------------------------------------------------
  # ■解析結果を出力
  #--------------------------------------------------------------------------------
  file = File::open( RESULT_FILE, "w" )
  gmmname_old = ""
  gmmname = "" 
  unused_message_list.each do |message|
    # gmmファイル名に変換
    if message.filename =~ /msg_(\w*)/ then
      gmmname = $1.strip + ".gmm"
    end

    # コメントを生成
    if gmmname_old != gmmname then
      head = "\n//=============================================\n" + "//\t" + gmmname + "\n" + "//============================================="
      file.puts( head )
      gmmname_old = gmmname
    end

    # str_idを出力
    file.puts( message.str_id )
  end
  file.close

  puts Benchmark::CAPTION
}

