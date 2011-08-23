####################################################################################
#
#
# スクリプトコマンドの実行時チェックデータ生成
#
# 2010.06.18  obata
# 2011.08.23  tamada  シーケンスの重複使用チェックを追加
#
#
####################################################################################
require "command.rb"

PROJECT_ROOT = ENV["PROJECT_ROOT"]
require PROJECT_ROOT + 'tools/constant'

#scrcmd_list.xlsの列の定義
class COL
  enum_const_set %w[
    COMMAND
    NORMAL_ENABLE
    INIT_ENABLE
    ZONE_CHANGE_ENABLE
    AUTHOR
    OVERLAY
    SEQUENCE
  ]
end

# コマンドリスト生成 ############################################
command_list = CommandList.new 

file = File.open( "scrcmd_table.cdat", "r" )
file_lines = file.readlines
file.close

code_no = 0

file_lines.each do |line|
  if line =~ /NO_CMD\s*\(\s*\)/ then
    command_data = CommandData.new
    command_data.seq = "NO_CMD"
    command_data.command = "NO_CMD"
    command_data.code_no = code_no
    command_data.normal_enable = false
    command_data.init_enable = false
    command_data.recover_enable = false
    command_data.zone_change_enable = false
    command_list.add( command_data )
    code_no = code_no + 1
  elsif line =~ /(EV_SEQ_\w+)/ then
    command_data = CommandData.new
    command_data.seq = $1.strip
    command_data.command = "NO_DATA_#{code_no}"
    command_data.code_no = code_no
    command_data.normal_enable = true
    command_data.init_enable = true
    command_data.recover_enable = true
    command_data.zone_change_enable = true
    command_list.add( command_data )
    code_no = code_no + 1
  end
end


# コマンド名, 使用許可フラグ取得 ##############################
file = File.open( "scrcmd_list.txt", "r" )
file_lines = file.readlines
file_lines.slice!(0) # 先頭行を削除
file.close 

file_lines.each do |line|
  line.gsub!( "\"", "" )
  items = line.split( /\t/ ) 

  seq = items[COL::SEQUENCE].strip
  command_data = command_list.search_by_seq( seq )
  if command_data != nil then

    normal_enable = if items[COL::NORMAL_ENABLE].include?( "×" ) then false else true end
    init_enable = if items[COL::INIT_ENABLE].include?( "×" ) then false else true end
    zone_change_enable = if items[COL::ZONE_CHANGE_ENABLE].include?( "×" ) then false else true end

    if command_data.defined == true then
      if command_data.normal_enable != normal_enable ||
        command_data.init_enable != init_enable ||
        command_data.zone_change_enable != zone_change_enable then
        STDERR.printf("シーケンスの重複使用：%s\n", command_data.seq )
        STDERR.printf("使用制限が違う！\n")
      end
    end
    command_data.normal_enable = normal_enable
    command_data.init_enable = init_enable
    command_data.recover_enable = init_enable
    command_data.zone_change_enable = zone_change_enable
    command_data.defined = true
    if items[COL::COMMAND] =~ /(_\w+)\(/ then
      command_data.command = $1.strip
    end 
  elsif seq != "簡易コマンド" && items[COL::OVERLAY] != "常駐" then
    STDERR.printf("Error: %s\n", items[COL::COMMAND] )
    STDERR.printf("未定義のシーケンス(%s)を使用しています！\n", seq )
    exit -1
  end
end

command_list.get_all_command_data.each{|command_data|
  if command_data.seq != "NO_CMD" && command_data.defined == false then
    #STDERR.printf("未使用のシーケンス　%03d:%s\n", command_data.code_no, command_data.seq )
  end
}


# テーブル出力 ##################################################
table_data = Array.new
table_data << "// resource/fldmapdata/script/scrcmd_list.rb により自動生成"
table_data << "static const CMD_CHECK_DATA cmd_check_data[] = {"
all_command_data = command_list.get_all_command_data
all_command_data.each do |command_data|
  code_no = command_data.code_no
  normal_enable = command_data.normal_enable.to_s.upcase
  init_enable = ( command_data.init_enable && command_data.recover_enable ).to_s.upcase
  zone_change_enable = command_data.zone_change_enable.to_s.upcase
  command = command_data.command
  table_data << "  { #{normal_enable}, #{init_enable}, #{zone_change_enable} }, // #{command} (#{code_no})"
end
table_data << "};"

file = File.open( "cmd_check_data.cdat", "w" ) 
file.puts( table_data )
file.close


# テーブル出力 ##################################################
table_data = Array.new
table_data << "$command_array = ["
all_command_data = command_list.get_all_command_data
all_command_data.each do |command_data|
  table_data << "  \"#{command_data.command}\","
end
table_data << "]"

file = File.open( "command_array.rb", "w" ) 
file.puts( table_data )
file.close
