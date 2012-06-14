#!ruby -Ks
#---------------------------
# wbt_dldata_pokemon.rb
#---------------------------

#--------------
# 定義いろいろ
#--------------
POKE_START_COL		= 5
POKE_START_ROW		= 2
#
P_TRAINER_START_COL    = 3
P_TRAINER_START_ROW    = 3
#
#REG_UP_START_COL	= 4
#REG_UP_START_ROW	= 3
#
TEMOTI_POKEMAX		= 6     	# 手持ちポケモン
NICKNAME_LEN		= 10+1		# ニックネーム
PTL_WAZA_MAX		= 4		# 技
#
EFFORT_MAX              = 510           # 努力値最大
#
$poke_str = ["ポケモン名",
             "フォルムNo",
             "レベル",
             "性別",
             "性格",
             "特性",
             "所持アイテム",
             "ボールID",
             "パワー乱数",
             "努力値",
             
             "ワザ",
             "なつきど",
             "レア",
             "ニックネーム",
             "PPUP",
             "パワー乱数(HP)",
             "パワー乱数(ATK)",
             "パワー乱数(DEF)",
             "パワー乱数(AGI)",
             "パワー乱数(SATK)",
             
             "パワー乱数(SDEF)",
             "努力値(HP)",
             "努力値(ATK)",
             "努力値(DEF)",
             "努力値(AGI)",
             "努力値(SATK)",
             "努力値(SDEF)",
             "ワザ1",
             "ワザ2",
             "ワザ3",
              
             "ワザ4",
             "ワザ1:PPUP",
             "ワザ2:PPUP",
             "ワザ3:PPUP",
             "ワザ4:PPUP",
]

#------------------------------------------------
#------------------------------------------------
class WBT_POKEMON_SINGLE
  #--------
  # 初期化
  #--------
  def initialize( num, trnum )
    @poke_id     = num;
    # 対応するトレーナーの番号
    @trainer_id	 = trnum;
    #
    @poke_max = 6
    @tr_max   = 4
  end
  
  #--------------------------------
  # データセット(内部で初期化あり)
  #--------------------------------
  def set_data( conv, local_dictionary )
    @conv        = conv;
    @local_dictionary     = local_dictionary 
#    @poke_id     = num;
    #
    @monsno      = 0;	# ポケモン番号
    #
    @nick_name = [];
    len = NICKNAME_LEN*2
    len.times{|i|
      @nick_name[i] = 0;
    }
    @expbuf    = [];	# 努力値
    6.times{|i|
      @expbuf[i] = 0;
    }
    #
    @item_no     = 0;	# アイテム番号
    @rate_flag   = 0;   # (レアフラグ)pad注意
    @tokusei     = 0;	# 特性
    
    @waza_id = [];	# 技
    PTL_WAZA_MAX.times{|i|
      @waza_id[i] = 0;
    }
    @pp_count    = 0;	# bit注意
    @seikaku     = 0;	# 性格
    @power_rnd   = 0;   # (性別、パワー乱数)bit注意
    @form_no     = 0;	# フォルム番号
    @ball_id     = 0;	# ボールid
    @level       = 0;	# レベル
    @friend_rate = 0;	# なつき度
    #
    @csv_pokename = ""    # csvでのポケモン名
    @csv_pokename_id = 0  # csvでのポケモン名
    @csv_form    = 0      # csvでのフォルム番号
    #
    @err_flg     = 0    # エラーフラグ(特性で使用)
    #
    @local_debug = 0;
    # debug
    if($debug_code == 1)
      @local_debug = 1;
    end
    #
    # ポケモン最大数チェック
    if(@poke_id < @poke_max)
      # トレーナー最大数チェック
      if(@trainer_id < @tr_max)
        set_parameter();
      end
    end
  end
  
  #----------------
  # パラメータセット
  #----------------
  def set_parameter()
    # ポケモン名
    chk_pokename();
    # フォルムNo
    chk_form();
    # レベル
    chk_level();
    # 性別
    chk_sex();
    # 性別
    chk_seikaku();
    # ニックネーム
    chk_nickname();
    # 特性
    chk_tokusei();
    # アイテム
    chk_item();
    # ボール
    chk_ball();
    # パワー乱数
    chk_power_rnd();
    # 努力値
    chk_exp();
    # 技
    chk_skill();
    # PPUP
    chk_ppup();
    # なつき度
    chk_friend();
    # レア
    chk_rare();
    
  end
  
  #----------------
  # ポケモン名
  #----------------
  def chk_pokename()
    #
    offset = 2*@poke_id+17*@trainer_id
    cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW )
    ret = chkblank( cstr, $poke_str[0] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW,
                                    $poke_str[0]);
      # エラーになった
      @err_flg |= 1;
      return;
    end
    # セット
    str = cstr
    id = hash2no( $monsno_hash, str );
    if(id == -1)
      # 該当するポケモンがない=>エラー
      # 入力エラー
      $wbt_error.print_hash_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW,
                                   $poke_str[0], str );
      # エラーになった
      @err_flg |= 1;
    else
      @monsno = id;
    end
    #
    # csv用(特性の判定で使用)
    @csv_pokename_id = id;
    @csv_pokename    = str;
    #
    if(@local_debug == 1)
      p sprintf("%s :%d: %s", $poke_str[0], @monsno, str)
    end
  end
  
  #----------------
  # フォルムNO
  #----------------
  def chk_form()
    #
    offset = 2*@poke_id+17*@trainer_id
    cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+1 )
    ret = chkblank( cstr, $poke_str[1] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+1,
                                    $poke_str[1]);
      # エラーになった
      @err_flg |= 2;
      return;
    end
    # セット
    @form_no = cstr.to_i
    #
    # 範囲チェック(0-255)(u8)
    flg = chk_num_range( @form_no, 0, 255 )
    if(flg == -1)
      # 範囲エラー
      $wbt_error.print_range_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+1,
                                    $poke_str[1], @form_no, 0, 255 );
      # エラーになった
      @err_flg |= 2;
    end
    #
    # csv用(特性の判定で使用)
    @csv_form = @form_no;
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $poke_str[1], @form_no )
    end
  end
  
  #----------------
  # レベル
  #----------------
  def chk_level()
    #
    offset = 2*@poke_id+17*@trainer_id
    cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+2 )
    ret = chkblank( cstr, $poke_str[2] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+2,
                                    $poke_str[2]);
      return;
    end
    # セット
    @level = cstr.to_i
    #
    # 範囲チェック(0-100)(u8)
    flg = chk_num_range( @level, 0, 100 )
    if(flg == -1)
      # 範囲エラー
      $wbt_error.print_range_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+2,
                                    $poke_str[2], @level, 0, 100 );
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $poke_str[2], @level )
    end
  end
  
  #----------------
  # 性別
  #----------------
  def chk_sex()
    #
    offset = 2*@poke_id+17*@trainer_id
    cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+3 )
    ret = chkblank( cstr, $poke_str[3] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+3,
                                    $poke_str[3]);
      return;
    end
    # セット
    str = cstr
    id = hash2no( $sex_hash, str );
    if(id == -1)
      # 入力エラー
      $wbt_error.print_hash_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+3,
                                   $poke_str[3], str );
    else
      # セット
      @power_rnd |= id<<30;
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :0x%x: %s", $poke_str[3], (@power_rnd>>30)&0x03, str)
    end
  end

  #----------------
  # ニックネーム
  #----------------
  def chk_nickname()
    #
    offset = 2*@poke_id+17*@trainer_id
    ret = chkblank_no_print( @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+4 ) )
    if(ret == -1)
      # 空白の場合はポケモン名をセットする
      #@nick_name = get_unicode( @conv, 5, POKE_START_COL+offset, POKE_START_ROW );
      str = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW )
      id = hash2no( $monsno_hash, str );
      @nick_name = @local_dictionary[id]
    else
      # セット
      @nick_name = get_unicode( @conv, 5, POKE_START_COL+offset, POKE_START_ROW+4 );
    end
    #
    # 入力チェック
    array = []
    array = chk_str_limit_array( @nick_name, 1, 10, $poke_str[13] );
    if(array[4] == -1)
      # 文字入力エラー
      $wbt_error.print_mes_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+4,
                                 $poke_str[13], array, 1, 10 );
    end
    #
#    if(@local_debug == 1)
#      p sprintf("%s :%s", $poke_str[13], @nick_name )
#    end
  end
  
  #----------------
  # 性格
  #----------------
  def chk_seikaku()
    #
    offset = 2*@poke_id+17*@trainer_id
    cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+5 )
    ret = chkblank( cstr, $poke_str[4] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+5,
                                    $poke_str[4]);
      return;
    end
    #
    # セット
    str = cstr
    id = hash2no( $seikaku_hash, str );
    if(id == -1)
      # 入力エラー
      $wbt_error.print_hash_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+5,
                                   $poke_str[4], str );
    else
      @seikaku = id;
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d: %s", $poke_str[4], @seikaku, str)
    end
  end
  
  #----------------
  # 特性
  #----------------
  def chk_tokusei()
    #
    offset = 2*@poke_id+17*@trainer_id
    cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+6 )
    ret = chkblank( cstr, $poke_str[5] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+6,
                                    $poke_str[5]);
      # エラーになった
      @err_flg |= 4;
      return;
    end
    # セット
    str = cstr
    id = hash2no( $tokusei_hash, str );
    if(id == -1)
      # 入力エラー
      $wbt_error.print_hash_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+6,
                                   $poke_str[5], str );
      # エラーになった
      @err_flg |= 4;
    else
      @tokusei = id;
    end
    #
    # 特性のチェック(csv)
    # ただし、すでにエラーになっている時は処理しない
    if(@err_flg == 0)
      ret = chk_csv_tokusei( @csv_pokename_id, @csv_form, id );
      if(ret == -1)
        # 特性エラー
        $wbt_error.print_tokusei_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+6,
                                      @csv_pokename, @csv_form, str );
      end
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d: %s", $poke_str[5], @tokusei, str)
    end
  end
  
  #------------------------
  # csvでの特性チェック
  #
  # monsno番号
  # form番号
  # tokusei番号
  #------------------------
  def chk_csv_tokusei( pokename, form, tokusei )
    size = $wbt_tokusei_tbl.size/6;
#    p sprintf("%d",size);
    size.times{|i|
      mbuffer = $wbt_tokusei_tbl[6*i]
#      p sprintf("%s, %s, %d, %s",mstr, pokename,form,tokusei);
      if(mbuffer == 0)
        break;
      end
      #
      # 該当のポケモンがいる=>特性を探す
      if(mbuffer == pokename)
        # フォルム番号をチェック
        if($wbt_tokusei_tbl[6*i+1] == form)
          3.times{|j|
            if($wbt_tokusei_tbl[6*i+2+j] == tokusei)
              return( 0 );
            end
          }
        end
      end
    }
    #
    return( -1 );
  end
  
  #----------------
  # アイテム
  #----------------
  def chk_item()
    #
    offset = 2*@poke_id+17*@trainer_id
    cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+7 )
    ret = chkblank( cstr, $poke_str[6] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+7, 
                                    $poke_str[6]);
      return;
    end
    # セット
    str = cstr
    id = hash2no( $item_hash, str );
    if(id == -1)
      # 入力エラー
      $wbt_error.print_hash_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+7,
                                   $poke_str[6], str );
    else
      @item_no = id;
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d: %s", $poke_str[6], @item_no, str)
    end
  end
  
  #----------------
  # ボール
  #----------------
  def chk_ball()
    #
    offset = 2*@poke_id+17*@trainer_id
    cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+8 )
    ret = chkblank( cstr, $poke_str[7] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+8, 
                                    $poke_str[7]);
      return;
    end
    # セット
    str = cstr
    id = hash2no( $ball_hash, str );
    if(id == -1)
      # 入力エラー
      $wbt_error.print_hash_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+8,
                                   $poke_str[7], str );
    else
      @ball_id = id;
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d: %s", $poke_str[7], @ball_id, str)
    end
  end
  
  #----------------
  # パワー乱数
  #----------------
  def chk_power_rnd()
    #
    offset = 2*@poke_id+17*@trainer_id
    6.times{|i|
      cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+10+i )
      ret = chkblank( cstr, $poke_str[15+i] )
      if(ret == -1)
        # 空白エラー
        $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+10+i,
                                      $poke_str[15+i]);
        return;
      end
      #
      # セット
      id = cstr.to_i
      #
      # 範囲チェック(0 - 31)
      flg = chk_num_range( id, 0, 31 )
      if(flg == -1)
        # 範囲エラー
        $wbt_error.print_range_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+10+i,
                                    $poke_str[15+i], id, 0, 31 );
        # 念のため
        id = 0
      end
      #
      @power_rnd |= (id<<(i*5))
    }
    #
    if(@local_debug == 1)
      p sprintf("%s :(%d,%d,%d,%d,%d,%d) : 0x%x",
                 $poke_str[8], 
                 (@power_rnd&0x1f),((@power_rnd>>5)&0x1f),((@power_rnd>>10)&0x1f),
                 ((@power_rnd>>15)&0x1f),((@power_rnd>>20)&0x1f),((@power_rnd>>25)&0x1f),
                 @power_rnd )
    end
  end
  
  #----------------
  # 努力値
  #----------------
  def chk_exp()
    #
    offset = 2*@poke_id+17*@trainer_id
    6.times{|i|
      cstr = @conv.get_cell(5, POKE_START_COL+1+offset, POKE_START_ROW+10+i )
      ret = chkblank( cstr, $poke_str[21+i] )
      if(ret == -1)
        # 空白エラー
        $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+1+offset, POKE_START_ROW+10+i,
                                      $poke_str[21+i]);
        return;
      end
      #
      # セット
      @expbuf[i] = cstr.to_i
      #
      # 範囲チェック(0 - 255)(u8)
      flg = chk_num_range( @expbuf[i], 0, 255 )
      if(flg == -1)
        # 範囲エラー
        $wbt_error.print_range_error( $current_xls_name, 5, POKE_START_COL+1+offset, POKE_START_ROW+10+i,
                                    $poke_str[21+i], @expbuf[i], 0, 255 );
      end
    }
    #
    # 努力値合計チェック
    total = 0
    6.times{|i|
      total = total + @expbuf[i]
    }
    if(total > EFFORT_MAX)
        # 範囲エラー
        $wbt_error.print_effort_error( $current_xls_name, 5, POKE_START_COL+1+offset, POKE_START_ROW+10, total );
    end
    
    #
    if(@local_debug == 1)
      p sprintf("%s :(%d,%d,%d,%d,%d,%d)",
                 $poke_str[9],
                 @expbuf[0],@expbuf[1],@expbuf[2],
                 @expbuf[3],@expbuf[4],@expbuf[5] );
    end
  end
  
  #----------------
  # 技
  #----------------
  def chk_skill()
    #
    offset = 2*@poke_id+17*@trainer_id
    #
    flg = 0
    PTL_WAZA_MAX.times{|i|
      cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+16+i )
      ret = chkblank_no_print( cstr );
      # 空白の場合は「なし」で処理
      if(ret == -1)
#        break;
#        poke_exit();
        flg = 1;
        if(i == 0)
          # 空白エラー(1つ目は必ずいれること)
          $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+16+i,
                                      $poke_str[27+i]);
          break;
        else
          next;
        end
      else
        # すでにエラーで入力がある(間隔のある設定)
        if(flg == 1)
          # 入力エラー
          $wbt_error.print_skill_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+16+i,
                                   $poke_str[27+i], cstr, 0 );
          next;
        end
      end
      #
      # セット
      str = cstr
      id = hash2no( $wazano_hash, str );
      if(id == -1)
        # 入力エラー
        $wbt_error.print_hash_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+16+i,
                                   $poke_str[27+i], str );
      else
        i.times{|j|
          if(@waza_id[j] == id)
            # 重複エラー
            $wbt_error.print_skill_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+16+i,
                                   $poke_str[27+i], cstr, 1);
            break;
          end
        }
        
        @waza_id[i] = id;
      end
    }
    if(@local_debug == 1)
      p sprintf("%s :(%d,%d,%d,%d)", $poke_str[10], @waza_id[0],@waza_id[1],@waza_id[2],@waza_id[3] )
    end
  end
  
  #----------------
  # PPUP
  #----------------
  def chk_ppup()
    #
    offset = 2*@poke_id+17*@trainer_id
    #
    PTL_WAZA_MAX.times{|i|
      cstr = @conv.get_cell(5, POKE_START_COL+1+offset, POKE_START_ROW+16+i )
      ret = chkblank( cstr, $poke_str[31+i] )
      # 念のため
      if(ret == -1)
        # 空白エラー
        $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+1+offset, POKE_START_ROW+16+i,
                                      $poke_str[31+i]);
        return;
      end
      #
      # セット
      str = cstr
      id = hash2no( $ppup_hash, str );
      if(id == -1)
        # 入力エラー
        $wbt_error.print_hash_error( $current_xls_name, 5, POKE_START_COL+1+offset, POKE_START_ROW+16+i,
                                   $poke_str[31+i], str );
      else
        @pp_count |= (id<<(i*2));
      end
      #
    }
    if(@local_debug == 1)
      p sprintf("%s : 0x%x", $poke_str[14], @pp_count );
    end
  end
  
  #----------------
  # なつき度
  #----------------
  def chk_friend()
    #
    offset = 2*@poke_id+17*@trainer_id
    cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+20 )
    ret = chkblank( cstr, $poke_str[11] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+20,
                                    $poke_str[11]);
      return;
    end
    # セット
    @friend_rate = cstr.to_i
    #
    # 範囲チェック(0 - 255)(u8)
    flg = chk_num_range( @friend_rate, 0, 255 )
    if(flg == -1)
      # 範囲エラー
      $wbt_error.print_range_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+20,
                                    $poke_str[11], @friend_rate, 0, 255 );
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $poke_str[11], @friend_rate )
    end
  end
  
  #----------------
  # レア
  #----------------
  def chk_rare()
    #
    offset = 2*@poke_id+17*@trainer_id
    cstr = @conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW+21 )
    ret = chkblank( cstr, $poke_str[12] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 5, POKE_START_COL+offset, POKE_START_ROW+21,
                                    $poke_str[12]);
      return;
    end
    # セット
    str = cstr
    id = hash2no( $onoff_hash, str );
    if(id >= 2)
      @rate_flag = 1;
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $poke_str[12], @rate_flag )
    end
  end
  
  #----------------
  # データのパック
  #----------------
  def pack2()
    array = Array.new
    
    array << @monsno;
    return array.pack("S");
  end
  
  #----------------
  # データのパック
  #----------------
  def pack3()
    array = Array.new
    #
    array << @item_no;
    array << @rate_flag; # bit注意
    array << @tokusei;
    6.times{|i|
      array << @expbuf[i];
    }
    
    PTL_WAZA_MAX.times{|i|
      array << @waza_id[i];
    }
    array << @pp_count;	   # bit注意
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
#    return array.pack("S1S11S1C8S4C2IC4")
    return array.pack("S1C8S4C2IC4")
  end
  
  #----------------
  # データのパック
  #----------------
  def pack()
    array = Array.new
    
    array << pack2();
    array << str2pack( @nick_name, NICKNAME_LEN );
    array << pack3();
    
    return array
  end
  
  #----------------------------------
  # 戻る処理
  #----------------------------------
  def poke_exit()
    @conv.close_file();
    exit(1)
  end
  
  #----------------
  # 最大数セット
  #----------------
  def set_pokemax( num )
    @poke_max = num
  end
  
  #----------------
  # 最大数のチェック
  #----------------
  def chk_pokemax( conv )
    # 空白か調べる
    offset = 2*@poke_id+17*@trainer_id
    cstr = conv.get_cell(5, POKE_START_COL+offset, POKE_START_ROW )
    ret = chkblank( cstr, $poke_str[0] )
    return( ret );
  end
  
  #----------------
  # トレーナー最大数セット
  #----------------
  def set_tr_max( num )
    @tr_max = num
  end

end

#------------------------------------------------
#------------------------------------------------
class WBT_POKEMON
  #--------
  # 初期化
  #--------
  def initialize( num )
    # 対応するトレーナーの番号
    @trainer_id = num;
    #
    @pokemon = [];
    TEMOTI_POKEMAX.times{|i|
      @pokemon[i] = WBT_POKEMON_SINGLE.new( i, num )
    }
  end
  
  #--------------------------------
  # データセット(内部で初期化あり)
  #--------------------------------
  def set_data( conv, local_dictionary )
    @conv = conv;
    #
    #------------------------
    # トレーナー数を調べる
    #------------------------
    tnum = chk_tr_num( conv )
    
    #------------------------
    # ポケモン数の下限を調べる
    #------------------------
    unum = chk_poke_unum( conv )
#    p unum
    
    #--------------------------------------
    # ポケモン数を調べる
    # レギュレーションの下限チェックを行う
    #--------------------------------------
    num = 0
    TEMOTI_POKEMAX.times{|i|
      if( @pokemon[i].chk_pokemax( conv ) != -1 )
        num = num+1
      else
        # 空白を見つけたら即終了
        break
      end
    }
    # トレーナー数以上のデータは、
    # 空白でもかまわない
    if(tnum > @trainer_id)
      # 最小チェック
      if(unum > num)
        $wbt_error.print_poke_error( $current_xls_name, 5, @trainer_id, num, unum )
      end
    end
#    p num
    #
    TEMOTI_POKEMAX.times{|i|
      @pokemon[i].set_pokemax( num )
      @pokemon[i].set_tr_max( tnum )
      @pokemon[i].set_data( conv, local_dictionary )
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
  
  #----------------------------------------
  # トレーナー数を求める
  #----------------------------------------
  def chk_tr_num( conv )
    tnum = 0
    4.times{|i|
      cstr = conv.get_cell(4, P_TRAINER_START_COL, P_TRAINER_START_ROW+i)
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
  
  #----------------------------------------
  # 参加ポケモンの下限を求める
  # (REG_UP_START_COL、REG_UP_START_ROWは、
  # wbt_dldata_rule.rbで定義)
  #----------------------------------------
  def chk_poke_unum( conv )
    num = 0
    cstr = conv.get_cell(1, REG_UP_START_COL, REG_UP_START_ROW)
    ret = chkblank_no_print( cstr )
    if(ret != -1)
      num = cstr.to_i
    end
    # 参加ポケモン数下限
    return num
  end
  
end

