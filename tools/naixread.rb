#!/usr/bin/ruby -Ks
#---------------------------------------------------------
#
# *.naixを読んで配列に格納するためのクラス
#
# 2009.05.12  tamada
#
#---------------------------------------------------------

class NAIXReader

  class NAIXReaderInputError < Exception; end

  def initialize( inputHandle )
    @index = nil
    @arc_name = nil
    readNaix( inputHandle )
  end

  #ファイル名を指定してNAIXReaderを生成する
  def NAIXReader.read( filename )
    File.open(filename){|file|
      return new(file)
    }
  end

  #定義配列を取得する
  def getArray
    @index
  end

  #アーカイブ名を取得する
  def getArchiveName
    @arc_name
  end

  def getIndex( name )
    if name =~/^NARC_/ then
      return @index.index(name)
    else
      p = "NARC_" + @arc_name + "_" + name
      #puts "seach short name :#{name} -->:#{p}"
      return @index.index(p)
    end
  end

  ###以下は外部から呼べない関数
  private

  #*.naixファイルを解釈して配列に格納
  def readNaix( inputHandle )
    inside_flag = false
    count = 0
    @index = Array.new

    inputHandle.each{|line|
      column = line.split
      case line
      when /#define /
        @arc_name = column[1].sub(/\ANARC_/,"").sub(/_NAIX_/,"").downcase
        
      when /^enum \{/
        inside_flag = true

      when /^\tNARC_/
        if inside_flag == false || column[1] != "=" then
          raise NAIXReaderInputError, "#{line}"
        end
        if count != Integer(column[2].sub(/,/,"")) then
          raise NAIXReaderInputError, "#{line}"
        end
        @index << column[0]
        count += 1

      when /\};$/
        inside_flag = false
      else
        #puts "ELSE LINE:#{line}"
      end
    }
    @index
  end

end

#---------------------------------------------------------
#---------------------------------------------------------
begin
  if __FILE__ == $0 then
    puts "使い方："
    puts "#require命令でスクリプトに取り込んで使用してください"
    puts "reader = NAIXRead.read(filename)"
    puts "array = reader.get  #配列で取得"
    puts "array.each{....     #適当に使用する"
  end
end

