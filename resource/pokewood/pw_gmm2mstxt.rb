#----------------------------------------------------------------------------
# ポケウッドシナリオデータのgmm→mstxt変換を呼び出すツール
#
# ruby pw_gmm2mstxt.rb <入力1> <入力2> <入力3> ...
#
# <入力>: gmmファイル名
#
# 入力ファイルに対して対応するmstxtファイルとタイムスタンプ比較をして
# コンバーターを呼び出すか判断します
#----------------------------------------------------------------------------
require 'fileutils'

filenum = ARGV.size

filenum.times{|i|
  #コンバート後ファイルパス生成
  mstxtname = "../message/msproj/src/pokewood/" + ARGV[i].sub(/gmm/,"mstxt")
  convname = "src/pokewood/" + ARGV[i].sub(/gmm/,"mstxt")

  #ファイル存在確認
  if FileTest.exist?(ARGV[i])==false then
    printf( "%s is not exist\n", ARGV[i])
    exit(1)
  end
  if FileTest.exist?(mstxtname)==false then
    printf( "%s is not exist\n", mstxtname)
    exit(1)
  end

  #タイムスタンプ取得
  gmm_stat   = File.stat( ARGV[i] )
  mstxt_stat = File.stat( mstxtname )
  printf( "%s=> %s\n", ARGV[i], mstxtname )

  #タイムスタンプ確認(GMMのほうが新しかったらコンバート）
  if gmm_stat.ctime > mstxt_stat.ctime then
    command = "../../tools/message/UpdateWithGmm.bat . " + ARGV[i] + " " +convname
    system ( command )
  end
}


exit(0)
