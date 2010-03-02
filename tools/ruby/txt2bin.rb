#!ruby -Ks
#!/usr/bin/ruby
#
# txt‚Ì‚P‚Ui”‚ğbin‚É‚·‚é
# 2009.06.04 k.ohno
#
#

require "jcode"
require 'nkf'



class Tex2Bin
 
  def spacesprit(filenames, array )
    fpr = File.new(filenames,"r")
    
    fpr.each{|line|
      line.split(' ').each{|sen|
        array.push(sen)
      }
    }
    fpr.close
    return array
    
    
  end
  
  def binout( value, type, outFH)
    case type
    when 8
      num = value.hex
      outFH.write([num].pack("c"))
    when 16
      num = value.hex
      outFH.write([num].pack("s"))
    when 32
      num = value.hex
      outFH.write([num].pack("l"))
    else
    end
  end

end

begin
  
  
  
  
  dep = Tex2Bin.new
  array = Array.new
  
  
  
  dep.spacesprit(ARGV[0] , array)
  
  outFH = File.new(ARGV[1],"w+")
  
  array.each{|value|
    dep.binout( value, 8, outFH)
    p value
  }
  
  outFH.close
  
  exit(0)
end
