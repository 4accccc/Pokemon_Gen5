#----------------------------------------------------------------------------
#
#   デバッグスクリプト選択用リストデータ生成
#
#   2009.11.29  tamada GAMEFREAK inc.
#
#----------------------------------------------------------------------------
require "FileUtils"

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
def reader  file
  result = Array.new

  file.each{|line|
    if line =~/#define SCRID_DEBUG/ then
      column = line.split
      sym = column[1].sub(/SCRID_DEBUG_/,"").upcase
      id = Integer(column[2].gsub(/[()]/,"") )
      result << [ id, sym ]
    end
  }
  return result
end

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
def make_binary( result )
  output = String.new
  result.each{|id, sym|
    output += [id].pack( "S" )
    output += [sym].pack( "a29x" )
  }
  return output
end

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
HEADER_DUMMY_STRING = <<DUMMY_STRING
//======================================================================
//  debug_list.h
//  デバッグスクリプト用定義ファイル（自動生成）
//======================================================================
#pragma once
DUMMY_STRING

def make_header( result )

output = HEADER_DUMMY_STRING
output += sprintf("enum{\n")
output += sprintf("  DEBUG_SCR_MAX = %d,\n", result.length )
output += sprintf("  DEBUG_SCR_OFS_ID = %d,\n", 0 )
output += sprintf("  DEBUG_SCR_OFS_NAME = %d,\n", 2 )
output += sprintf("  DEBUG_SCR_NAME_LEN = %d,\n", 32 - 2 )
output += sprintf("  DEBUG_SCR_EACH_SIZE = %d,\n", 32 )
output += sprintf("};")

return output
end

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
if ARGV.length < 2 then
  STDERR.puts "引数が足りません"
  exit
end
if FileTest.exists?(ARGV[0]) == false then
  STDERR.puts "読み込みファイル#{ARGV[0]}が存在しません"
  exit
end

File.open(ARGV[0],"r"){|file|
  result = reader( file )
  puts make_header( result )
  bin_out = make_binary( result )
  File.open(ARGV[1],"wb"){|file|
    file.write( bin_out )
  }
}




