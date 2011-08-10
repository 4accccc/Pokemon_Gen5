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
      str = @conv.get_cell(0, CONFIG_START_COL+langid, CONFIG_START_ROW+i)
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
  
end

