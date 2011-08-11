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
  #----------------
  def get_param( langid )
    @data = []
    12.times{|i|
      str = @conv.get_cell(0, CONFIG_START_COL+langid, CONFIG_START_ROW+1+i)
      if(str == nil)
        break;
#        return( @data )
      end
      @data[i] = str;
      #
      if(@local_debug == 1)
        p sprintf("data :%d,%s",i,@data[i])
      end
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
        # 該当する性別がない=>エラー
        print_error( str, 0 );
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
  
end

