#=====================================================================================
#
# make環境監視用スクリプト
#
# 2010.01.06  tamada GAMEFREAK inc.
# 2010.02.04  Javaの存在チェックを追加
#
#
# 引数に環境変数で指定されている動作ルートディレクトリを渡し、現在のディレクトリと比較する。
# 一致しない場合エラーにする
#
# Javaの存在チェックを行う
# 存在しない場合エラーにする
#
#
#=====================================================================================

#------------------------------------------------------
# 引数をチェック
#------------------------------------------------------
arg_dir = ARGV.shift
if arg_dir == nil || arg_dir == "" || !( File.exists?(arg_dir) ) then
  puts "enviroment check:"
  puts "対象ディレクトリ（#{arg_dir}）が見つかりません！"
  exit -1
end

#------------------------------------------------------
# 現在のディレクトリと引数で受け取ったディレクトリの比較
#------------------------------------------------------
target_dir = arg_dir.gsub( /\\/, "/" ).sub( /\/$/, "" ).downcase
nowdir = `cygpath -m #{Dir.pwd}`.chomp.downcase
if nowdir != target_dir then
  puts "enviroment check:"
  puts "対象ディレクトリ（#{target_dir}）と現在のディレクトリ（#{nowdir}）が一致しません！"
  exit -1
end

#------------------------------------------------------
# Javaの存在チェック
#------------------------------------------------------
javapath = `which java`.chomp + ".exe"

if javapath =~ /^which: no java in / || File.exists?( javapath ) == false then
  puts "enviroment check:"
  puts javapath
  puts "Javaが見つかりません"
  exit -1
end


