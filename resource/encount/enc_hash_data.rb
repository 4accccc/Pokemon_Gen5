#//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#/**
# *
# *	@file		enc_hash_data.rb
# *	@brief  エンカウントデータクラス
# *	@author	Masayuki Onoue
# *	@date		2011.09.02
# *
# */
#//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


class CEncHashData
  attr_accessor :white_data ,:black_data ,:black_enable ,:season_enable

  # 初期化
  def initialize
    @white_data = {GTYPE_GROUND=>{},GTYPE_WATER=>{},GTYPE_FISHING=>{}}
    @black_data = {GTYPE_GROUND=>{},GTYPE_WATER=>{},GTYPE_FISHING=>{}}
  end

  # クラスを結合する
  def <<(other)

    # 季節あり優先
    if other.season_enable == true then
      @season_enable = true
    end
 
    # データの結合
    concat_data( @white_data ,other.white_data )
    concat_data( @black_data ,other.black_data )

  end
  
  # データ結合
  def concat_data dest , data
    data.each_key{|type|
      data[type].each_key{|season|
        if dest[type][season] == nil then
          dest[type][season] = Array.new
        end
        dest[type][season].concat(data[type][season])
        dest[type][season].uniq!
      }
    }
  end
  
  # 季節有りマップと季節なしマップが存在する場合は季節なしマップのデータを削除
  def merge_season
    if @season_enable == true then
      GTYPE_MAX.times{|g_type|
        # ブラックデータのマージン
        if @black_data[g_type].key?(SEASON_NONE) then
          no_season_data = @black_data[g_type][SEASON_NONE]
          @black_data[g_type][SEASON_SPRING].concat(no_season_data) 
          @black_data[g_type][SEASON_SUMMER].concat(no_season_data) 
          @black_data[g_type][SEASON_AUTUMN].concat(no_season_data) 
          @black_data[g_type][SEASON_WINTER].concat(no_season_data)
          @black_data[g_type].delete(SEASON_NONE)
          # 全てのキーをユニークにする
          @black_data[g_type].each_key{|season|
            @black_data[g_type][season].uniq!
          }
        end
        # ホワイトデータのマージン
        if @white_data[g_type].key?(SEASON_NONE) then
          no_season_data = @white_data[g_type][SEASON_NONE]
          @white_data[g_type][SEASON_SPRING].concat(no_season_data) 
          @white_data[g_type][SEASON_SUMMER].concat(no_season_data) 
          @white_data[g_type][SEASON_AUTUMN].concat(no_season_data) 
          @white_data[g_type][SEASON_WINTER].concat(no_season_data) 
          @white_data[g_type].delete(SEASON_NONE)
          # 全てのキーをユニークにする
          @white_data[g_type].each_key{|season|
            @white_data[g_type][season].uniq!
          }
        end
      }
    end
  end

end

