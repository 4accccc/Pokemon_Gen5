#=================================================================
#
#   リビジョン・ユーザー名取り出しスクリプト
#
#   2009.11.26  tamada GAMEFREAK inc.
#
#   svn info コマンドの出力を解析して必要なパラメータを取り出す
#
#   オプション：
#   -n    ユーザー名
#   -r    リビジョン
#   -d    日付
#   オプションを指定しない場合、上記すべて
#
#=================================================================

result = `svn info -r HEAD`
lines = result.split("\r\n")
author = "Not Found"
rev = "Not Found"
date = "Not Found"

lines.each{|line|
  column = line.split
  if line =~/Last Changed Author/ then
    author = column[3]
  elsif line =~/Last Changed Rev/ then
    rev = column[3]
  elsif line =~/Last Changed Date/ then
    date = column[3] + " " + column[4]
  else
  #  p line
  end
}
case ARGV[0]
when "-n" then
  puts author
when "-r" then
  puts rev
when "-d" then
  puts date
else
  puts author
  puts rev
  puts date
end

