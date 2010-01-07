



  ##CRC-CCITT = x16 + x12 + x5 + 1
  
  
#  (1) 初期値として、crcRegに0xFFFFを入れる
#(2) inDataに入力値（1バイト）を格納し、以下(3)～(6)を8回ループさせる
#(3) crcRegとinDataの排他的論理和を取り、最下位ﾋﾞｯﾄのみwに抽出。
#　　（w = crcReg ^ inData & 0x0001）
#(4) crcRegを右に1ビットシフトしcrcRegに格納 (crcReg = crcReg >> 1)
#(5) wが1であれば、crcRegと0x8408の排他的論理和を取った結果をcrcRegに格納する。
#　　wが0なら何もしない。
#(6) inDataを右に1ビットシフトする (inData = inData >> 1)
  
  

  
 
begin
  crcReg = 0xffff
  
  fpr = File.new(ARGV[0],"rb")
  fpw = File.new(ARGV[1],"wb")
    
  fpr.each_byte{| data |
    inData = data
    fpw.write([data].pack("c"))
    
    
    for i in 1..8 
      w = (crcReg ^ inData) & 0x0001
      crcReg = crcReg >> 1
      if w == 1
        crcReg = crcReg ^ 0x8408
      end
      inData = inData >> 1
    end
  }
  fpr.close
  
  fpw.write([crcReg].pack("s"))
  fpw.close
  
end


