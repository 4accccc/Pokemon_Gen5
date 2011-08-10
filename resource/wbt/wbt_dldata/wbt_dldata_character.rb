#!ruby -Ks
#---------------------------
# wbt_dldata_character.rb
#---------------------------
require "wbt_dldata_pokemon.rb"

#--------------
# 定義いろいろ
#--------------
WBTDL_CHAR_MAX		= 4	# キャラ
SAVELEN_TRAINER_NAME	= 14	# 名前
WBTDL_TRAINER_TITLE_LEN	= 22	# タイトル
WBTDL_MSG_LEN		= 73	# メッセージ
#
#--------------------------------------------------------------
#--------------------------------------------------------------
#------------------
# キャラ(単体)
#------------------
class WBT_CHARACTER_SINGLE
  #--------
  # 初期化
  #--------
  def initialize( conv, num )
    @conv = conv;
    @char_id = num;
    #
    @pokemon = WBT_POKEMON.new( conv )
    #
    @name = [];
    SAVELEN_TRAINER_NAME.times{|i|
      @name[i] = 0x7777;
    }
    @title = [];
    WBTDL_TRAINER_TITLE_LEN.times{|i|
      @title[i] = 0;
    }
    @msg_before = [];
    WBTDL_MSG_LEN.times{|i|
      @msg_before[i] = 0;
    }
    @msg_win = [];
    WBTDL_MSG_LEN.times{|i|
      @msg_win[i] = 0;
    }
    @msg_lose = [];
    WBTDL_MSG_LEN.times{|i|
      @msg_lose[i] = 0;
    }
    @msg_announce = [];
    WBTDL_MSG_LEN.times{|i|
      @msg_announce[i] = 0;
    }
    #
    @type_id             = 253;
    @msg_before_id        = 0;
    @msg_after_win_id     = 0;
    @msg_after_lose_id    = 0;
    @msg_win_id           = 0;
    @msg_lose_id          = 0;
    @msg_announce_id      = 0;
    @lang_id              = 0;
    @ai_bit               = 0x8888;
    @pad                  = 0;
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
    #
    # 型が異なるものは、
    # packは別で処理しないとだめらしい
    #--------
    # ポケモン
    #--------
    array << @pokemon.pack
    #
    array << pack2();
    #
    return array;
  end
  
  #----------
  # キャラ
  #----------
  def pack2()
    array = Array.new
    
    # パラメータの処理
    SAVELEN_TRAINER_NAME.times{|i|
      array << @name[i];
    }
    WBTDL_TRAINER_TITLE_LEN.times{|i|
      array << @title[i];
    }
    WBTDL_MSG_LEN.times{|i|
      array << @msg_before[i];
    }
    WBTDL_MSG_LEN.times{|i|
      array << @msg_win[i];
    }
    WBTDL_MSG_LEN.times{|i|
      array << @msg_lose[i];
    }
    WBTDL_MSG_LEN.times{|i|
      array << @msg_announce[i];
    }
    #
    array << @type_id;
    array << @msg_before_id;
    array << @msg_after_win_id;
    array << @msg_after_lose_id;
    array << @msg_win_id;
    array << @msg_lose_id;
    array << @msg_announce_id;
    array << @lang_id;
    array << @ai_bit;
    array << @pad;	# pad
    #
    # debug
    if(@local_debug == 1)
      p sprintf("character (%d) test (size : 0x%x)",@char_id,(28+44+146+146+146+146+8+4))
    end
    
    # 出力形式をヘッダとあわせること
#    return array.pack("S14S22S73S73S73S73C8SC2S2")  # pad注意
    return array.pack("S14S22S73S73S73S73C8S2")  # サイズ注意
  end
end

#--------------------------------------------------------------
#--------------------------------------------------------------
#------------------
# キャラ(まとめ)
#------------------
class WBT_CHARACTER
  #--------
  # 初期化
  #--------
  def initialize( conv )
    @conv = conv;
    #
    @character = [];
    WBTDL_CHAR_MAX.times{|i|
      @character[i] = WBT_CHARACTER_SINGLE.new( conv, i )
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
    #
    WBTDL_CHAR_MAX.times{|i|
      array << @character[i].pack
    }
    return array
  end
end

