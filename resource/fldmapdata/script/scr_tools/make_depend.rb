
def putDependRule( filename )
  targetname = filename.sub( /\.ev$/,"_def.h" )
  output = ""
  output += sprintf("%s: %s\n", targetname, filename )
  output += sprintf("\t$(EV_DEF) %s $(SCR_OFFS_ID_H)\n", filename);
  output += "\n"

  return output
end

output = <<HEADER_STRING
#==============================================================
#
# 自動生成ファイル：ヘッダ生成ルール
#
#==============================================================
HEADER_STRING

ARGV.each{|filename|
  if filename =~ /^sp_/ then next end
  output += putDependRule( filename )
}

puts output
