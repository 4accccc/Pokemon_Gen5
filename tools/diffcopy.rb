#!/usr/bin/ruby -Ks
# ファイル内容を比較して違っていたらコピーする
#  > diffcopy.rb コピー元ファイル... コピー先ディレクトリ
#    ※ コピー元ファイルは複数指定できる
#  このツールは動作が重いのが難点
#  姉妹品のdiffcopyはshellで書かれているので早いですが
#  こちらはnaixの日付を読み飛ばして比較しています

require 'ftools'


def naixcheck(baseHandle, targetHandle)
  
  baseHandle.each {|baseline|
    targetline = targetHandle.gets()
    if /^\tUpdate:/ =~ baseline
    elsif targetline == baseline
    else
      return false
    end
  }
  return true
end


def nanrHeaderCheck(baseHandle, targetHandle)
  
  baseHandle.each {|baseline|
    targetline = targetHandle.gets()
    if /creation date ==>/ =~ baseline
    elsif targetline == baseline
    else
      return false
    end
  }
  return true
end


begin
  exit 1 if ARGV.size < 2

  destdir = ARGV.pop
  exit 1 unless FileTest.directory?(destdir)
  destdir = destdir.sub(/\/$/, '') # 最後に / があれば取る

  ARGV.each do |sfile|
    basename = File.basename(sfile)
    extname = File.extname(sfile)
    target = destdir + '/' + basename
    nIgnore = false
    if FileTest.exist?(target)      # コピー先が既に存在し、
      if extname == ".naix" #naixなら独自検査
        baseHandle = File::open(basename)
        targetHandle = File::open(target)
        nIgnore = naixcheck(baseHandle, targetHandle)
        baseHandle.close
        targetHandle.close
      elsif /LBLDEFS.h/ =~ basename #hならLBLDEFS.hとみなして独自検査
        baseHandle = File::open(basename)
        targetHandle = File::open(target)
        nIgnore = nanrHeaderCheck(baseHandle, targetHandle)
        baseHandle.close
        targetHandle.close
      elsif File.compare(sfile, target)   # 内容が同じだったら
        nIgnore = true
      else
      end
    end
    if nIgnore == false
      #→コピーする
      File.copy(sfile, target)
      puts "copied: #{basename} -> #{target}"
    end
  end

  exit 0
end
