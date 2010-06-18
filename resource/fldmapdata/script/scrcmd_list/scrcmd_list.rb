require "command.rb"


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

  command_data = command_list.search_by_seq( items[5].strip )
  if command_data != nil then
    if items[0] =~ /(_\w+)\(/ then
      command_data.command = $1.strip
    end 
    if items[1].include?( "×" ) then
      command_data.normal_enable = false
    end 
    if items[2].include?( "×" ) then
      command_data.init_enable = false
      command_data.recover_enable = false
    end
    if items[3].include?( "×" ) then
      command_data.zone_change_enable = false
    end 
  end
end


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
