#!/usr/bin/ruby -Ks
# naixのタイムスタンプ部分を削除する naixファイルは複数指定可能
# basename を sfileに変更 2010/03/17 k.ohno

require 'ftools'
require 'FileUtils'


def naixcopy(baseHandle, targetHandle)
  
  baseHandle.each {|baseline|
    if /^\tUpdate:/ =~ baseline
      targetHandle.puts("")
    else
      targetHandle.puts(baseline)
    end
  }
end

begin
  exit 1 if ARGV.size < 1

  ARGV.each do |sfile|
    basename = File.basename(sfile)
    extname = File.extname(sfile)
    tempname = basename + ".tmp"
    if FileTest.exist?(sfile)
      if extname == ".naix" #naixならスタンプを消す
        baseHandle = File::open(sfile)
        targetHandle = File::open(tempname,"w")
        naixcopy(baseHandle, targetHandle)
        baseHandle.close
        targetHandle.close
        
        FileUtils.rm(sfile)
        FileUtils.mv(tempname,sfile)
        
      end
    end
  end


end
