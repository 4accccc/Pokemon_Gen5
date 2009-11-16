#!/usr/bin/ruby

# アトリビュートファイル
# 2009.11.13 k.ohno
#
# 使用方法：convAttr24.rb imput.bin output.bin
#
#  xyz xyz dd attr value tribitの並びを
#
#  xyz xyz dd  attr value と
#  tribitarray  に変更する
#
STR_USAGE = "use: ruby convAttr24.rb imput.bin output.bin"



###
###  ビルドモデルの単体クラス
###
class AttrData
  
  
  def initialize
    @X1=0
    @Y1=0
    @Z1=0
    @X2=0
    @Y2=0
    @Z2=0
    @D1=0
    @D2=0
    @attrPattern=0
    @value=0
  end
  
  def getData(inFH)
    @X1=inFH.read(2).unpack("s")
    @Y1=inFH.read(2).unpack("s")
    @Z1=inFH.read(2).unpack("s")
    @X2=inFH.read(2).unpack("s")
    @Y2=inFH.read(2).unpack("s")
    @Z2=inFH.read(2).unpack("s")
    @D1=inFH.read(4).unpack("l")
    @D2=inFH.read(4).unpack("l")
    @attrPattern=inFH.read(2).unpack("s")
    @value=inFH.read(2).unpack("s")
  end
  
  
  def outData1(outFH)
    outFH.write(@X1.pack("s"))
    outFH.write(@Y1.pack("s"))
    outFH.write(@Z1.pack("s"))
    outFH.write(@X2.pack("s"))
    outFH.write(@Y2.pack("s"))
    outFH.write(@Z2.pack("s"))
    outFH.write(@D1.pack("l"))
    outFH.write(@D2.pack("l"))
    outFH.write(@attrPattern.pack("c"))
    outFH.write(@value.pack("c"))
  end
  
  
  def outData2(outFH)
#    outFH.write(@attrPattern.pack("B1"))
  end
  
  def GetTriBit()
    num = @value[0]
    if num < 0
      return 1
    else
      return 0
    end
  end
  
  
end



### ファイルの入力

def FileRead( file, inarray, xyarray )
  
  fpr = File.new(file,"rb")
  x = fpr.read(2).unpack("s")
  y = fpr.read(2).unpack("s")
  xn = x[0]
  yn = y[0]
  
  xyarray.push(xn)
  xyarray.push(yn)
  
  loopnum = xn * yn
  for i in 1..loopnum
    attrd = AttrData.new
    attrd.getData( fpr )
    inarray.push(attrd)
  end
  fpr.close
  return loopnum
end


### ファイルの出力
### @param  シート
### @return 不定
def OutFile(dp3Array, file , xyarray )
  
  outFH = File.new(file,"wb")
  
  x = xyarray[0]
  y = xyarray[1]
  
  outFH.write([x].pack("s"))
  outFH.write([y].pack("s"))
  
  dp3Array.each{ | bm3dp |
    bm3dp.outData1(outFH)
  }
  
  cnt = 0
  bitshift = 0
  dp3Array.each{ | bm3dp |
    bit = bm3dp.GetTriBit()
    bitshift = bitshift << 1
    bitshift = bitshift + bit
    cnt = cnt + 1
    if cnt==8
      ar = Array.new
      ar.push(bitshift)
      outFH.write(ar.pack("c"))
      bitshift=0
      cnt=0
    end
  }
  outFH.close
end




### メイン
begin
    
  rarray = Array.new
  xyarray = Array.new
    
  length = FileRead(ARGV[0],rarray,xyarray )
  OutFile(rarray,ARGV[1],xyarray)
  
end


