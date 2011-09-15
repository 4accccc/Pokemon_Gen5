#!ruby -Ks
#---------------------------
# wbt_dldata_config.rb
#
# 設定ファイル
#---------------------------

require "wbt_dldata_util.rb"

# パラメータ開始位置
CONFIG_START_ROW	= 3
CONFIG_START_COL	= 3

# 文字列
$config_str = [ "出力",
]

#------------------------------------------------
#------------------------------------------------
#------------------
# 設定ファイル
#------------------
class WBT_CONFIG
  #--------
  # 初期化
  #--------
  def initialize( conv )
    @conv = conv;
    @data = []
    @output = []
    #
    @local_debug = 0;
    # debug
    if($debug_code == 1)
      @local_debug = 1;
    end
  end
  
  #----------------
  # パラメータの取得
  # (nilでも入れておく)
  #----------------
  def get_param()
    @data = []
    7.times{|j|
      12.times{|i|
        str = @conv.get_cell(0, CONFIG_START_COL+j, CONFIG_START_ROW+1+i)
        @data[12*j+i] = str;
        #
        if(@local_debug == 1)
          p sprintf("data :%d,%s",i,@data[12*j+i])
        end
      }
    }
    return( @data )
  end
  
  #----------------
  # 出力ファイルの取得
  #----------------
  def get_output()
    @output = []
    7.times{|i|
      str = @conv.get_cell(0, CONFIG_START_COL+i, CONFIG_START_ROW)
      # セット
      id = hash2no( $ox_hash, str );
      if(id == -1)
        # 該当する「○×」がない=>エラー
        print_error_str( $config_str[0], str );
        config_exit();
      else
        @output[i] = id;
      end
      #
      if(@local_debug == 1)
        p sprintf("output :%d,%d",i,@output[i])
      end
    }
    return( @output )
  end
  
  #----------------------------------
  # 戻る処理
  #----------------------------------
  def config_exit()
    @conv.close_file();
    exit(1)
  end
  
end

