#!ruby -Ks
#!/usr/bin/ruby
#
# カップ名＋対戦ルールは番号管理にした
# 2010.01.06 k.ohno
# レギュレーションをバイナリにするプログラム
# 2009.06.04 k.ohno
#
#

require "jcode"
require 'nkf'


COL_NO = 0    #No
COL_CUPNAME = 1    #カップ名
COL_RULENAME = 2    #対戦ルール
COL_PROGRAM_LAVEL = 3    #プログラムで使うラベル
COL_NUM_LO = 4    #参加数下限
COL_NUM_HI = 5    #参加数上限
COL_LEVEL = 6    #参加レベル
COL_LEVEL_RANGE = 7    #レベル範囲
COL_LEVEL_TOTAL = 8    #レベル合計
COL_BOTH_POKE = 9    #同じポケモン
COL_BOTH_ITEM = 10    #同じどうぐ
COL_VETO_POKE = 11    #参加禁止ポケモン
COL_VETO_ITEM = 12    #持ち込み禁止道具
COL_MUST_POKE = 13    #必須ポケモン
COL_MUST_POKE_FORM = 14    #必須ポケモンフォルム
COL_SHOOTER = 15    #シューター
COL_TIME_VS = 16    #対戦時間
COL_TIME_COMMAND = 17    #入力時間
COL_NICKNAME = 18    #ニックネーム表示
COL_AGE_LO = 19    #年齢制限以上
COL_AGE_HI = 20    #年齢制限以下
COL_SHOW_POKE = 21    #ポケモン見せ合い
COL_TIME_SHOW_POKE = 22 #ポケモン見せ合い時間
COL_BTL_TYPE = 23 #バトルタイプ
COL_ROM_CODE = 24 #ROMコード
COL_WORLDCUP_NAME = 25 #大会名
COL_WORLDCUP_NO = 26 #大会番号
COL_LANG_CODE = 27 #言語コード
COL_START_YEAR = 28 #開始年
COL_START_MONTH = 29 #開始月
COL_START_DAY = 30 #開始日
COL_END_YEAR = 31 #終了年
COL_END_MONTH = 32 #終了月
COL_END_DAY = 33 #終了日



POKENUM_MAX_BYTE = (656/8)  ##このくらいに増えるかも ８２バイト
ITEMNUM_MAX_BYTE = (608/8)  ##このくらいにふえるかも




class RegulationBin
  
  def initialize
    
    @HashCupRange = {"シングルバトル"=>0 ,"ダブルバトル"=>1 ,"トリプルバトル"=>2,
      "ローテーションバトル"=>3, "マルチバトル"=>4, "シューターバトル"=>5 , "デバッグバトル"=>6 }
    @HashRuleRange = {"レベル５０"=>0 ,"せいげんなし"=>1 ,"スタンダード"=>2, "ランダムマッチ"=>3, "バトルサブウェイ"=>4, "フラット"=>5}
    @HashLevelRange = {"なし"=>0 ,"以上"=>1 ,"以下"=>2, "以上補正"=>3, "全補正"=>4, "以下補正"=>5}
    @HashOKNG = {"NG"=>0,"OK"=>1, "MANUAL"=>2}
    @HashONOFF = {"OFF"=>0,"ON"=>1, "MANUAL"=>2}
    @HashBATTLE = {"SINGLE"=>0,"DOUBLE"=>1, "TRIPLE"=>2, "ROTATION"=>3, "MULTI"=>4}

    @HashCOUNTRY = {"JAPAN"=>1,"ENGLISH"=>1, "FRANCE"=>2, "ITALY"=>3, "GERMANY"=>4, "SPAIN"=>5, "KOREA"=>6}
    @HashROM = {"ALL"=>0xffffffff,"SAPPHIRE"=>0x2, "RUBY"=>0x4, "EMERALD"=>0x08, "RED"=>0x10,
      "GREEN"=>0x20, "GOLD"=>0x80, "SILVER"=>0x100,"DIAMOND"=>0x800, "PEARL"=>0x1000, "PLATINUM"=>0x2000,
      "COLOSSEUM"=>0x10000, "WHITE"=>0x100000, "BLACK"=>0x200000}
    
    @TempNum = "0"
    @HashPokeGmm = Hash.new
    @HashItemGmm = Hash.new
  end
  
  
  def bitingmm(names, outFH , max, gmmhash)
    bitarray = Array.new
    maxmai = max - 1
    
    for i in 0..maxmai
      bitarray.push(0)
    end
    
    byteorder = Array.new([1,2,4,8,16,32,64,128])
    
    
    arrayname = names.split(',')
    
    arrayname.each{|monsname|
      
      keyno = gmmhash.index(monsname).to_i
      if keyno == nil
        p "不明なポケモンかアイテムがいたので停止します " + monsname
        exit(-1)
      end
      bitarray[keyno / 8]  = bitarray[keyno / 8] + byteorder[keyno % 8]
    }
    for i in 0..maxmai
      num = bitarray[i]
      outFH.write([num].pack("c"))
    end
    
    
    
  end
  
  
  
  def bitinrom(names, outFH , gmmhash)
    
    keynoall=0
    arrayname = names.split(',')
    
    arrayname.each{|romname|
      keyno = gmmhash[romname]
      if keyno == nil
        p "不明なROMコードがあったので停止します " + romname
        exit(-1)
      end
      keynoall  = keynoall + keyno
    }

    outFH.write([keynoall].pack("l"))
    
  end
  
  
  
  
  def valuefunc(index , value, outFH)
    case index
    when COL_NO    #No
    when COL_CUPNAME    #カップ名
      num = @HashCupRange[value]
      outFH.write([num].pack("c"))
    when COL_RULENAME    #対戦ルール
      num = @HashRuleRange[value]
      outFH.write([num].pack("c"))
    when COL_NUM_LO    #参加数下限
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_NUM_HI    #参加数上限
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_LEVEL    #参加レベル
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_LEVEL_RANGE    #レベル範囲
      num = @HashLevelRange[value]
      outFH.write([num].pack("c"))
    when COL_LEVEL_TOTAL    #レベル合計
      num = value.to_i
      outFH.write([num].pack("s"))
    when COL_BOTH_POKE    #同じポケモン
      num = @HashOKNG[value]
      outFH.write([num].pack("c"))
    when COL_BOTH_ITEM    #同じどうぐ
      num = @HashOKNG[value]
      outFH.write([num].pack("c"))
    when COL_VETO_POKE    #参加禁止ポケモン
      if value =~ /なし/
        num = 0
        for i in 1..POKENUM_MAX_BYTE
          outFH.write([num].pack("c"))
        end
      else
        bitingmm(value,outFH, POKENUM_MAX_BYTE ,@HashPokeGmm)
      end        
    when COL_VETO_ITEM    #持ち込み禁止道具
      if value =~ /なし/
        num = 0
        for i in 1..ITEMNUM_MAX_BYTE 
          outFH.write([num].pack("c"))
        end
      else
        bitingmm(value,outFH, ITEMNUM_MAX_BYTE ,@HashItemGmm)
      end        
    when COL_MUST_POKE    #必須ポケモン
      if value =~ /なし/
        num = 0
        outFH.write([num].pack("s"))
      else
        keyno = @HashPokeGmm.index(value).to_i
        outFH.write([keyno].pack("s"))
      end
    when COL_MUST_POKE_FORM    #必須ポケモンフォルム
      num = value.to_i
      outFH.write([num].pack("c")) 
    when COL_SHOOTER    #シューター
      num = @HashOKNG[value]
      outFH.write([num].pack("c"))
    when COL_TIME_VS    #対戦時間
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_TIME_COMMAND    #入力時間
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_NICKNAME    #ニックネーム表示
      num = @HashONOFF[value]
      outFH.write([num].pack("c"))
    when COL_AGE_LO    #年齢制限以上
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_AGE_HI    #年齢制限以下
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_SHOW_POKE    #ポケモン見せ合い
      num = @HashOKNG[value]
      outFH.write([num].pack("c"))
    when COL_TIME_SHOW_POKE    #ポケモン見せ合い時間
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_BTL_TYPE  #バトルタイプ
      num = @HashBATTLE[value]
      outFH.write([num].pack("c"))
    when COL_WORLDCUP_NAME    #カップ名
      msg = NKF.nkf("-m0 -S --utf16",value)
      i = 0
      mojibyte1 = 0
      msg.each_byte{ | mojibyte |
        if i % 2 == 0
          mojibyte1 = mojibyte
        else
          outFH.putc( mojibyte )
          outFH.putc( mojibyte1 )
        end
        i=i+1
      }
      outFH.putc(0xff)  ##終端
      outFH.putc(0xff)
      num = 144 - msg.length
      k=0
      while k < num
        outFH.putc(0)
        k=k+1
      end
    when COL_ROM_CODE  #ROMコード
      if value == "ALL"
        num = 0xffffffff
        outFH.write([num].pack("l"))
      else
        bitinrom(value, outFH, @HashROM)
      end
    when COL_WORLDCUP_NO    #カップNo
      num = value.to_i
      outFH.write([num].pack("s"))
    when COL_LANG_CODE  #言語コード
      num = @HashCOUNTRY[value]
      outFH.write([num].pack("c"))
    when COL_START_YEAR  #開始年
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_START_MONTH  #開始月
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_START_DAY  #開始日
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_END_YEAR  #終了年
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_END_MONTH  #終了月
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_END_DAY  #終了日
      num = value.to_i
      outFH.write([num].pack("c"))
      
      ##ステータスは0
      num = 0
      outFH.write([num].pack("c"))
      outFH.write([num].pack("s"))
      
    else
    end
    
    
  end

  
  
  def programfunc(index , value, outFH)
    case index
    when COL_NO               # No
      @TempNum = value
      
    when COL_PROGRAM_LAVEL    # Program
      num = value.to_i
      num = @TempNum.to_i
      outFH.puts( "#define REG_" + value + " " +  num.to_s)
    else
    end
  end

  def headerplus(outHeader)
    outHeader.puts("//レギュレーションのデータ引き出しラベルです")
    outHeader.puts("//regulation.rb regulation.xls からの自動生成です")
    outHeader.puts("#pragma once")
  end
  
  
  def convline( tabfile, outfile,outheader )
    
    outHeader = File.new(outheader,"w")
    
    headerplus(outHeader)
    
    fpr = File.new(tabfile,"r")
    lineindex=0
    fpr.each{ |line|
      index = 0
      if line =~ /^\d+/
      elsif line =~ /^No/
        next
      else
        break
      end
      outFH = File.new("regulation" + sprintf("%03d",lineindex) + ".bin","wb")
      tabarray = line.split(/\t+/)
      tabarray.each{ |value|
        valuefunc(index, value, outFH)
        programfunc(index, value, outHeader)
        index = index + 1
      }
      outFH.close
      lineindex = lineindex + 1
    }
    outHeader.close
    fpr.close
    
  end
  
  
  def gmmread( gmmfile , hashgmm)
    fpr = File.new(gmmfile,"r")
    pokeno = 0
    fpr.each{ |line|
      if line =~ /<row\s+id="MONSNAME_(\d+)">/
        pokeno = $1
      elsif line =~ /<row\s+id="ITEMNAME_(\d+)">/
        pokeno = $1
      elsif pokeno != 0
        line = NKF.nkf('-m0 -Ws',line)
        if line =~ /<language.name=.JPN.*>(.*)<\/language>/
          monsname = $1
          hashgmm.store(pokeno, monsname)
        end
        pokeno = 0
      end
    }
    fpr.close
  end
  
  
  def allgmmread(pokegmm,itemgmm)
  
    gmmread(pokegmm, @HashPokeGmm)
    gmmread(itemgmm, @HashItemGmm)
  end
  
  
  
end

begin
  
  
  
  
  dep = RegulationBin.new
  dep.allgmmread(ARGV[3],ARGV[4])
  dep.convline( ARGV[0],ARGV[1],ARGV[2] )
  
  
  exit(0)
end
