#!/usr/bin/ruby
#
#  make_prog_filesから .oを抜き出してファイルにします
#
# OBJS_OVERLAY=./obj/ARM9-TS.HYB/Release/net_devwifi.o ./obj/ARM9-TS.HYB/Release/dwc_rap.o 
# #
#



class MPFRead

  
  
  
  def FileRead( mpfname, convFileLine )
    ignoreflg = 0
    
    fpr = File.new(mpfname)
    
    fpr.each{ |line|
      if /^SRCS_OVERLAY.*/ =~ line    ### オーバーレイ定義が出るまで無視
        ignoreflg=1
      end
      if /^#/ =~ line    ### コメントは無視
      elsif ignoreflg==0
      elsif /(\w+\.c)/ =~ line
        convFileLine.push($1)
      end
    }
    fpr.close
  end
  
  
  def FileWrite( outname, convFileLine, dirname)
    
    fpw = File.new(outname,"w")
    fpw.write("OBJS_OVERLAY=")
    
    convFileLine.each{ |line|
      sourcename = File::basename(line,'.*') 
      fpw.write("./obj/ARM9-TS/Release/" + sourcename + ".o ")
    }
    fpw.puts("")
    fpw.close
  end
  
  
  
  
end







### メイン
begin
  
  ConvFileLine = Array.new
  
  mpfhandle = MPFRead.new
  mpfhandle.FileRead( ARGV[0], ConvFileLine )  ##コンバート対象ファイルを読み込む
  mpfhandle.FileWrite( ARGV[1], ConvFileLine,"./obj/ARM9-TS/Release/")
  mpfhandle.FileWrite( ARGV[2], ConvFileLine,"./obj/ARM9-TS.HYB/Release/")
  
  
  #  MPFRead.TagConv()   ## タグを置き換える
#  MPFRead.FileWrite( ARGV[1])  ##コンバート結果を出力する
  
  exit(0)
end
