#!ruby -Ks
#---------------------------
# wbt_dldata_data.rb
#---------------------------

require "wbt_dldata_util.rb"

#--------------
# 定義いろいろ
#--------------
WBTDL_TOURNAMENT_NAME_LEN	 = 37		# トーナメント名(21->37)
WBTDL_TOURNAMENT_REG_EXPLAIN_LEN = 111		# 参加制限
WBTDL_TOURNAMENT_EXPLAIN_LEN     = 75     	# 説明(111->75)

# パラメータ開始位置
DATA_UP_START_ROW	= 3
DATA_UP_START_COL	= 3

# 各項目
DATA_TOURNAMENT		= 0
DATA_VERSION		= 1
#
#DATA_LANG_JPN		= 2	# (0:JPN、1:ENG、2:FRA、3:ITA、4:GER、5:SPA、6:KOR)
DATA_STR_JPN		= 4
DATA_EXP00_JPN		= 13
DATA_EXP01_JPN		= 22
#
#DATA_LANG_NUM		= 7
#
# 要整理
$title_str = ["トーナメントID",
                "対応カセットバージョン",
                "トーナメント名",
                "参加制限メッセージ",
                "内容メッセージ",
]

#------------------------------------------------
#------------------------------------------------
class WBT_DATA
  #--------
  # 初期化
  #--------
  def initialize()
    # 生成のみ
  end
  
  #-------------------------------
  # データセット(内部の初期化あり)
  #-------------------------------
  def set_data( conv )
    @conv = conv;
    # 言語をセット
    @local_language = 0;
    chk_local_language();
    #
    @version       = 0;
    @day           = [0,1,1];	# 年月日用
    @lang_id       = 0;
    @tournament_id = 0;
    #
    @tournament_name = [];
    len = WBTDL_TOURNAMENT_NAME_LEN*2
    len.times{|i|
      @tournament_name[i] = 0;
    }
    @contents_explain = [];
    len = WBTDL_TOURNAMENT_EXPLAIN_LEN*2
    len.times{|i|
      @contents_explain[i] = 0;
    }
    @regulation_explain = [];
    len = WBTDL_TOURNAMENT_REG_EXPLAIN_LEN*2
    len.times{|i|
      @regulation_explain[i] = 0;
    }
    #
    @local_debug = 0;
    # debug
    if($debug_code == 1)
      @local_debug = 1;
    end
    
    # パラメータセット
    set_parameter();
  end
  
  #----------------
  # データのパック
  #----------------
  def pack2()
    array = Array.new
    
    # パラメータの処理
    array << @version
    array << @day[0]
    array << @day[1]
    array << @day[2]
    array << @lang_id
    array << @tournament_id
    #
    # debug
    if(@local_debug == 1)
      p sprintf("data test (size : 0x%x)",(4+4+2+42+222+222))
    end
    
    # 出力形式をヘッダとあわせること
    return array.pack("IC4S")
  end
  
  
  #----------------
  # データのパック
  #----------------
  def pack()
    array = Array.new
    
    array << pack2();
    array << str2pack( @tournament_name,    WBTDL_TOURNAMENT_NAME_LEN );
    array << str2pack( @contents_explain,   WBTDL_TOURNAMENT_EXPLAIN_LEN );
    array << str2pack( @regulation_explain, WBTDL_TOURNAMENT_REG_EXPLAIN_LEN );
    #
    return array
  end
  
  #----------------
  # パラメータセット
  #----------------
  def set_parameter()
    # トーナメントID
    chk_tournament_id();
    # カセットバージョン
    chk_version();
    # 対応言語
    chk_lang();
    # トーナメント名
    chk_tournament_name();
    # 参加制限
    chk_regulation_explain();
    # 内容
    chk_contents_explain();
  end
  
  #----------------
  # トーナメントID
  #----------------
  def chk_tournament_id()
    #
    cstr = @conv.get_cell(0,DATA_UP_START_COL+DATA_TOURNAMENT, DATA_UP_START_ROW );
    ret = chkblank( cstr, $title_str[0] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_TOURNAMENT, DATA_UP_START_ROW, $title_str[0]);
      return;
    end
    # セット
    @tournament_id = cstr.to_i;
    
    # 範囲のチェック(1 - 65535)
    flg = chk_num_range( @tournament_id, 1, 65535 );
    if(flg == -1)
      # 範囲エラー
      $wbt_error.print_range_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_TOURNAMENT, DATA_UP_START_ROW,
				    $title_str[0], @tournament_id, 1, 65535 );
    end
    
    if(@local_debug == 1)
      p sprintf("%s :%d", $title_str[0],@tournament_id)
    end
  end
  
  #------------------------
  # 対応カセットバージョン
  #------------------------
  def chk_version()
    #
    cstr = @conv.get_cell(0, DATA_UP_START_COL+DATA_VERSION, DATA_UP_START_ROW)
    ret = chkblank( cstr, $title_str[1] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_VERSION, DATA_UP_START_ROW, $title_str[4]);
      return;
    end
    
    # セット
    num = hash2no( $version_hash, cstr );
    if(num == -1)
      # 入力エラー
      $wbt_error.print_hash_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_VERSION, DATA_UP_START_ROW,
                                   $title_str[1], cstr );
    else
      @version = num;
    end
    if(@local_debug == 1)
      p sprintf("%s :%d", $title_str[1], @version)
    end
  end
  
  #------------------------
  # 対応言語
  #------------------------
  def chk_lang()
    # 仕様変更で全体の国コードを使用
    @lang_id = $current_lang_num
=begin
    #
    DATA_LANG_NUM.times{|i|
      cstr = @conv.get_cell(0, DATA_UP_START_COL+DATA_LANG_JPN+i, DATA_UP_START_ROW)
      ret = chkblank( cstr, $title_str[5+i] )
      if(ret == -1)
        # 空白エラー
        $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_LANG_JPN+i, DATA_UP_START_ROW,
				      $title_str[5+i]);
        return;
      end
      # セット
      num = hash2no( $ng_hash, cstr );
      if(num >= 2)
        @lang_id = 1+i;
        #
        if(@local_debug == 1)
          p sprintf("%s :%d", $title_str[5+i],@lang_id)
        end
        #
        break;
      end
    }
=end
  end
  
  #----------------------------------
  # トーナメント名(言語IDを見ること)
  #----------------------------------
  def chk_tournament_name()
    #
    ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+DATA_STR_JPN+@local_language, DATA_UP_START_ROW),
                                      $title_str[2] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_STR_JPN+@local_language, DATA_UP_START_ROW,
				    $title_str[2]);
      return;
    end
    #
    # 変換
    @tournament_name = get_unicode( @conv, 0, DATA_UP_START_COL+DATA_STR_JPN+@local_language, DATA_UP_START_ROW )
    # 入力チェック
    flg = 0
    if((@local_language == 0)||(@local_language == 6))
      flg = 1
    end
    array = []
    array = chk_str_limit_array( @tournament_name, 1, $str_limit[flg], $title_str[2] );
    if(array[4] == -1)
      # 文字入力エラー
      $wbt_error.print_mes_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_STR_JPN+@local_language, DATA_UP_START_ROW,
                                 $title_str[2], array, 1, $str_limit[flg] );
    end
    #
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s :%s", $title_str[2],@tournament_name)
#    end
  end
  
  #----------------------------------
  # 参加制限(言語IDを見ること)
  #----------------------------------
  def chk_regulation_explain()
    #
    ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW),
                                      $title_str[3] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW,
				    $title_str[3]);
      return;
    end
    
    # 変換
    @regulation_explain = get_unicode(@conv, 0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW)
    #
    # 入力チェック
    flg = 0
    if((@local_language == 0)||(@local_language == 6))
      flg = 1
    end
    array = []
    array = chk_str_limit_array( @regulation_explain, 3, $str_limit[2+flg], $title_str[3] );
    if(array[4] == -1)
      # 文字入力エラー
      $wbt_error.print_mes_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW,
                                 $title_str[3], array, 3, $str_limit[2+flg] );
    end
    #
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s :%s", $title_str[3],
#		@conv.get_cell(0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW))
#    end
  end
  
  #----------------------------------
  # 内容(言語IDを見ること)
  #----------------------------------
  def chk_contents_explain()
    #
    ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+DATA_EXP01_JPN+@local_language, DATA_UP_START_ROW),
                                      $title_str[4] )
    if(ret == -1)
      # 空白エラー
      $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP01_JPN+@local_language, DATA_UP_START_ROW,
                                    $title_str[4]);
      return;
    end
    
    # 変換
    @contents_explain = get_unicode(@conv, 0, DATA_UP_START_COL+DATA_EXP01_JPN+@local_language, DATA_UP_START_ROW)
    #
    # 入力チェック
    flg = 0
    if((@local_language == 0)||(@local_language == 6))
      flg = 1
    end
    array = []
    array = chk_str_limit_array( @contents_explain, 2, $str_limit[2+flg], $title_str[4] );
    if(array[4] == -1)
      # 文字入力エラー
      $wbt_error.print_mes_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP01_JPN+@local_language, DATA_UP_START_ROW,
                                 $title_str[4], array, 2, $str_limit[2+flg] );
    end
    
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s :%s", $title_str[4],
#		@conv.get_cell(0, DATA_UP_START_COL+DATA_EXP01_JPN+@local_language, DATA_UP_START_ROW))
#    end
  end
  
  #----------------------------------
  # 戻る処理
  #----------------------------------
  def data_exit()
    @conv.close_file();
    exit(1)
  end
  
  #----------------------------------
  # 言語チェック
  #----------------------------------
  def chk_local_language()
    # 本当はチェックがあるはず
    @local_language = $current_lang_id;
  end
end

