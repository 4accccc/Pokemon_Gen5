



  ##CRC-CCITT = x16 + x12 + x5 + 1
  
  
#  (1) 初期値として、crcRegに0xFFFFを入れる
#(2) inDataに入力値（1バイト）を格納し、以下(3)～(6)を8回ループさせる
#(3) crcRegとinDataの排他的論理和を取り、最下位ﾋﾞｯﾄのみwに抽出。
#　　（w = crcReg ^ inData & 0x0001）
#(4) crcRegを右に1ビットシフトしcrcRegに格納 (crcReg = crcReg >> 1)
#(5) wが1であれば、crcRegと0x8408の排他的論理和を取った結果をcrcRegに格納する。
#　　wが0なら何もしない。
#(6) inDataを右に1ビットシフトする (inData = inData >> 1)
  
  
$CRC16Table = [
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0,
]

require "version.rb"

# (0:ブロックごとのcrcを埋め込む、1:全体のcrcを最後に付け足す)
$chk_mode = 0
#
# debug (0:off、1:on)
$debug_mode = 0;
#
# ruby or exe(0:ruby、1:exe)
$current_mode = $exe_mode

#BLOCK_SIZE = 0x120c	# 4620byte
#BLOCK_SIZE = 0x122c	# 4652byte
BLOCK_SIZE = 0x11e4	# 4580byte

# 使用方法メッセージ
$usage_str = [
  "\n使い方\nruby my_crc_ccitt.rb in.bin out.bin [-t]",    # ruby
  "\n使い方\nmy_crc_ccitt.exe in.bin out.bin [-t]",        # exe
]

#----------------------------------------------------------
#----------------------------------------------------------
#------------
# 引数チェック
#------------
if ARGV[0] == nil
  puts "引数1に入力ファイル名を指定してください"
  puts $usage_str[$current_mode]
  exit(1)
end
if ARGV[1] == nil
  puts "引数2に出力ファイル名を指定してください"
  puts $usage_str[$current_mode]
  exit(1)
end
#
# 存在チェック
unless FileTest.exists?(ARGV[0])
  puts "入力ファイル #{ARGV[0]} が見つかりません"
  exit(1)
end
#
# 全体のcrc用
if(ARGV[2] == '-t')
  $chk_mode = 1
end


# 個別処理
if($chk_mode == 0)

begin
  crcReg = 0xffff
  #
  #------------------
  # サイズを取得する
  #------------------
  i = 0
  fpr = File.new(ARGV[0],"rb")
  fpr.each_byte{| data |
    i += 1
  }
  fpr.close
  size = i;
  #
  if($debug_mode == 1)
    p sprintf("size :%d",size)
  end
  # ループの回数を求める
  num = size/BLOCK_SIZE;
  if($debug_mode == 1)
    p sprintf("loop num :%d",num)
  end
  #
  #--------------------
  # 読み込みと書き込み
  #--------------------
  fpr = File.new(ARGV[0],"rb")
  fpw = File.new(ARGV[1],"wb")
  #
  i = 0
  j = 0
  flg = 0
  crcReg = 0xffff
  fpr.each_byte{| data |
    if(flg == 0)
      inData = data
      fpw.write([data].pack("c"))
      #
      x = (crcReg >> 8)
      x = (x) ^ inData
      x = x & 0xff
      crcReg = (crcReg << 8) & 0xffff
      crcReg = crcReg ^ $CRC16Table[x]
    end
    #
    # 4byte前でブロック
    i += 1
    if(i == (j+1)*BLOCK_SIZE-4)
      fpw.write([crcReg].pack("i"))
#      fpw.write([0].pack("s"))
      flg = 1
    elsif(i == (j+1)*BLOCK_SIZE)
      flg = 0
      #
      j += 1
      if(j >= num)
        break;
      end
      # 初期化
      crcReg = 0xffff
    end
  }
  #
  fpr.close
#  fpw.write([crcReg].pack("s"))
#  fpw.write([crcReg].pack("i"))
  fpw.close
end

p sprintf("%s -> %s",ARGV[0], ARGV[1])
p sprintf("end")

end      # $chk_mode(0)


#-----------------------------------------------------
#-----------------------------------------------------
# 全体のcc用
if($chk_mode == 1)

begin
  crcReg = 0xffff
  
  fpr = File.new(ARGV[0],"rb")
  fpw = File.new(ARGV[1],"wb")
  
  # 読み込み
  fpr.each_byte{| data |
    inData = data
    fpw.write([data].pack("c"))
    
    x = (crcReg >> 8)
    x = (x) ^ inData
    x = x & 0xff
    crcReg = (crcReg << 8) & 0xffff
    crcReg = crcReg ^ $CRC16Table[x]
  }
  fpr.close
  
  # p crcReg.to_s(16)
#  fpw.write([crcReg].pack("s"))
  fpw.write([crcReg].pack("i"))
  fpw.close
end

p sprintf("%s -> %s",ARGV[0], ARGV[1])
p sprintf("end 2")

end       # chk_mode(1)


