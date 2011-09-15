#!ruby -Ks
#---------------------------
# wbt_dldata_rule.rb
#---------------------------

require "wbt_dldata_util.rb"

#--------------
# 定義いろいろ
#--------------
WBTDL_POKE_MAX		= 649		# ポケモン最大
WBTDL_ITEM_MAX		= 564		# アイテム最大(要調整)

WBTDL_POKENUMBER_MAX	= 649/8+1	# ポケモン制限(要調整)(82)
WBTDL_POKEITEM_MAX	= 608/8		# 道具制限(要調整)(76)
SHOOTER_ITEM_MAX 	= 7		# 要調整
#
# パラメータ開始位置
# レギュレーション用
REG_UP_START_ROW	= 3
REG_UP_START_COL	= 4
# レギュレーション管理用
RULE_UP_START_ROW	= 3
RULE_UP_START_COL	= 3

# 要整理
$rule_error_str = ["ポケモン名",
                   "アイテム名",
                   "参加条件",
                   "バトルルール",
                   "禁止技",
                   "ＢＧＭ",
                   "レベル",
                   "同じポケモン",
                   "同じどうぐ",
                   "レベル範囲",
                   "ニックネーム",
                   "タイプ",
];
#--------------------------------------------------------------
#--------------------------------------------------------------
class WBT_RULE
  #--------
  # 初期化
  #--------
  def initialize()
    #
  end
  
  #---------------------------------
  # データのセット(内部で初期化あり)
  #---------------------------------
  def set_data( conv )
    @conv = conv;
    #
    @join_info = [3,3,0];	# 参加下限、参加上限、参加レベル
    @level_range  = 0;		# レベル範囲
    @level_total  = 0;		# レベル合計
    @both_info = [0,0];		# 同じポケモン、同じどうぐ
    #
    @veto_poke_bit = [];	# 禁止ポケモン
    WBTDL_POKENUMBER_MAX.times{|i|
      @veto_poke_bit[i] = 0;
    }
    @veto_item_bit = [];	# 禁止アイテム
    WBTDL_POKEITEM_MAX.times{|i|
      @veto_item_bit[i] = 0;
    }
    #
    @pad = 0;
    @btl_rule           = 0;	# バトルルール
    @bgm_flag           = 0;	# BGMフラグ
    @need_poketype      = 0;	# 必要なポケモンタイプ
    @veto_waza_no       = 0;	# 禁止技番号
    
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
    # 参加条件(下限、上限、レベル)
    chk_join_info();
    # レベル範囲
    chk_level_range();
    # レベル合計
    chk_level_total();
    # 同じポケモン、どうぐ
    chk_same();
    # 参加禁止ポケモン
    chk_pokemon();
    # 禁止アイテム
    chk_item();
    # バトルルール
    chk_battle_rule();
    # 禁止技
    chk_skill();
    # タイプ
    chk_type();
    # BGM
    chk_bgm();
  end
  
  #----------------
  # 参加いろいろ
  #----------------
  def chk_join_info()
    #
    3.times{|i|
      ret = chkblank( @conv.get_cell(1, REG_UP_START_COL+i, REG_UP_START_ROW), 
                                        REG_UP_START_COL+i, $rule_error_str[2])
      if(ret == -1)
         wbt_exit();
      end
      # セット
      @join_info[i] = @conv.get_cell(1, REG_UP_START_COL+i, REG_UP_START_ROW).to_i;
      
      if(@local_debug == 1)
        p sprintf("%s :%d",$rule_error_str[2], @join_info[i])
      end
    }
  end
  
  #----------------
  # レベル範囲
  #----------------
  def chk_level_range()
    #
    ret = chkblank( @conv.get_cell(1, REG_UP_START_COL+3, REG_UP_START_ROW), 
                    REG_UP_START_COL+3, $rule_error_str[9])
    if(ret == -1)
      wbt_exit();
    end
    # セット
    num = hash2no( $level_range_hash, @conv.get_cell(1, REG_UP_START_COL+3, REG_UP_START_ROW) );
    if(num == -1)
      print_error( @conv.get_cell(1, REG_UP_START_COL+3, REG_UP_START_ROW), 0 );
    else
      @level_range = num;
    end
    
    if(@local_debug == 1)
      p sprintf("%d :%s",@level_range, $rule_error_str[9] )
    end
  end
  
  #----------------
  # レベル合計
  #----------------
  def chk_level_total()
    #
    ret = chkblank( @conv.get_cell(1, REG_UP_START_COL+4, REG_UP_START_ROW), 
                    1+REG_UP_START_COL+4, $rule_error_str[6])
    if(ret == -1)
      wbt_exit();
    end
    # セット
    @level_total = @conv.get_cell(1, REG_UP_START_COL+4, REG_UP_START_ROW).to_i;
    
    if(@local_debug == 1)
      p sprintf("%d :%s",@level_total, $rule_error_str[6])
    end
  end
  
  #----------------
  # 同じチェック
  #----------------
  def chk_same()
    #
    2.times{|i|
      # ブランクチェック
      ret = chkblank( @conv.get_cell(1, REG_UP_START_COL+5+i, REG_UP_START_ROW), 
                                     1+REG_UP_START_COL+5+i, $rule_error_str[7+i]);
      if(ret == -1)
        wbt_exit();
      end
      #
      # セット
      num = hash2no( $ng_hash, @conv.get_cell(1, REG_UP_START_COL+5+i, REG_UP_START_ROW) );
      if(num >= 2)
        @both_info[i] = 1;
      else
        @both_info[i] = 0;
      end
      #
      if(@local_debug == 1)
        p sprintf("%d :%s",@both_info[i], $rule_error_str[7+i])
      end
    }
  end
  
  #----------------
  # 参加禁止ポケモン
  # (参照するシートが違うので注意)
  #----------------
  def chk_pokemon()
    #
    WBTDL_POKE_MAX.times{|i|
      # ブランクチェック
      ret = chkblank( @conv.get_cell(2, RULE_UP_START_COL+i, RULE_UP_START_ROW), 1+RULE_UP_START_COL+i, $rule_error_str[0]);
      if(ret == -1)
        wbt_exit();
      end
      #
      # セット
      flg = 0;
      num = hash2no( $ng_hash, @conv.get_cell(2, RULE_UP_START_COL+i, RULE_UP_START_ROW) );
      # NGの場合=>ビットを立てる
      if(num < 2)
        flg = 1;
        # ポケモン名とIDを調べ、そこでの番号を使う
        # =>エクセルでの並びは使用しない
        str = @conv.get_cell(2, RULE_UP_START_COL+i, RULE_UP_START_ROW-1);
        id = hash2no( $monsno_hash, str );
        if(id == -1)
           # 該当するポケモンがない=>エラー
           print_error( str, 0 );
        else
          off_bit  = id%8;
          off_char = id/8;
          @veto_poke_bit[off_char] |= (1<<(off_bit))
        end
      end
      
      if(@local_debug == 1)
        if(flg == 1)
          p sprintf("%d :%s", i,@conv.get_cell(2, RULE_UP_START_COL+i, RULE_UP_START_ROW-1))
        end
      end
    }
  end
  
  #----------------
  # 禁止アイテム
  # (参照するシートが違うので注意)
  #----------------
  def chk_item()
    #
    WBTDL_ITEM_MAX.times{|i|
      # ブランクチェック
      ret = chkblank( @conv.get_cell(2, RULE_UP_START_COL+i, RULE_UP_START_ROW+4), 1+RULE_UP_START_COL+i, $rule_error_str[1]);
      if(ret == -1)
        wbt_exit();
      end
      #
      # セット
      flg = 0;
      num = hash2no( $ng_hash, @conv.get_cell(2, RULE_UP_START_COL+i, RULE_UP_START_ROW+4) );
      # NGの場合=>ビットを立てる
      if(num < 2)
        flg = 1;
        # アイテム名とIDを調べ、そこでの番号を使う
        # =>エクセルでの並びは使用しない
        str = @conv.get_cell(2, RULE_UP_START_COL+i, RULE_UP_START_ROW+4-1);
        id = hash2no( $item_hash, str );
        if(id == -1)
           # 該当するアイテムがない=>エラー
           print_error( str, 0 );
        else
          off_bit  = id%8;
          off_char = id/8;
          @veto_item_bit[off_char] |= (1<<(off_bit))
        end
      end
      
      if(@local_debug == 1)
        if(flg == 1)
          p sprintf("%d :%s", i,@conv.get_cell(2, RULE_UP_START_COL+i, RULE_UP_START_ROW+4-1))
        end
      end
    }
  end
  
  #----------------------------
  # ニックネーム表示(準備だけ)
  #----------------------------
  def chk_nickname()
    #
    # ブランクチェック
    ret = chkblank( @conv.get_cell(1, REG_UP_START_COL+15, REG_UP_START_ROW), 
                                      REG_UP_START_COL+15, $rule_error_str[10]);
    if(ret == -1)
      wbt_exit();
    end
    #
    # セット
    num = hash2no( $onoff_hash, @conv.get_cell(1, REG_UP_START_COL+15, REG_UP_START_ROW) );
#    if(num >= 2)
#      @both_info[i] = 1;
#    else
#      @both_info[i] = 0;
#    end
    #
    if(@local_debug == 1)
      p sprintf("%d :%s",num, $rule_error_str[10])
    end
  end
  
  #----------------
  # 禁止技
  #----------------
  def chk_skill()
    #
    # ブランクチェック
    ret = chkblank( @conv.get_cell(1, REG_UP_START_COL+22, REG_UP_START_ROW), 1+REG_UP_START_COL+22, $rule_error_str[1]);
    if(ret == -1)
      wbt_exit();
    end
    #
    str = @conv.get_cell(1, REG_UP_START_COL+22, REG_UP_START_ROW);
    id = hash2no( $wazano_hash, str );
    if(id == -1)
      # 該当するアイテムがない=>エラー
      print_error( str, 0 );
    else
      @veto_waza_no = id;
    end
    
    if(@local_debug == 1)
      p sprintf("%d :%s", id, @conv.get_cell(1, REG_UP_START_COL+22, REG_UP_START_ROW))
    end
  end
  
  #----------------
  # バトル方法
  #----------------
  def chk_battle_rule()
    #
    ret = chkblank( @conv.get_cell(1, REG_UP_START_COL+19, REG_UP_START_ROW), 
                    1+REG_UP_START_COL+19, $rule_error_str[3])
    if(ret == -1)
      wbt_exit();
    end
    # セット
    num = hash2no( $battle_rule_hash, @conv.get_cell(1, REG_UP_START_COL+19, REG_UP_START_ROW) );
    if(num == -1)
      print_error( @conv.get_cell(1, REG_UP_START_COL+19, REG_UP_START_ROW), 0 );
    else
      @btl_rule = num;
    end
    
    if(@local_debug == 1)
      p sprintf("%d :%s",@btl_rule, $rule_error_str[3] )
    end
  end
  
  #----------------
  # 必要タイプ
  #----------------
  def chk_type()
    #
    ret = chkblank( @conv.get_cell(1, REG_UP_START_COL+23, REG_UP_START_ROW), 
                                      REG_UP_START_COL+23, $rule_error_str[11])
    if(ret == -1)
      wbt_exit();
    end
    # セット
    num = hash2no( $type_hash, @conv.get_cell(1, REG_UP_START_COL+23, REG_UP_START_ROW) );
    if(num == -1)
      print_error( @conv.get_cell(1, REG_UP_START_COL+23, REG_UP_START_ROW), 0 );
    else
      @need_poketype = num;
    end
    
    if(@local_debug == 1)
      p sprintf("%d :%s",@need_poketype, $rule_error_str[11] )
    end
  end
  
  #----------------
  # BGMフラグ
  #----------------
  def chk_bgm()
    #
    # ブランクチェック
    ret = chkblank( @conv.get_cell(1, REG_UP_START_COL+24, REG_UP_START_ROW), 1+REG_UP_START_COL+25, $rule_error_str[1]);
    if(ret == -1)
      wbt_exit();
    end
    #
    num = @conv.get_cell(1, REG_UP_START_COL+24, REG_UP_START_ROW).to_i;
    @bgm_flag = num;
    #
    if(@local_debug == 1)
      p sprintf("%d :%s", num, $rule_error_str[5])
    end
  end
  
  #----------------
  # データのパック
  #----------------
  def pack()
    array = Array.new
    
    # パラメータの処理
    array << @join_info[0];
    array << @join_info[1];
    array << @join_info[2];
    array << @level_range;
    #
    array << @level_total;
    #
    array << @both_info[0];
    array << @both_info[1];
    #
    WBTDL_POKENUMBER_MAX.times{|i|
      array << @veto_poke_bit[i];
    }
    WBTDL_POKEITEM_MAX.times{|i|
      array << @veto_item_bit[i];
    }
    array << @pad;
    array << @btl_rule;
    array << @bgm_flag;
    array << @need_poketype;
    array << @veto_waza_no;
    
    # debug
    if(@local_debug == 1)
      p sprintf("rule test (0x%x)",(4+2+2+82+76+4+2));
    end
    
    # 出力形式をヘッダとあわせること
    return array.pack("C4SC2C82C76C4S")	# サイズ注意
  end
  
  #----------------
  # データのパック
  #----------------
  def wbt_exit()
    @conv.close_file()
    exit(1)
  end
  
end


