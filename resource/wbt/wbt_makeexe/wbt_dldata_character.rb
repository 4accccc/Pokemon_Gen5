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
TR_MATCH_START_COL  = 3
TR_MATCH_START_ROW  = 3
#
WBTDL_CHAR_MAX		= 4	# キャラ
SAVELEN_TRAINER_NAME	= 16	# 名前(10で制限)
WBTDL_TRAINER_TITLE_LEN	= 22	# タイトル(20で制限)
WBTDL_MSG_LEN		= 74	# メッセージ(36,36で制限)
#
$trainer_str = ["トレーナー名",
                "戦闘前メッセージ",
                "肩書き名(フリーワード)",
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
                "AI設定",		# AIのまとめで使用
                "トレーナータイプ",
                "AI基本",
                "AI威力高い",
                "AIエキスパート",
                "AIダブルバトルAI",
];

# colのテーブル(trainer_strと対にしておく)
$tr_col = [
	0,	# トレーナー名
	15,	# 戦闘前フリーワード
	1,	# 肩書き
	27,	# 戦闘勝フリーワード
	36,	# 戦闘負フリーワード
	45,	# 決勝前口上フリーワード
	14,	# 戦闘前メッセージID
	23,	# 戦闘後、勝ちメッセージID
	24,	# 戦闘後、負けメッセージID
	26,	# 戦闘勝メッセージID
	
	35,	# 戦闘負メッセージID
	44,	# 決勝前口上ID
	7,	# 国コード		# 未使用
	9,	# AI基本		# dummy
	8,	# トレーナータイプ
	9,	# AI基本
	10,	# AI威力高い
	11,	# AIエキスパート
	12,	# AIダブルバトルAI
];

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
    #
    @tr_max = 4;
  end
  
  #--------------------------------
  # データセット(内部で初期化あり)
  #--------------------------------
  def set_data( conv, local_dictionary )
    @conv = conv;
    # 言語の設定
    @local_language = 0;
    chk_local_language();
#    @char_id = num;
    #
    @pokemon.set_data( conv, local_dictionary )
    #
    @name = [];
    len = SAVELEN_TRAINER_NAME*2
    len.times{|i|
      @name[i] = 0;
    }
    @title = [];
    len = WBTDL_TRAINER_TITLE_LEN*2
    len.times{|i|
      @title[i] = 0;
    }
    @msg_before = [];
    len = WBTDL_MSG_LEN*2
    len.times{|i|
      @msg_before[i] = 0;
    }
    @msg_after_win = [];
    len = WBTDL_MSG_LEN*2
    len.times{|i|
      @msg_after_win[i] = 0;
    }
    @msg_after_lose = [];
    len = WBTDL_MSG_LEN*2
    len.times{|i|
      @msg_after_lose[i] = 0;
    }
    @msg_announce = [];
    len = WBTDL_MSG_LEN*2
    len.times{|i|
      @msg_announce[i] = 0;
    }
    #
    @type_id              = 0;
    @msg_before_id        = 0;
    @msg_btl_win_id       = 0;
    @msg_btl_lose_id      = 0;
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
    # 最大数をセット
    if(@char_id < @tr_max)
      set_parameter();
    end
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
     
     
     # 戦闘前メッセージID
     chk_before_mes_id();
     # 戦闘勝ちメッセージID
     chk_btl_win_mes_id();
     # 戦闘負けメッセージID
     chk_btl_lose_mes_id();
     # 決勝前口上メッセージID
     chk_announce_mes_id();
     
     # 戦闘前メッセージ
     chk_msg_before();
     # 戦闘勝ちメッセージ
     chk_msg_win();
     # 戦闘負けメッセージ
     chk_msg_lose();
     # 決勝前メッセージ
     chk_msg_announce();
     
     
     # 戦闘後、勝ちメッセージID
     chk_btl_after_win_mes_id();
     # 戦闘後、負けメッセージID
     chk_btl_after_lose_mes_id();
  end
  
  #----------------------------------
  # トレーナー名
  #----------------------------------
  def chk_trainer_name()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+$tr_col[0], TRAINER_START_ROW+@char_id),
                                      $trainer_str[0])
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[0], TRAINER_START_ROW+@char_id,
                                    $trainer_str[0]);
      return;
    end
    # 変換
    @name = get_unicode( @conv, 4, TRAINER_START_COL+$tr_col[0], TRAINER_START_ROW+@char_id );
    #
    # 入力チェック
    array = []
    array = chk_str_limit_array( @name, 1, 10, $trainer_str[0] );
    if(array[4] == -1)
      # 文字入力エラー
      $wbt_error.print_mes_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[0], TRAINER_START_ROW+@char_id,
                                 $trainer_str[0], array, 1, 10 );
    end
    
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s(%d) :%s", $trainer_str[0],@char_id,
#				@conv.get_cell(4, TRAINER_START_COL+$tr_col[0], TRAINER_START_ROW+@char_id))
#    end
  end
  
  #----------------------------------
  # トレーナータイトル(肩書き?)
  #----------------------------------
  def chk_trainer_title()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+$tr_col[2]+@local_language, TRAINER_START_ROW+@char_id),
                                      $trainer_str[2] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[2]+@local_language,
                                    TRAINER_START_ROW+@char_id, $trainer_str[2]);
      return;
    end
    
    # 変換
    @title = get_unicode( @conv, 4, TRAINER_START_COL+$tr_col[2]+@local_language, TRAINER_START_ROW+@char_id );
    
    # 入力チェック
    array = []
    array = chk_str_limit_array( @title, 1, 20, $trainer_str[2] );
    if(array[4] == -1)
      # 文字入力エラー
      $wbt_error.print_mes_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[2]+@local_language,
                                  TRAINER_START_ROW+@char_id, $trainer_str[2], array, 1, 20 );
    end
    #
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s(%d) :%s", $trainer_str[2],@char_id, 
#			@conv.get_cell(4, TRAINER_START_COL+$tr_col[1], TRAINER_START_ROW+@char_id))
#    end
  end
  
  #----------------------------------
  # 戦闘前メッセージ
  #----------------------------------
  def chk_msg_before()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+$tr_col[1]+@local_language, TRAINER_START_ROW+@char_id),
                                      $trainer_str[1])
    if(ret == -1)
#      p @msg_before_id
      # メッセージIDが0以外の場合は空白でもよい
      if(@msg_before_id == 0)
        # 空白エラー
        $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[1]+@local_language,
                                      TRAINER_START_ROW+@char_id, $trainer_str[1]);
      end
      return;
    end
    
    # 変換
    @msg_before = get_unicode( @conv, 4, TRAINER_START_COL+$tr_col[1]+@local_language, TRAINER_START_ROW+@char_id );
    
    # 入力チェック
    flg = 0
    if((@local_language == 0)||(@local_language == 6))
      flg = 1
    end
    array = []
    array = chk_str_limit_array( @msg_before, 2, $str_limit[2+flg], $trainer_str[1] );
    if(array[4] == -1)
      # 入力エラー
      $wbt_error.print_mes_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[1]+@local_language,
				 TRAINER_START_ROW+@char_id,
                                 $trainer_str[1], array, 2, $str_limit[2+flg] );
    end
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s(%d) :%s", $trainer_str[1],@char_id, 
#                              @conv.get_cell(4, TRAINER_START_COL+$tr_col[1]+@local_language, TRAINER_START_ROW+@char_id))
#    end
  end
  
  #----------------------------------
  # 戦闘勝ちメッセージ
  #----------------------------------
  def chk_msg_win()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+$tr_col[3]+@local_language, TRAINER_START_ROW+@char_id),
                                      $trainer_str[3] )
    if(ret == -1)
#      p @msg_btl_win_id
      # メッセージIDが0以外の場合は空白でもよい
      if(@msg_btl_win_id == 0)
        # 空白エラー
        $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[3]+@local_language, 
							  TRAINER_START_ROW+@char_id, $trainer_str[3]);
      end
      return;
    end
    
    # 変換
    @msg_after_win = get_unicode( @conv, 4, TRAINER_START_COL+$tr_col[3]+@local_language, TRAINER_START_ROW+@char_id );
    
    # 入力チェック
    flg = 0
    if((@local_language == 0)||(@local_language == 6))
      flg = 1
    end
    array = []
    array = chk_str_limit_array( @msg_after_win, 2, $str_limit[2+flg], $trainer_str[3] );
    if(array[4] == -1)
      # 文字入力エラー
      $wbt_error.print_mes_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[3]+@local_language,
							TRAINER_START_ROW+@char_id,
                                 $trainer_str[3], array, 2, $str_limit[2+flg] );
    end
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s(%d) :%s", $trainer_str[3],@char_id,
#                              @conv.get_cell(4, TRAINER_START_COL+$tr_col[3]+@local_language, TRAINER_START_ROW+@char_id))
#    end
  end
  
  #----------------------------------
  # 戦闘負けメッセージ
  #----------------------------------
  def chk_msg_lose()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+$tr_col[4]+@local_language, TRAINER_START_ROW+@char_id),
                                      $trainer_str[4] )
    if(ret == -1)
#      p @msg_btl_lose_id
      # メッセージIDが0以外の場合は空白でもよい
      if(@msg_btl_lose_id == 0)
        # 空白エラー
        $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[4]+@local_language, 
							  TRAINER_START_ROW+@char_id, $trainer_str[4]);
      end
      return;
    end
    
    # 変換
    @msg_after_lose = get_unicode( @conv, 4, TRAINER_START_COL+$tr_col[4]+@local_language, TRAINER_START_ROW+@char_id );
    
    # 入力チェック
    flg = 0
    if((@local_language == 0)||(@local_language == 6))
      flg = 1
    end
    array = []
    array = chk_str_limit_array( @msg_after_lose, 2, $str_limit[2+flg], $trainer_str[4] );
    if(array[4] == -1)
      # 文字入力エラー
      $wbt_error.print_mes_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[4]+@local_language,
							TRAINER_START_ROW+@char_id,
                                 $trainer_str[4], array, 2, $str_limit[2+flg] );
    end
    #
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s(%d) :%s", $trainer_str[4],@char_id,
#                              @conv.get_cell(4, TRAINER_START_COL+$tr_col[4]+@local_language, TRAINER_START_ROW+@char_id))
#    end
  end
  
  #----------------------------------
  # 決勝前メッセージ
  #----------------------------------
  def chk_msg_announce()
    #
    ret = chkblank( @conv.get_cell(4, TRAINER_START_COL+$tr_col[5]+@local_language, TRAINER_START_ROW+@char_id),
                                      $trainer_str[5] )
    if(ret == -1)
#      p @msg_announce_id
      # メッセージIDが0以外の場合は空白でもよい
      if(@msg_announce_id == 0)
        # 空白エラー
        $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[5]+@local_language, 
							  TRAINER_START_ROW+@char_id, $trainer_str[5]);
      end
      return;
    end
    
    # 変換
    @msg_announce = get_unicode( @conv, 4, TRAINER_START_COL+$tr_col[5]+@local_language, TRAINER_START_ROW+@char_id );
    
    # 入力チェック
    flg = 0
    if((@local_language == 0)||(@local_language == 6))
      flg = 1
    end
    array = []
    array = chk_str_limit_array( @msg_announce, 2, $str_limit[2+flg], $trainer_str[5] );
    if(array[4] == -1)
      # 文字入力エラー
      $wbt_error.print_mes_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[5]+@local_language,
							TRAINER_START_ROW+@char_id,
                                 $trainer_str[5], array, 2, $str_limit[2+flg] );
    end
    
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s(%d) :%s", $trainer_str[5],@char_id, 
#                              @conv.get_cell(4, TRAINER_START_COL+$tr_col[5]+@local_language, TRAINER_START_ROW+@char_id))
#    end
  end
  
  #--------------------
  # 戦闘前メッセージID
  #--------------------
  def chk_before_mes_id()
    #
    cstr = @conv.get_cell(4, TRAINER_START_COL+$tr_col[6], TRAINER_START_ROW+@char_id )
    ret = chkblank( cstr, $trainer_str[6] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[6], 
							  TRAINER_START_ROW+@char_id, $trainer_str[6]);
      return;
    end
    # セット
    @msg_before_id = cstr.to_i;
    #
    # 範囲チェック(0 - 255)(u8)
    flg = chk_num_range( @msg_before_id, 0, 255 )
    if(flg == -1)
      # 範囲エラー
      $wbt_error.print_range_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[6], TRAINER_START_ROW+@char_id,
                                    $trainer_str[6], @msg_before_id, 0, 255 );
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[6],@msg_before_id)
    end
  end
  
  #--------------------------
  # 戦闘勝ちメッセージID(新)
  #--------------------------
  def chk_btl_win_mes_id()
    #
    cstr = @conv.get_cell(4, TRAINER_START_COL+$tr_col[9], TRAINER_START_ROW+@char_id )
    ret = chkblank( cstr, $trainer_str[9] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[9], 
							  TRAINER_START_ROW+@char_id, $trainer_str[9]);
      return;
    end
    # セット
    @msg_btl_win_id = cstr.to_i;
    #
    # 範囲チェック(0 - 255)(u8)
    flg = chk_num_range( @msg_btl_win_id, 0, 255 )
    if(flg == -1)
      # 範囲エラー
      $wbt_error.print_range_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[9], TRAINER_START_ROW+@char_id,
                                    $trainer_str[9], @msg_btl_win_id, 0, 255 );
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[9],@msg_btl_win_id)
    end
  end
  
  #------------------------------
  # 戦闘負けメッセージID(新)
  #------------------------------
  def chk_btl_lose_mes_id()
    #
    cstr = @conv.get_cell(4, TRAINER_START_COL+$tr_col[10], TRAINER_START_ROW+@char_id )
    ret = chkblank( cstr, $trainer_str[10] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[10], 
							  TRAINER_START_ROW+@char_id, $trainer_str[10]);
      return;
    end
    # セット
    @msg_btl_lose_id = cstr.to_i;
    #
    # 範囲チェック(0 - 255)(u8)
    flg = chk_num_range( @msg_btl_lose_id, 0, 255 )
    if(flg == -1)
      # 範囲エラー
      $wbt_error.print_range_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[10], TRAINER_START_ROW+@char_id,
                                    $trainer_str[10], @msg_btl_lose_id, 0, 255 );
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[10],@msg_btl_lose_id)
    end
  end
  
  #-------------------------------
  # 戦闘後、勝ちメッセージID(新)
  #-------------------------------
  def chk_btl_after_win_mes_id()
    #
    cstr = @conv.get_cell(4, TRAINER_START_COL+$tr_col[7], TRAINER_START_ROW+@char_id )
    ret = chkblank( cstr, $trainer_str[7] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[7],
							  TRAINER_START_ROW+@char_id, $trainer_str[7]);
      return;
    end
    # セット
    @msg_win_id = cstr.to_i;
    #
    # 範囲チェック(1 - 255)(u8)
    flg = chk_num_range( @msg_win_id, 1, 255 )
    if(flg == -1)
      # 範囲エラー
      $wbt_error.print_range_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[7], TRAINER_START_ROW+@char_id,
                                    $trainer_str[7], @msg_win_id, 1, 255 );
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[7],@msg_win_id)
    end
  end
  
  #------------------------------
  # 戦闘後、負けメッセージID(新)
  #------------------------------
  def chk_btl_after_lose_mes_id()
    #
    cstr = @conv.get_cell(4, TRAINER_START_COL+$tr_col[8], TRAINER_START_ROW+@char_id )
    ret = chkblank( cstr, $trainer_str[8] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[8],
							  TRAINER_START_ROW+@char_id, $trainer_str[8]);
      return;
    end
    # セット
    @msg_lose_id = cstr.to_i;
    #
    # 範囲チェック(1 - 255)(u8)
    flg = chk_num_range( @msg_lose_id, 1, 255 )
    if(flg == -1)
      # 範囲エラー
      $wbt_error.print_range_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[8], TRAINER_START_ROW+@char_id,
                                    $trainer_str[8], @msg_lose_id, 1, 255 );
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[8],@msg_lose_id)
    end
  end
  
  #--------------------------
  # 決勝前口上メッセージID
  #--------------------------
  def chk_announce_mes_id()
    #
    cstr = @conv.get_cell(4, TRAINER_START_COL+$tr_col[11], TRAINER_START_ROW+@char_id )
    ret = chkblank( cstr, $trainer_str[11] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[11],
							  TRAINER_START_ROW+@char_id, $trainer_str[11]);
      return;
    end
    # セット
    @msg_announce_id = cstr.to_i;
    #
    # 範囲チェック(0 - 255)(u8)
    flg = chk_num_range( @msg_announce_id, 0, 255 )
    if(flg == -1)
      # 範囲エラー
      $wbt_error.print_range_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[11], TRAINER_START_ROW+@char_id,
                                    $trainer_str[11], @msg_announce_id, 0, 255 );
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[11],@msg_announce_id)
    end
  end
  
  #--------------------
  # 国コード
  #--------------------
  def chk_lang()
    # 仕様変更により未使用だが、互換のためセット
    @lang_id = $current_lang_num
=begin
    #
    cstr = @conv.get_cell(4, TRAINER_START_COL+$tr_col[12], TRAINER_START_ROW+@char_id )
    ret = chkblank( cstr, $trainer_str[12] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[12],
							  TRAINER_START_ROW+@char_id, $trainer_str[12]);
      return;
    end
    # セット
    str = cstr
    id = hash2no( $lang_hash, str );
    if(id == -1)
      # 入力エラー
      $wbt_error.print_hash_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[12], TRAINER_START_ROW+@char_id,
                                   $trainer_str[12], str );
    else
      # セット
      @lang_id = id;
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[12],@lang_id)
    end
=end
  end
  
  #--------------------
  # AI
  #--------------------
  def chk_ai()
    #
    4.times{|i|
      cstr = @conv.get_cell(4, TRAINER_START_COL+$tr_col[15+i], TRAINER_START_ROW+@char_id )
      ret = chkblank( cstr, $trainer_str[15+i] )
      if(ret == -1)
        # 空白エラー
        $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[15+i],
							    TRAINER_START_ROW+@char_id, $trainer_str[15+i]);
        return;
      end
      # セット
      str = cstr
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
    cstr = @conv.get_cell(4, TRAINER_START_COL+$tr_col[14], TRAINER_START_ROW+@char_id )
    ret = chkblank( cstr, $trainer_str[14] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[14],
							  TRAINER_START_ROW+@char_id, $trainer_str[14]);
      return;
    end
    
    # セット
    num = hash2no( $trainer_hash, cstr );
    if(num == -1)
      # 入力エラー
      $wbt_error.print_hash_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[14], TRAINER_START_ROW+@char_id,
                                   $trainer_str[14], cstr );
    else
      @type_id = num;
    end
    
=begin
    # 番号を取得
    num = cstr.to_i
    #
    # 番号をそのままセット
    # 変換はプログラム側で対応(仕様変更)
    @type_id = num;
    #
    # 範囲制限(0-29)
    flg = chk_num_range( @type_id, 0, 29 )
    if(flg == -1)
      # 範囲エラー
      $wbt_error.print_range_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[14], TRAINER_START_ROW+@char_id,
                                    $trainer_str[14], @type_id, 0, 29 );
    end
=end
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
    array << str2pack( @msg_after_win, WBTDL_MSG_LEN );
    # 戦闘負けメッセージ
    array << str2pack( @msg_after_lose, WBTDL_MSG_LEN );
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
    array << @msg_btl_win_id;
    array << @msg_btl_lose_id;
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
    # 国コード設定(全体のを使用)
    @local_language = $current_lang_id
=begin
    #
    cstr = @conv.get_cell(4, TRAINER_START_COL+$tr_col[12], TRAINER_START_ROW+@char_id )
    ret = chkblank( cstr, $trainer_str[12] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[12],
							  TRAINER_START_ROW+@char_id, $trainer_str[12]);
      return;
    end
    # セット
    str = cstr
    id = hash2no( $lang_tbl_hash, str );
    if(id == -1)
      # 入力エラー
      $wbt_error.print_hash_error( $current_xls_name, 4, TRAINER_START_COL+$tr_col[12], TRAINER_START_ROW+@char_id,
                                   $trainer_str[12], str );
    else
      @local_language = id;
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $trainer_str[12],@local_language)
    end
=end
  end
  
  #----------------------------
  # データセットの個数をセット
  #----------------------------
  def set_tr_max( num )
    @tr_max = num
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
  def set_data( conv, local_dictionary )
    @conv = conv;
    #
    #------------------------
    # データ設定数をセット
    #------------------------
    # 設定数
    dnum = chk_dataset_num( conv )
    # ここでのトレーナー数
    tnum = chk_tr_num( conv )
    #
    if(dnum > 4)
      # データ設定エラー
      $wbt_error.print_dataset_error( $current_xls_name, 3, dnum )
    else
      if(dnum > tnum)
        $wbt_error.print_tr_error( $current_xls_name, 4, tnum, dnum );
      end
    end
    #
    WBTDL_CHAR_MAX.times{|i|
      @character[i].set_tr_max( tnum )
      @character[i].set_data( conv, local_dictionary )
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
  
  #----------------------------------------
  # トレーナー管理のデータ設定数のチェック
  #----------------------------------------
  def chk_dataset_num( conv )
    tnum = 0
    7.times{|i|
      cstr = conv.get_cell(3, TR_MATCH_START_COL, TR_MATCH_START_ROW+i )
      ret = chkblank_no_print( cstr )
      if(ret != -1)
        # データ設定を見つける
        num = hash2no( $match_hash, cstr );
        if(num == 2)
          tnum = tnum+1
        end
      end
    }
    # ここまでで、データ設定数が求まる
    return( tnum )
#    p @dataset_num
  end
  
  #----------------------------------------
  # トレーナー数を求める
  #----------------------------------------
  def chk_tr_num( conv )
    tnum = 0
    4.times{|i|
      cstr = conv.get_cell(4, TRAINER_START_COL+$tr_col[0], TRAINER_START_ROW+i)
      ret = chkblank_no_print( cstr )
      if(ret != -1)
        tnum = tnum+1
      else
        # 空白があったら即終了
        break;
      end
    }
    # ここまででトレーナー数が求まる
    return tnum
  end
  
end

