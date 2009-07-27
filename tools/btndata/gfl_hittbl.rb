#!/usr/bin/ruby
# GFL_UI_TP_HITTBL を適当に作るrubyコンバータ
#
#  テキストから数字の行を抜き出して
#  {},をつけるだけ
#


begin
  dataArray = Array.new
  
  fpr = File.new(ARGV[0])
  fpr.each{ |line|
    line.chomp!
    if line =~ /^=(.*)/
      dataArray.push( $1 )
    end
  }
  fpr.close
  
  fpw = File.new(ARGV[1],"w")
  dataArray.each{ |line|
    outLine = "{"+line+"},"
    fpw.puts(outLine)
  }
  fpw.close
  
end
