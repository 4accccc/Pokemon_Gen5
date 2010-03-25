#!ruby -Ks
#!/usr/bin/ruby
#
# CGEARパターンをデータ化
# 2010.03.18 k.ohno
#
#

require "jcode"
require 'nkf'





class PanelPattern
  
  def initialize(str)
    @PatternName = str
    @ArrayData=[]
    @HashPattern={"●"=>1, "■"=>2, "▲"=>3, "□"=>4, "◇"=>5, "旗"=>6 }
  end
  
  
  def panelRead(line, no)
    
    buffer = line.split(',')
    buffer.each{| pattern |
      
      @HashPattern.each{ |key1,value1|
     #   if pattern =~ /#{key1}/
        
        
     #   end
      }
    
    }
    
  end
  
  
  
  
  
  
end

begin
  
  
  fh = File.new(ARGV[0],"r")
  loopflg = 0
  dep = PanelPattern.new("pattern0")
  
  fh.each{ |line|
    if line =~ /^pattern(\d+)/
      dep = PanelPattern.new("pattern"+$1)
      loopflg = 1
    elsif loopflg < 7
      dep.panelRead( line ,loopflg)
      loopflg=loopflg+1
    end
  }
  exit(0)
end
