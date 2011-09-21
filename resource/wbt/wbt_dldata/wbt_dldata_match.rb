#!ruby -Ks
#---------------------------
# wbt_dldata_match.rb
#
#---------------------------

require "wbt_dldata_util.rb"

MATCH_START_COL  = 3
MATCH_START_ROW  = 3
#
WBT_MATCH_MAX	 = 7;	# 組み合わせデータ(WBT_TRAINER_NUM-1)

$match_str = ["設定方法",
              "優先度",
              "トレーナーID指定"
]

#------------------------------------------------
#------------------------------------------------
#------------------
# 組み合わせ(単体)
#------------------
class WBT_MATCH_SINGLE
  #--------
  # 初期化
  #--------
  def initialize( num )
    @match_id = num;
    
  end
  
  #--------------------------------
  # データセット(内部で初期化あり)
  #--------------------------------
  def set_data( conv )
    @conv = conv;
#    @match_id = num;
    #
    @type        = 0;
    @pri         = 0;
    @id          = 0;
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
    # 設定方法
    chk_match()
    # 優先度
    chk_prio();
    # ID
    chk_id();
  end
  
  #----------------
  # 設定方法
  #----------------
  def chk_match()
    #
    ret = chkblank( @conv.get_cell(3, MATCH_START_COL, MATCH_START_ROW+@match_id ),
                                      $match_str[0] )
    if(ret == -1)
      print_error_info( $current_xls_name, 3, MATCH_START_COL, MATCH_START_ROW+@match_id );
      match_exit();
    end
    # セット
    num = hash2no( $match_hash, @conv.get_cell(3, MATCH_START_COL, MATCH_START_ROW+@match_id) );
    if(num == -1)
      print_error_str( $match_str[0], @conv.get_cell(3, MATCH_START_COL, MATCH_START_ROW+@match_id) );
      print_error_info( $current_xls_name, 3, MATCH_START_COL, MATCH_START_ROW+@match_id );
      match_exit();
    else
      @type = num;
    end
    
    if(@local_debug == 1)
      p sprintf("%d :%s",@type, $match_str[0] )
    end
  end
  
  #----------------
  # 優先度
  #----------------
  def chk_prio()
    #
    ret = chkblank( @conv.get_cell(3, MATCH_START_COL+1, MATCH_START_ROW+@match_id ),
                                      $match_str[1] )
    if(ret == -1)
      print_error_info( $current_xls_name, 3, MATCH_START_COL+1, MATCH_START_ROW+@match_id );
      match_exit();
    end
    # セット
    @pri = @conv.get_cell(3, MATCH_START_COL+1, MATCH_START_ROW+@match_id).to_i
    # 範囲のチェック(0 - 5)
    flg = chk_num_range( @pri, 0, 5 );
    if(flg == -1)
      print_error_num( $match_str[1], @pri, 0, 5 );
      print_error_info( $current_xls_name, 3, MATCH_START_COL+1, MATCH_START_ROW+@match_id );
      match_exit();
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $match_str[1], @pri )
    end
  end
  
  #----------------
  # トレーナーID指定
  #----------------
  def chk_id()
    #
    ret = chkblank( @conv.get_cell(3, MATCH_START_COL+2, MATCH_START_ROW+@match_id ),
                                      $match_str[2] )
    if(ret == -1)
      print_error_info( $current_xls_name, 3, MATCH_START_COL+2, MATCH_START_ROW+@match_id );
      match_exit();
    end
    #
    num = hash2no( $skip_hash, @conv.get_cell(3, MATCH_START_COL+2, MATCH_START_ROW+@match_id) );
    if(num == -1)
      @id = @conv.get_cell(3, MATCH_START_COL+2, MATCH_START_ROW+@match_id).to_i
    else
      @id = 0
    end
    # 範囲のチェック(0 - 999)
    flg = chk_num_range( @id, 0, 999 );
    if(flg == -1)
      print_error_num( $match_str[2], @id, 0, 999 );
      print_error_info( $current_xls_name, 3, MATCH_START_COL+2, MATCH_START_ROW+@match_id );
      match_exit();
    end
    #
    if(@local_debug == 1)
      p sprintf("%s :%d", $match_str[2], @id )
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
  
  #----------------------------------
  # 戻る処理
  #----------------------------------
  def match_exit()
    @conv.close_file();
    exit(1)
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
  def initialize()
    @match = [];
    WBT_MATCH_MAX.times{|i|
      @match[i] = WBT_MATCH_SINGLE.new( i )
    }
    #
  end
  
  #--------------------------------
  # データセット(内部で初期化あり)
  #--------------------------------
  def set_data( conv )
    @conv = conv;
    #
    WBT_MATCH_MAX.times{|i|
      @match[i].set_data(conv)
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

