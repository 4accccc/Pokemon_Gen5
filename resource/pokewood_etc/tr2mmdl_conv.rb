#------------------------------------------------------------------------------
#
# ポケウッド（仮）のトレーナータイプ指定から動作モデル指定を取り出すテーブルを作る
#
# @author  tamada GAMEFREAK inc.
# @date     2011.09.22
#------------------------------------------------------------------------------

raw_data = Array.new

File.open(ARGV[0],"r"){|readfile|
  read_start = false
  readfile.each{|line|
    if line =~ /\#START/ then
      read_start = true
    elsif line =~ /\#END/ then
      read_start = false
    elsif read_start == true then
      raw_data << line.chomp
    end
  }
}

File.open(ARGV[1],"w"){|writefile|
  writefile.puts("//TEST")
}
