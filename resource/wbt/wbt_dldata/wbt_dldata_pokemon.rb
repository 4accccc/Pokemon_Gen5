#!ruby -Ks
#---------------------------
# wbt_dldata_pokemon.rb
#---------------------------

#--------------
# 定義いろいろ
#--------------
TEMOTI_POKEMAX		= 6     	# 手持ちポケモン
NICKNAME_LEN		= 10+1		# ニックネーム
PTL_WAZA_MAX		= 4		# 技

#------------------------------------------------
#------------------------------------------------
class WBT_POKEMON_SINGLE
  #--------
  # 初期化
  #--------
  def initialize( conv, num )
    @conv        = conv;
    @poke_id     = num;
    #
    @monsno      = 254;
    #
    @nick_name = [];
    NICKNAME_LEN.times{|i|
      @nick_name[i] = 0;
    }
    @item_no     = 0;
    @rate_flag   = 0;    # pad注意
    @tokusei     = 0;
    @hp_exp      = 0;
    @pow_exp     = 0;
    @def_exp     = 0;
    @agi_exp     = 0;
    @spepow_exp  = 0;
    @spedef_exp  = 0;
    
    @waza_id = [];
    PTL_WAZA_MAX.times{|i|
      @waza_id[i] = 0;
    }
    @pp_count    = 0;	# bit注意
    @seikaku     = 0;
    @power_rnd   = 0;   # bit注意
    @form_no     = 0;
    @ball_id     = 0;
    @level       = 0;
    @friend_rate = 0;
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
  def pack()
    array = Array.new
    
    array << @monsno;
    #
    NICKNAME_LEN.times{|i|
      array << @nick_name[i];
    }
    array << @item_no;
    array << @rate_flag; # bit注意
    array << @tokusei;
    array << @hp_exp;
    array << @pow_exp;
    array << @def_exp;
    array << @agi_exp;
    array << @spepow_exp;
    array << @spedef_exp;
    
    PTL_WAZA_MAX.times{|i|
      array << @waza_id[i];
    }
    array << @pp_count;	# bit注意
    array << @seikaku;
    array << @power_rnd;   # bit注意
    array << @form_no;
    array << @ball_id;
    array << @level;
    array << @friend_rate;
    
    # debug
    if(@local_debug == 1)
      p sprintf("poke (%d) test :(size :0x%x)",@poke_id, (2+22+2+8+8+2+4+4))
    end
    
    # 出力形式をヘッダとあわせること
    return array.pack("S1S11S1C8S4C2IC4")
  end
  
end

#------------------------------------------------
#------------------------------------------------
class WBT_POKEMON
  #--------
  # 初期化
  #--------
  def initialize( conv )
    @conv = conv;
    #
    @pokemon = [];
    TEMOTI_POKEMAX.times{|i|
      @pokemon[i] = WBT_POKEMON_SINGLE.new( conv, i )
    }
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
  def pack()
    array = Array.new
    
    TEMOTI_POKEMAX.times{|i|
      array << @pokemon[i].pack
    }
    return array
  end
end

