#################################################################################
# @brief  使用していないメッセージのリストアップ
# @file   unused_message_listup.rb
# @author obata
# @date   2010.05.13
################################################################################# 
require "message.rb"

RESULT_FILE      = "../result/result.txt" # 解析結果の出力ファイル
EV_FILES         = "../../../*.ev" # チェック対象の .ev ファイル
MSG_ID_DEF_FILES = "../../../../../../prog/include/msg/script/*.h" # メッセージIDの定義ファイル


# メッセージリスト
all_message_list = Array.new
used_message_list = Array.new
unused_message_list = Array.new


#--------------------------------------------------------------------------------
# ■すべてのメッセージIDを取得
#--------------------------------------------------------------------------------
header_list = Dir[ MSG_ID_DEF_FILES ]
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
ev_list = Dir[ EV_FILES ]

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
    puts check_message.str_id
    unused_message_list << check_message
  end
end

puts all_message_list.size
puts used_message_list.size
puts unused_message_list.size

#--------------------------------------------------------------------------------
# ■解析結果を出力
#--------------------------------------------------------------------------------
file = File::open( RESULT_FILE, "w" )
unused_message_list.each do |message|
  file.puts( message.filename + ": " + message.str_id )
end
file.close
