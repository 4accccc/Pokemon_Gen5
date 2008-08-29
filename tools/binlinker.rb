#!/usr/bin/ruby -Ks
# ファイルを連結する ヘッダーには連結したファイルの相対アドレスが入る
#  > binlinker.rb コピー元ファイル ...    出力するファイル
#    ※ コピー元ファイルは複数指定できる
# 例  ruby binlinker.rb test1.bin test2.bin test3.bin output.bin
# k.ohno   2008.08.29


require 'ftools'

begin
  exit 1 if ARGV.size < 2

  outFileName = ARGV.pop
  
  ArrayFile = []
  ArraySize = []
  counter = 0
  
  ARGV.each do |sfile|
    if FileTest.exist?(sfile)      # 存在確認
      ArraySize[counter] = File.size(sfile)
      ArrayFile[counter] = sfile;
      counter = counter + 1
    end
  end
  
  if counter == 0
    exit 1
  end
  
  writer = File::open(outFileName,"wb")
  
  totalsize = counter * 4
  ArraySize.each do |size|
    writer.putc(totalsize)
    writer.putc(totalsize/256)
    writer.putc(totalsize/65536)
    writer.putc(totalsize/16777216)
    totalsize = totalsize + size
  end
  
  ArrayFile.each do |sfile|
    baseHandle = File::open(sfile)
    writer.write(baseHandle.read)
    baseHandle.close
  end
  
  writer.close


end
