#!ruby -Ks
#!/usr/bin/ruby
#
# レギュレーションをバイナリにするプログラム
# 2009.06.04 k.ohno
#
#

require "jcode"
require 'nkf'

COL_BASE_EVENTID = 0  #イベントＩＤ(最大2048件まで)
COL_BASE_ROMVERSION = 1 #	対象バージョン
COL_BASE_MESSAGE = 2 #	説明文
COL_BASE_EVENTTITLE = 3 #イベントタイトル
COL_BASE_TIME = 4	#受信した時間
COL_BASE_TYPE = 5   #	配信データ種類
COL_BASE_NO = 6	#ポケモン＆アイテム番号	
COL_BASE_FORM = 7 #フォルム
COL_BASE_ONCE = 8  #	一度だけの配信
COL_BASE_HAVE = 9  #	受け取ったフラグ
COL_BASE_RARE = 10 #	レア

COL_POKE_EVENTID = 0 #イベントＩＤ(最大2048件まで)
COL_POKE_PLAYERID  = 1 #	親(プレーヤー)のID
COL_POKE_ROMVERSION  = 2 #	作られたROMバージョン
COL_POKE_RIBBON  = 3 #	リボンビット 16本
COL_POKE_GETBALL  = 4 #	捕まえたボール
COL_POKE_HAVEITEM  = 5 #	もちもの
COL_POKE_TECHNIQUE1  = 6 #	技1
COL_POKE_TECHNIQUE2  = 7 #	技2
COL_POKE_TECHNIQUE3  = 8 #	技3
COL_POKE_TECHNIQUE4  = 9 #	技4
COL_POKE_MONSTER  = 10 #	モンスターNO
COL_POKE_FORM  = 11 #	フォルムNO
COL_POKE_LANGCODE  = 12 #	国コード
COL_POKE_NICKNAME  = 13 #	ニックネーム
COL_POKE_PERSONALITY = 14 #	性格
COL_POKE_SEX  = 15 #	性別
COL_POKE_ATTRIBUTE  = 16 #	とくせい
COL_POKE_RARE  = 17 #	レアにするかどうか 0=ランダムレアない  1=ランダム 2=レア
COL_POKE_GETPLACE  = 18 #	捕まえた場所
COL_POKE_BARTHPLACE  = 19 #	生まれた場所
COL_POKE_GETLEVEL  = 20 #	捕まえたLEVEL
COL_POKE_COOL  = 21 #	かっこよさ
COL_POKE_BEAUTIFUL  = 22 #	うつくしさ
COL_POKE_PRETTY  = 23 #	かわいさ
COL_POKE_SAGE  = 24 #	かしこさ
COL_POKE_STRONG  = 25 #	たくましさ
COL_POKE_GLOSS  = 26 #	毛艶
COL_POKE_RANDHP  = 27 #	HP乱数
COL_POKE_RANDSTR  = 28 #	攻撃力乱数
COL_POKE_RANDDEF  = 29 #	防御力乱数
COL_POKE_RANDSPEED  = 30 #	素早さ乱数
COL_POKE_RANDMAGIC  = 31 #	特攻乱数
COL_POKE_RANDSPIRIT  = 32 #	特防乱数
COL_POKE_PLAYERNAMESTR  = 33 #	親の名前
COL_POKE_PLAYERSEX  = 34 #	親の性別 0 1 2
COL_POKE_LEVEL  =  35 #	ポケモンレベル
COL_POKE_EGG  =  36 #	タマゴかどうか

COL_ITEM_EVENTID = 0    #イベントＩＤ(最大2048件まで)
COL_ITEM_NO = 1    #	道具番号
COL_ITEM_MOVIE = 2    #	映画配信かどうか






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

POKENUM_MAX_BYTE = (656/8)  ##このくらいに増えるかも ８２バイト
ITEMNUM_MAX_BYTE = (608/8)  ##このくらいにふえるかも




class RegulationBin
  
  def initialize
    @HashLevelRange = {"なし"=>0 ,"以上"=>1 ,"以下"=>2, "以上補正"=>3, "全補正"=>4, "以下補正"=>5}
    @HashOKNG = {"NG"=>0,"OK"=>1, "MANUAL"=>2}
    @HashONOFF = {"OFF"=>0,"ON"=>1, "MANUAL"=>2}
    @HashBATTLE = {"SINGLE"=>0,"DOUBLE"=>1, "TRIPLE"=>2, "ROTATION"=>3, "MULTI"=>4}
    @HashROM = {"WHITE"=>0,"BLACK"=>1, "GOLD"=>2, "SILVER"=>3, "PLATINUM"=>4, "DIAMOND"=>5, "PEARL"=>6, "EMERALD"=>7 , "RED"=>8 , "GREEN"=>9 , "RUBY"=>10 , "SAPPHIRE"=>11 }
    @TempNum = "0"
    @HashPokeGmm = Hash.new
    @HashItemGmm = Hash.new
    @HashWazaGmm = Hash.new
  end
  
  
  def bitingmm(names, outFH , max, gmmhash)
    bitarray = Array.new
    for i in 0..max
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
    for i in 0..max
      num = bitarray[i]
      outFH.write([num].pack("c"))
    end
    
    
    
  end
  
  def mojiout(value ,outFH , max)
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
      num =max - msg.length
      k=0
      while k < num
        outFH.putc(0)
        k=k+1
      end
   end



  
  def valuefunc(index , value, outFH)
    case index
    when COL_NO    #No
    when COL_CUPNAME    #カップ名
      msg = NKF.nkf('-m0 -S --utf16',value)
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
      num = 24 - msg.length
      k=0
      while k < num
        outFH.putc(0)
        k=k+1
      end
    when COL_RULENAME    #対戦ルール
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
      num = 24 - msg.length
      k=0
      while k < num
        outFH.putc(0)
        k=k+1
      end
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
        p keyno
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
      p value
      num = @HashBATTLE[value]
      outFH.write([num].pack("c"))
    else
    end
    
    
  end

  def valuefunc_poke(index , value, outFH)
    case index
    when COL_POKE_EVENTID  #イベントＩＤ(最大2048件まで)
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_PLAYERID   #	親(プレーヤー)のID
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_ROMVERSION   #	作られたROMバージョン



    when COL_POKE_RIBBON   #	リボンビット 16本
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_GETBALL   #	捕まえたボール
      if value =~ /なし/
        num = 0
        outFH.write([num].pack("s"))
      else
        keyno = @HashItemGmm.index(value).to_i
        outFH.write([keyno].pack("s"))
      end
    when COL_POKE_HAVEITEM   #	もちもの
      if value =~ /なし/
        num = 0
        outFH.write([num].pack("s"))
      else
        keyno = @HashItemGmm.index(value).to_i
        outFH.write([keyno].pack("s"))
      end
    when COL_POKE_TECHNIQUE1   #	技1
      if value =~ /なし/
        num = 0
        outFH.write([num].pack("s"))
      else
        keyno = @HashWazaGmm.index(value).to_i
        outFH.write([keyno].pack("s"))
      end
    when COL_POKE_TECHNIQUE2   #	技2
      if value =~ /なし/
        num = 0
        outFH.write([num].pack("s"))
      else
        keyno = @HashWazaGmm.index(value).to_i
        outFH.write([keyno].pack("s"))
      end
    when COL_POKE_TECHNIQUE3   #	技3
      if value =~ /なし/
        num = 0
        outFH.write([num].pack("s"))
      else
        keyno = @HashWazaGmm.index(value).to_i
        outFH.write([keyno].pack("s"))
      end
    when COL_POKE_TECHNIQUE4   #	技4
      if value =~ /なし/
        num = 0
        outFH.write([num].pack("s"))
      else
        keyno = @HashWazaGmm.index(value).to_i
        outFH.write([keyno].pack("s"))
      end
    when COL_POKE_MONSTER   #	モンスターNO
      if value =~ /なし/
        num = 0
        outFH.write([num].pack("s"))
      else
        keyno = @HashPokeGmm.index(value).to_i
        outFH.write([keyno].pack("s"))
      end
    when COL_POKE_FORM   #	フォルムNO
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_LANGCODE   #	国コード
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_NICKNAME   #	ニックネーム
	mojiout(value, outFH, )

    when COL_POKE_PERSONALITY  #	性格


    when COL_POKE_SEX   #	性別
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_ATTRIBUTE  #	とくせい


    when COL_POKE_RARE   #	レアにするかどうか 0=ランダムレアない  1=ランダム 2=レア
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_GETPLACE   #	捕まえた場所

    when COL_POKE_BARTHPLACE   #	生まれた場所

    when COL_POKE_GETLEVEL  #	捕まえたLEVEL
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_COOL   #	かっこよさ
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_BEAUTIFUL   #	うつくしさ
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_PRETTY   #	かわいさ
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_SAGE   #	かしこさ
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_STRONG   #	たくましさ
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_GLOSS   #	毛艶
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_RANDHP   #	HP乱数
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_RANDSTR   #	攻撃力乱数
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_RANDDEF   #	防御力乱数
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_RANDSPEED  #	素早さ乱数
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_RANDMAGIC   #	特攻乱数
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_RANDSPIRIT   #	特防乱数
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_PLAYERNAMESTR   #	親の名前
    when COL_POKE_PLAYERSEX   #	親の性別 0 1 2
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_LEVEL   #	ポケモンレベル
      num = value.to_i
      outFH.write([num].pack("c"))
    when COL_POKE_EGG   #	タマゴかどうか
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
    outHeader.puts("//reguration.rb reguration.xls からの自動生成です")
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
      outFH = File.new("reguration" + sprintf("%03d",lineindex) + ".bin","wb")
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
  
  #GMMファイルから必要な文字列テーブルものを読み込み
  def gmmread( gmmfile , hashgmm)
    fpr = File.new(gmmfile,"r")
    pokeno = 0
    fpr.each{ |line|
      if line =~ /<row\s+id="MONSNAME_(\d+)">/
        pokeno = $1
      elsif line =~ /<row\s+id="ITEMNAME_(\d+)">/
        pokeno = $1
      elsif line =~ /<row\s+id="WAZANAME_(\d+)">/
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
  
  
  def allgmmread(pokegmm,itemgmm, wazagmm)
  
    gmmread(pokegmm, @HashPokeGmm)
    gmmread(itemgmm, @HashItemGmm)
    gmmread(wazagmm, @HashWazaGmm)

  end
  
end

begin
  
  
  
  
  dep = RegulationBin.new
  dep.allgmmread(ARGV[3],ARGV[4],ARGV[5])
  dep.convline( ARGV[0],ARGV[1],ARGV[2] )
  
  
  exit(0)
end
