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
DATA_LANG_JPN		= 2	# (0:JPN、1:ENG、2:FRA、3:ITA、4:GER、5:SPA、6:KOR)
DATA_STR_JPN		= 11
DATA_EXP00_JPN		= 20
DATA_EXP01_JPN		= 29
#
DATA_LANG_NUM		= 7
#
# 要整理
$title_str = ["トーナメントID",
                "配信開始年",			# 未使用
                "配信開始月",			# 未使用
                "配信開始日",			# 未使用
                "対応カセットバージョン",
                "対応言語 日本語",
                "対応言語 英語",
                "対応言語 フランス語",
                "対応言語 イタリア語",
                "対応言語 ドイツ語",
		
                "対応言語 スペイン語",
                "対応言語 韓国語",
                "トーナメント名",
                "参加制限メッセージ",
                "内容メッセージ"
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
#      print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_TOURNAMENT, DATA_UP_START_ROW );
#      data_exit();
      $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_TOURNAMENT, DATA_UP_START_ROW );
    end
    # セット
    @tournament_id = cstr.to_i;
    
    # 範囲のチェック(1 - 65535)
    flg = chk_num_range( @tournament_id, 1, 65535 );
    if(flg == -1)
#      print_error_num( $title_str[0], @tournament_id, 1, 65535 );
#      print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_TOURNAMENT, DATA_UP_START_ROW );
#      data_exit();
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
    ret = chkblank( cstr, $title_str[4] )
    if(ret == -1)
#      print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_VERSION, DATA_UP_START_ROW );
#      data_exit();
      $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_VERSION, DATA_UP_START_ROW );
    end
    # セット
    num = hash2no( $version_hash, cstr );
    if(num == -1)
#      print_error_str( $title_str[4], cstr );
#      print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_VERSION, DATA_UP_START_ROW );
#      data_exit();
      $wbt_error.print_hash_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_VERSION, DATA_UP_START_ROW,
                                   $title_str[4], cstr );
    else
      @version = num;
    end
    if(@local_debug == 1)
      p sprintf("%s :%d", $title_str[4], @version)
    end
  end
  
  #------------------------
  # 対応言語
  #------------------------
  def chk_lang()
    #
    DATA_LANG_NUM.times{|i|
      cstr = @conv.get_cell(0, DATA_UP_START_COL+DATA_LANG_JPN+i, DATA_UP_START_ROW)
      ret = chkblank( cstr, $title_str[DATA_LANG_JPN+i] )
      if(ret == -1)
#        print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_LANG_JPN+i, DATA_UP_START_ROW );
#        data_exit();
        $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_LANG_JPN+i, DATA_UP_START_ROW );
      end
      # セット
      num = hash2no( $ng_hash, cstr );
      if(num >= 2)
        @lang_id = 1+i;
        #
        if(@local_debug == 1)
          p sprintf("%s :%d", $title_str[DATA_LANG_JPN+i],@lang_id)
        end
        #
        break;
      end
    }
  end
  
  #----------------------------------
  # トーナメント名(言語IDを見ること)
  #----------------------------------
  def chk_tournament_name()
    #
    ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+DATA_STR_JPN+@local_language, DATA_UP_START_ROW),
                                      $title_str[12] )
    if(ret == -1)
#      print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_STR_JPN+@local_language, DATA_UP_START_ROW );
#      data_exit();
      $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_STR_JPN+@local_language, DATA_UP_START_ROW );
    end
    #
    # 変換
    @tournament_name = get_unicode( @conv, 0, DATA_UP_START_COL+DATA_STR_JPN+@local_language, DATA_UP_START_ROW )
    # 入力チェック
    flg = 0
    if((@local_language == 0)||(@local_language == 6))
      flg = 1
    end
#    flg = chk_str_limit( @tournament_name, 1, $str_limit[flg], $title_str[12] );
#    if(flg == -1)
#      print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_STR_JPN+@local_language, DATA_UP_START_ROW );
#      data_exit();
#    end
    array = []
    array = chk_str_limit_array( @tournament_name, 1, $str_limit[flg], $title_str[12] );
    if(array[4] == -1)
#      print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_STR_JPN+@local_language, DATA_UP_START_ROW );
#      data_exit();
      $wbt_error.print_mes_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_STR_JPN+@local_language, DATA_UP_START_ROW,
                                 $title_str[12], array, 1, $str_limit[flg] );
    end
    #
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s :%s", $title_str[12],@tournament_name)
#    end
  end
  
  #----------------------------------
  # 参加制限(言語IDを見ること)
  #----------------------------------
  def chk_regulation_explain()
    #
    ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW),
                                      $title_str[13] )
    if(ret == -1)
#      print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW );
#      data_exit();
      $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW);
    end
    
    # 変換
    @regulation_explain = get_unicode(@conv, 0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW)
    #
    # 入力チェック
    flg = 0
    if((@local_language == 0)||(@local_language == 6))
      flg = 1
    end
#    flg = chk_str_limit( @regulation_explain, 3, $str_limit[2+flg], $title_str[13] );
#    if(flg == -1)
#      print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW );
#      data_exit();
#    end
    array = []
    array = chk_str_limit_array( @regulation_explain, 3, $str_limit[2+flg], $title_str[13] );
    if(array[4] == -1)
      $wbt_error.print_mes_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW,
                                 $title_str[13], array, 3, $str_limit[2+flg] );
    end
    #
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s :%s", $title_str[13],
#		@conv.get_cell(0, DATA_UP_START_COL+DATA_EXP00_JPN+@local_language, DATA_UP_START_ROW))
#    end
  end
  
  #----------------------------------
  # 内容(言語IDを見ること)
  #----------------------------------
  def chk_contents_explain()
    #
    ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+DATA_EXP01_JPN+@local_language, DATA_UP_START_ROW),
                                      $title_str[14] )
    if(ret == -1)
#      print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP01_JPN+@local_language, DATA_UP_START_ROW );
#      data_exit();
      $wbt_error.print_blank_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP01_JPN+@local_language, DATA_UP_START_ROW);
    end
    
    # 変換
    @contents_explain = get_unicode(@conv, 0, DATA_UP_START_COL+DATA_EXP01_JPN+@local_language, DATA_UP_START_ROW)
    #
    # 入力チェック
    flg = 0
    if((@local_language == 0)||(@local_language == 6))
      flg = 1
    end
#    flg = chk_str_limit( @contents_explain, 2, $str_limit[2+flg], $title_str[14] );
#    if(flg == -1)
#      print_error_info( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP01_JPN+@local_language, DATA_UP_START_ROW );
#      data_exit();
#    end
    array = []
    array = chk_str_limit_array( @contents_explain, 2, $str_limit[2+flg], $title_str[14] );
    if(array[4] == -1)
      $wbt_error.print_mes_error( $current_xls_name, 0, DATA_UP_START_COL+DATA_EXP01_JPN+@local_language, DATA_UP_START_ROW,
                                 $title_str[14], array, 2, $str_limit[2+flg] );
    end
    
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s :%s", $title_str[14],
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

