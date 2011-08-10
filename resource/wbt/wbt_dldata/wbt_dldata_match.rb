#!ruby -Ks
#---------------------------
# wbt_dldata_match.rb
#
#---------------------------

require "wbt_dldata_util.rb"

WBT_MATCH_MAX	 = 7;	# 組み合わせデータ(WBT_TRAINER_NUM-1)

#------------------------------------------------
#------------------------------------------------
#------------------
# 組み合わせ(単体)
#------------------
class WBT_MATCH_SINGLE
  #--------
  # 初期化
  #--------
  def initialize( conv, num )
    @conv = conv;
    @match_id = num;
    #
    @type        = 255;
    @pri         = 0;
    @id          = 0;
    #
    @local_debug = 0;
    # debug
    if($debug_code == 1)
      @local_debug = 1;
    end
  end
  
  #----------------
  # データのパック
  #----------------
  def pack2()
    array = Array.new
    
    array << @type;
    array << @pri;
    array << @id;
    #
    # debug
    if(@local_debug == 1)
      p sprintf("match (%d) test (size :0x%x)",@match_id, (2+2))
    end
    
    # 出力形式をヘッダとあわせること
    return array.pack("C2S")
  end
  
  #----------------
  # データのパック
  #----------------
  def pack()
    array = Array.new
    
    array << pack2();
    
    return array
  end
end

#------------------------------------------------
#------------------------------------------------
#------------------
# 組み合わせ(まとめ)
#------------------
class WBT_MATCH
  #--------
  # 初期化
  #--------
  def initialize( conv )
    @conv = conv;
    #
    @match = [];
    WBT_MATCH_MAX.times{|i|
      @match[i] = WBT_MATCH_SINGLE.new( conv, i )
    }
    @crc = 0;
    #
    @local_debug = 0;
    # debug
    if($debug_code == 1)
      @local_debug = 1;
    end
  end
  
  #----------------
  # CRC(仮)
  #----------------
  def pack3()
    array = Array.new
    
    array << @crc
    
    # debug
    if(@local_debug == 1)
      p sprintf("data test (crc) (size : 0x%x)",(4))
    end
    
    return array.pack("I")
  end
  
  #----------------
  # データのパック
  #----------------
  def pack()
    array = Array.new
    
    WBT_MATCH_MAX.times{|i|
      array << @match[i].pack
    }
    array << pack3();
    
    return array
  end
  
end

