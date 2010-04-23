#!/usr/bin/ruby -Ks
# naixのenumにmaxを追加する 2010/04/22 nagihashi

require 'ftools'
require 'FileUtils'


def naixaddmax(baseHandle, targetHandle)  
  searchHandle  = baseHandle
  define_name   = "";

#defineのあとの定義を貰う
  searchHandle.each {|baseline|
    if /#define/ =~ baseline

      define_name = baseline.strip
      define_name = define_name.gsub( "#define ", "" )
      define_name = define_name.gsub( "NAIX_", "MAX" )

      p define_name
    end
  }

#};の前に挿入する
  baseHandle.rewind
  baseHandle.each {|baseline|
    baseline  = baseline.gsub( "\r","" )

    if / = (\d)+$/ =~ baseline
      p baseline
      baseline  = baseline.sub( /$/,"," )
    end

    if /\};/ =~ baseline
      targetHandle.puts("\t"+define_name)
    end
    targetHandle.puts(baseline)
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
        naixaddmax(baseHandle, targetHandle)
        baseHandle.close
        targetHandle.close
        
        FileUtils.rm(sfile)
        FileUtils.mv(tempname,sfile)
        
      end
    end
  end


end
