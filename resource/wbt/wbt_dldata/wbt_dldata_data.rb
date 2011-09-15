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
WBTDL_TOURNAMENT_EXPLAIN_LEN     = 111     	# 説明

# パラメータ開始位置
DATA_UP_START_ROW	= 3
DATA_UP_START_COL	= 3

# 各項目
DATA_TOURNAMENT		= 0
DATA_YEAR		= 1
DATA_MONTH		= 2
DATA_DAY		= 3
DATA_VERSION		= 4
#
DATA_LANG_JPN		= 5	# (0:JPN、1:ENG、2:FRA、3:ITA、4:GER、5:SPA、6:KOR)
DATA_STR_JPN		= 14
DATA_EXP00_JPN		= 23
DATA_EXP01_JPN		= 32
#
DATA_LANG_NUM		= 7
#
# 要整理
$title_str = ["トーナメントID",
                "配信開始年",
                "配信開始月",
                "配信開始日",
                "カセットバージョン",
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
    @day           = [0,1,1];
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
    # 配信開始日
    chk_day();
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
    ret = chkblank( @conv.get_cell(0,DATA_UP_START_COL+DATA_TOURNAMENT, DATA_UP_START_ROW ),
                                     DATA_UP_START_COL+DATA_TOURNAMENT, $title_str[0] )
    if(ret == -1)
      data_exit();
    end
    # セット
    @tournament_id = @conv.get_cell(0, DATA_UP_START_COL+DATA_TOURNAMENT, DATA_UP_START_ROW).to_i;
    
    # 範囲のチェック(1 - 65535)
    flg = chk_num_range( @tournament_id, 1, 65535 );
    if(flg == -1)
      print_error_num( $title_str[0], @tournament_id );
      data_exit();
    end
    
    if(@local_debug == 1)
      p sprintf("%s :%d", $title_str[0],@tournament_id)
    end
  end
  
  #----------------
  # 配信開始年月日
  #----------------
  def chk_day()
    #
    3.times{|i|
      ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+DATA_YEAR+i, DATA_UP_START_ROW), 
                                        DATA_UP_START_COL+DATA_YEAR+i, $title_str[1+i])
      if(ret == -1)
        data_exit();
      end
      # セット
      @day[i] = @conv.get_cell(0, DATA_UP_START_COL+DATA_YEAR+i, DATA_UP_START_ROW).to_i;
      
      # 範囲チェック
      if(i == 0)
        min = 0
        max = 99
      elsif(i == 1)
        min = 1
        max = 12
      elsif(i == 2)
        min = 1
        max = 31
      end
      #
      flg = chk_num_range( @day[i], min, max )
      if(flg == -1)
        print_error_num( $title_str[1+i], @day[i] );
        data_exit();
      end
      
      if(@local_debug == 1)
        p sprintf("%s :%d",$title_str[1+i], @day[i])
      end
    }
  end
  
  #------------------------
  # 対応カセットバージョン
  #------------------------
  def chk_version()
    #
    ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+4, DATA_UP_START_ROW),
                                      DATA_UP_START_COL+4, $title_str[4] )
    if(ret == -1)
      data_exit();
    end
    # セット
    num = hash2no( $version_hash, @conv.get_cell(0, DATA_UP_START_COL+4, DATA_UP_START_ROW) );
    if(num == -1)
      print_error_str( $title_str[4], @conv.get_cell(0, DATA_UP_START_COL+4, DATA_UP_START_ROW) );
      data_exit();
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
      ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+DATA_LANG_JPN+i, DATA_UP_START_ROW), 
                      1+DATA_UP_START_COL+DATA_LANG_JPN+i, $title_str[DATA_LANG_JPN+i])
      if(ret == -1)
        data_exit();
      end
      # セット
      num = hash2no( $ng_hash, @conv.get_cell(0, DATA_UP_START_COL+DATA_LANG_JPN+i, DATA_UP_START_ROW) );
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
    ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+14+@local_language, DATA_UP_START_ROW),
                                      DATA_UP_START_COL+14+@local_language, $title_str[12])
    if(ret == -1)
      data_exit();
    end
    #
    # 変換
    @tournament_name = get_unicode( 0, DATA_UP_START_COL+14+@local_language, DATA_UP_START_ROW )
    #
#    size = @tournament_name.size;
#    p sprintf("size :%d",size)
#    size.times{|i|
#      p sprintf( "%x",@tournament_name[i] )
#    }
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
    ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+23+@local_language, DATA_UP_START_ROW),
                                      DATA_UP_START_COL+23+@local_language, $title_str[13])
    if(ret == -1)
      data_exit();
    end
    
    # 変換
    @regulation_explain = get_unicode(0, DATA_UP_START_COL+23+@local_language, DATA_UP_START_ROW)
    
    # セット(コードの変換注意)
#    if(@local_debug == 1)
#      p sprintf("%s :%s", $title_str[13],@conv.get_cell(0, DATA_UP_START_COL+23+@local_language, DATA_UP_START_ROW))
#    end
  end
  
  #----------------------------------
  # 内容(言語IDを見ること)
  #----------------------------------
  def chk_contents_explain()
    #
    ret = chkblank( @conv.get_cell(0, DATA_UP_START_COL+32+@local_language, DATA_UP_START_ROW), 
                                      DATA_UP_START_COL+32+@local_language, $title_str[14])
    if(ret == -1)
      data_exit();
    end
    
    # 変換
    @contents_explain = get_unicode(0, DATA_UP_START_COL+32+@local_language, DATA_UP_START_ROW)
    
    # セット(コードの変換注意)
    if(@local_debug == 1)
      p sprintf("%s :%s", $title_str[14],@conv.get_cell(0, DATA_UP_START_COL+32+@local_language, DATA_UP_START_ROW))
    end
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

