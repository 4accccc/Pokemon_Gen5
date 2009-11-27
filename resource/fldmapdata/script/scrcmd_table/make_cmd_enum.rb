#===================================================================
#
#   スクリプトコマンド名と数値の対応表生成
#
#   2009 tamada GAMEFREAK inc.
#===================================================================

File.open("../../../../prog/src/field/scrcmd_table.cdat"){|file|
  count = 0
  maxlen = 0
  output = Array.new
  file.each{|line|
    column = line.split
    if line =~/DEF_CMD\(/ then
      cmdname = column[1].sub(/,.*$/,"")
      output << cmdname
      if column[1].length > maxlen then maxlen = cmdname.length end
      count += 1
    elsif line =~/NO_CMD\(/ then
      output << ""
      count += 1
    end
  }
  output.each_with_index{|line, count|
    if line != "" then
      printf("%s%s %04x (%3d)\n", line, " " * (maxlen + 1 - line.length), count, count)
    end
  }
}
