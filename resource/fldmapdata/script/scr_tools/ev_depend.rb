#----------------------------------------------------------------------------
#
#   script依存関係ファイルの生成
#
#   2011.04.27  tamada
#
#   [in]  マップスクリプトの名前
#   [out] マップスクリプトの依存関係記述（標準出力に出力）
#
#   命名規則から各マップに対応するスクリプトが依存するファイルとの関係を記述する
#
#----------------------------------------------------------------------------

def putEvDependRule( filename )
  targetname = filename.sub( /\.ev$/,".bin")
  headername = filename.sub( /\.ev$/,".h" )
  output = ""
  output += sprintf("%s: $(EVENTHEADERDIR)%s", targetname, headername )
  output += sprintf(" $(MSGHEADERDIR)msg_%s", headername )
  output += "\n\n"
end

output = <<HEADER_STRING
#==============================================================
#
# 自動生成ファイル：evファイル依存関係
#
#==============================================================
HEADER_STRING

ARGV.each{|filename|
  if filename =~ /^sp_/ then next end
  output += putEvDependRule( filename )
}

puts output
