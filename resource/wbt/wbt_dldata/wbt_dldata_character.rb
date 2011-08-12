#!ruby -Ks
#---------------------------
# wbt_dldata_character.rb
#---------------------------
require "wbt_dldata_pokemon.rb"

#--------------
# 定義いろいろ
#--------------
TRAINER_START_COL    = 3
TRAINER_START_ROW    = 3
#
TYPE_START_COL    = 3
TYPE_START_ROW    = 3
#
WBTDL_CHAR_MAX		= 4	# キャラ
SAVELEN_TRAINER_NAME	= 14	# 名前
WBTDL_TRAINER_TITLE_LEN	= 22	# タイトル
WBTDL_MSG_LEN		= 73	# メッセージ
#
$trainer_str = ["トレーナー名",
                "戦闘前メッセージ",
                "肩書き",
                "戦闘勝ちメッセージ",
                "戦闘負けメッセージ",
                "決勝前メッセージ",
                "戦闘前メッセージID",
                "戦闘後、勝ちメッセージID",
                "戦闘後、負けメッセージID",
                "戦闘勝ちメッセージID",
                "戦闘負けメッセージID",
                "決勝前口上ID",
                "国コード",
                "AI設定",
                "トレーナータイプ",
];

# トレーナータイプのhash用のヘッダ
$trtype_header = "TRTYPE_";

#--------------------------------------------------------------
#--------------------------------------------------------------
#------------------
# キャラ(単体)
#------------------
class WBT_CHARACTER_SINGLE
  #-----------------------
  # 初期化(番号をセット)
  #-----------------------
  def initialize( num )
    @char_id = num;
    @pokemon = WBT_POKEMON.new( num )
  end
  
  #--------------------------------
  # データセット(内部で初期化あり)
  #--------------------------------
  def set_data( conv )
    @conv = conv;
    # 言語の設定
    @local_language = 0;
    chk_local_language();
#    @char_id = num;
    #
    @pokemon.set_data( conv )
    #
    @name = [];
    SAVELEN_TRAINER_NAME.times{|i|
      @name[i] = 0;
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
    @type_id              = 0;
    @msg_before_id        = 0;
    @msg_after_win_id     = 0;
    @msg_after_lose_id    = 0;
    @msg_win_id           = 0;
    @msg_lose_id          = 0;
    @msg_announce_id      = 0;
    @lang_id              = 0;
    @ai_bit               = 0;
    @pad                  = 0;
   #
    @local_debug = 0;
    # debug
    if($debug_code == 1)
      @local_debug = 1;
    end
    #
    set_parameter();
  end
  
  #----------------
  # パラメータセット
  #----------------
  def set_parameter()
     #
     # トレーナー名
     chk_trainer_name();
     # 肩書き
     chk_trainer_title();
     # トレーナータイプ
     chk_trtype();
     # AI処理
     chk_ai();
     # 国コード
     chk_lang();
     
     # 戦闘前メッセージ
     chk_msg_before();
     # 戦闘勝ちメッセージ
     chk_msg_win();
     # 戦闘負けメッセージ
     chk_msg_lose();
     # 決勝前メッセージ
     chk_msg_announce();
     # 戦闘前メッセージID
     chk_before_mes_id();
     # 戦闘後、勝ちメッセージID
     chk_after_win_mes_id();
     # 戦闘後、負けメッセージID
     chk_after_lose_mes_id();
     # 戦闘勝ちメッセージID
     chk_win_mes_id();
     # 戦闘負けメッセージID
     chk_lose_mes_id();
     # 決勝前口上メッセージID
     chk_announce_mes_id();
  end
  
  #----------------------------------
  # トレーナー名
  #----------------------------------
  def chk_trainer_name()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL, TRAINER_START_ROW+@char_id),
                          TRAINER_START_COL, $trainer_str[0])
    if(ret == -1)
      trainer_exit();
    end
    
    # 変換
    str = @conv.get_cell(4, TRAINER_START_COL, TRAINER_START_ROW+@char_id )
    @name = NKF.nkf("-w16L0", str)
    
    # セット(コードの変換注意)
    if(@local_debug == 1)
      p sprintf("%s(%d) :%s", $trainer_str[0],@char_id, @conv.get_cell(4, TRAINER_START_COL, TRAINER_START_ROW+@char_id))
    end
  end
  
  #----------------------------------
  # トレーナータイトル(肩書き?)
  #----------------------------------
  def chk_trainer_title()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+1, TRAINER_START_ROW+@char_id),
                          TRAINER_START_COL+1, $trainer_str[2])
    if(ret == -1)
      trainer_exit();
    end
    
    # 変換
    str = @conv.get_cell(4, TRAINER_START_COL+1, TRAINER_START_ROW+@char_id )
    @title = NKF.nkf("-w16L0", str)
    
    # セット(コードの変換注意)
    if(@local_debug == 1)
      p sprintf("%s(%d) :%s", $trainer_str[2],@char_id, @conv.get_cell(4, TRAINER_START_COL+1, TRAINER_START_ROW+@char_id))
    end
  end
  
  #----------------------------------
  # 戦闘前メッセージ
  #----------------------------------
  def chk_msg_before()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+12+@local_language, TRAINER_START_ROW+@char_id),
                                      TRAINER_START_COL+12+@local_language, $trainer_str[1])
    if(ret == -1)
      trainer_exit();
    end
    
    # 変換
    str = @conv.get_cell(4, TRAINER_START_COL+12+@local_language, TRAINER_START_ROW+@char_id )
    @msg_before = NKF.nkf("-w16L0", str)
    
    # セット(コードの変換注意)
    if(@local_debug == 1)
      p sprintf("%s(%d) :%s", $trainer_str[1],@char_id, 
                              @conv.get_cell(4, TRAINER_START_COL+12+@local_language, TRAINER_START_ROW+@char_id))
    end
  end
  
  #----------------------------------
  # 戦闘勝ちメッセージ
  #----------------------------------
  def chk_msg_win()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+24+@local_language, TRAINER_START_ROW+@char_id),
                                      TRAINER_START_COL+24+@local_language, $trainer_str[3])
    if(ret == -1)
      trainer_exit();
    end
    
    # 変換
    str = @conv.get_cell(4, TRAINER_START_COL+24+@local_language, TRAINER_START_ROW+@char_id )
    @msg_win = NKF.nkf("-w16L0", str)
    
    # セット(コードの変換注意)
    if(@local_debug == 1)
      p sprintf("%s(%d) :%s", $trainer_str[3],@char_id,
                              @conv.get_cell(4, TRAINER_START_COL+24+@local_language, TRAINER_START_ROW+@char_id))
    end
  end
  
  #----------------------------------
  # 戦闘負けメッセージ
  #----------------------------------
  def chk_msg_lose()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+34+@local_language, TRAINER_START_ROW+@char_id),
                                      TRAINER_START_COL+34+@local_language, $trainer_str[4])
    if(ret == -1)
      trainer_exit();
    end
    
    # 変換
    str = @conv.get_cell(4, TRAINER_START_COL+34+@local_language, TRAINER_START_ROW+@char_id )
    @msg_win = NKF.nkf("-w16L0", str)
    
    # セット(コードの変換注意)
    if(@local_debug == 1)
      p sprintf("%s(%d) :%s", $trainer_str[4],@char_id,
                              @conv.get_cell(4, TRAINER_START_COL+34+@local_language, TRAINER_START_ROW+@char_id))
    end
  end
  
  #----------------------------------
  # 決勝前メッセージ
  #----------------------------------
  def chk_msg_announce()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+43+@local_language, TRAINER_START_ROW+@char_id),
                                      TRAINER_START_COL+43+@local_language, $trainer_str[5])
    if(ret == -1)
      trainer_exit();
    end
    
    # 変換
    str = @conv.get_cell(4, TRAINER_START_COL+43+@local_language, TRAINER_START_ROW+@char_id )
    @msg_announce = NKF.nkf("-w16L0", str)
    
    # セット(コードの変換注意)
    if(@local_debug == 1)
      p sprintf("%s(%d) :%s", $trainer_str[5],@char_id, 
                              @conv.get_cell(4, TRAINER_START_COL+43+@local_language, TRAINER_START_ROW+@char_id))
    end
  end
  
  #--------------------
  # 戦闘前メッセージID
  #--------------------
  def chk_before_mes_id()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+11, TRAINER_START_ROW+@char_id ),
                                      TRAINER_START_COL+11, $trainer_str[6] )
    if(ret == -1)
      trainer_exit();
    end
    # セット
    @msg_before_id = @conv.get_cell(4, TRAINER_START_COL+11, TRAINER_START_ROW+@char_id).to_i;
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[6],@msg_before_id)
    end
  end
  
  #--------------------
  # 戦闘後、勝ちメッセージID
  #--------------------
  def chk_after_win_mes_id()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+20, TRAINER_START_ROW+@char_id ),
                                      TRAINER_START_COL+20, $trainer_str[7] )
    if(ret == -1)
      trainer_exit();
    end
    # セット
    @msg_after_win_id = @conv.get_cell(4, TRAINER_START_COL+20, TRAINER_START_ROW+@char_id).to_i;
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[7],@msg_after_win_id)
    end
  end
  
  #--------------------
  # 戦闘後、負けメッセージID
  #--------------------
  def chk_after_lose_mes_id()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+21, TRAINER_START_ROW+@char_id ),
                                      TRAINER_START_COL+21, $trainer_str[8] )
    if(ret == -1)
      trainer_exit();
    end
    # セット
    @msg_after_lose_id = @conv.get_cell(4, TRAINER_START_COL+21, TRAINER_START_ROW+@char_id).to_i;
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[8],@msg_after_lose_id)
    end
  end
  
  #--------------------
  # 戦闘勝ちメッセージID
  #--------------------
  def chk_win_mes_id()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+23, TRAINER_START_ROW+@char_id ),
                                      TRAINER_START_COL+23, $trainer_str[9] )
    if(ret == -1)
      trainer_exit();
    end
    # セット
    @msg_win_id = @conv.get_cell(4, TRAINER_START_COL+23, TRAINER_START_ROW+@char_id).to_i;
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[9],@msg_win_id)
    end
  end
  
  #--------------------
  # 戦闘負けメッセージID
  #--------------------
  def chk_lose_mes_id()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+33, TRAINER_START_ROW+@char_id ),
                                      TRAINER_START_COL+33, $trainer_str[10] )
    if(ret == -1)
      trainer_exit();
    end
    # セット
    @msg_lose_id = @conv.get_cell(4, TRAINER_START_COL+33, TRAINER_START_ROW+@char_id).to_i;
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[10],@msg_lose_id)
    end
  end
  
  #--------------------
  # 決勝前口上メッセージID
  #--------------------
  def chk_announce_mes_id()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+42, TRAINER_START_ROW+@char_id ),
                                      TRAINER_START_COL+42, $trainer_str[11] )
    if(ret == -1)
      trainer_exit();
    end
    # セット
    @msg_announce_id = @conv.get_cell(4, TRAINER_START_COL+42, TRAINER_START_ROW+@char_id).to_i;
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[11],@msg_announce_id)
    end
  end
  
  #--------------------
  # 国コード
  #--------------------
  def chk_lang()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+9, TRAINER_START_ROW+@char_id ),
                                      TRAINER_START_COL+9, $trainer_str[12] )
    if(ret == -1)
      trainer_exit();
    end
    # セット
    str = @conv.get_cell(4, TRAINER_START_COL+9, TRAINER_START_ROW+@char_id)
    id = hash2no( $lang_hash, str );
    if(id == -1)
      # 該当する性別がない=>エラー
      print_error( str, 0 );
    else
      @lang_id = id;
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[12],@lang_id)
    end
  end
  
  #--------------------
  # AI
  #--------------------
  def chk_ai()
    #
    4.times{|i|
      ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+3+i, TRAINER_START_ROW+@char_id ),
                                        TRAINER_START_COL+3+i, $trainer_str[13] )
      if(ret == -1)
        trainer_exit();
      end
      # セット
      str = @conv.get_cell(4, TRAINER_START_COL+3+i, TRAINER_START_ROW+@char_id)
      id = hash2no( $onoff_hash, str );
      if(id >= 2)
        @ai_bit |= (1<<i);
      end
    }
    #
    if(@local_debug == 1)
      p sprintf("%s :0x%x", $trainer_str[13],@ai_bit)
    end
  end
  
  #--------------------
  # トレーナータイプ
  #--------------------
  def chk_trtype()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+2, TRAINER_START_ROW+@char_id ),
                                      TRAINER_START_COL+2, $trainer_str[14] )
    if(ret == -1)
      trainer_exit();
    end
    
    # 番号を取得
    num = @conv.get_cell(4, TRAINER_START_COL+2, TRAINER_START_ROW+@char_id).to_i
    #
    if(num < 1)
        p sprintf("%d :error",num)
        num = 1;
    end
    
    # トレーナータイプ指定からトレーナータイプを取得
    str = @conv.get_cell(5, TYPE_START_COL+num-1, TYPE_START_ROW)
    str = $trtype_header + str
    #
    id = hash2no( $trtype_id_hash, str );
    if(id == -1)
      # 該当する性別がない=>エラー
      print_error( str, 0 );
    else
      @type_id = id;
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[14],@type_id)
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
    # トレーナー名
    array << str2pack( @name, SAVELEN_TRAINER_NAME );
    # トレーナー肩書き
    array << str2pack( @title, WBTDL_TRAINER_TITLE_LEN );
    # 戦闘前メッセージ
    array << str2pack( @msg_before, WBTDL_MSG_LEN );
    # 戦闘勝ちメッセージ
    array << str2pack( @msg_win, WBTDL_MSG_LEN );
    # 戦闘負けメッセージ
    array << str2pack( @msg_lose, WBTDL_MSG_LEN );
    # 決勝前メッセージ
    array << str2pack( @msg_announce, WBTDL_MSG_LEN );
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
#    return array.pack("S14S22S73S73S73S73C8S2")  # サイズ注意
    return array.pack("C8S2")  # サイズ注意
  end
  
  #----------------------------------
  # 戻る処理
  #----------------------------------
  def trainer_exit()
    @conv.close_file();
    exit(1)
  end
  
  #--------------------
  # 国の判定
  #--------------------
  def chk_local_language()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+9, TRAINER_START_ROW+@char_id ),
                                      TRAINER_START_COL+9, $trainer_str[12] )
    if(ret == -1)
      trainer_exit();
    end
    # セット
    str = @conv.get_cell(4, TRAINER_START_COL+9, TRAINER_START_ROW+@char_id)
    id = hash2no( $lang_tbl_hash, str );
    if(id == -1)
      # 該当する性別がない=>エラー
      print_error( str, 0 );
    else
      @local_language = id;
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[12],@local_language_id)
    end
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
  def initialize()
    @character = [];
    WBTDL_CHAR_MAX.times{|i|
      @character[i] = WBT_CHARACTER_SINGLE.new( i )
    }
  end
  
  #----------------------------------
  # データのセット(内部で初期化あり)
  #----------------------------------
  def set_data( conv )
    @conv = conv;
    #
    WBTDL_CHAR_MAX.times{|i|
      @character[i].set_data( conv )
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

