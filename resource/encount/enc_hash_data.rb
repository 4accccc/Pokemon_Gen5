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

$KCODE = "SJIS" #文字コードをSJISに設定。monsno_hash.rbの読み込みに必要
require File.dirname(__FILE__) + '/../personal/monsno_hash.rb'

class CEncHashData
  attr_accessor :white_data ,:black_data ,:black_enable ,:season_enable ,:black_ground_type ,:white_ground_type ,:ground_season_w ,:ground_season_b

  # 初期化
  def initialize
    @white_data = {GTYPE_GROUND=>{},GTYPE_WATER=>{},GTYPE_FISHING=>{}}
    @black_data = {GTYPE_GROUND=>{},GTYPE_WATER=>{},GTYPE_FISHING=>{}}
    @white_ground_type = GROUND_TYPE_NONE
    @black_ground_type = GROUND_TYPE_NONE
    @ground_season_w = [false,false,false]
    @ground_season_b = [false,false,false]
    @season_enable = false
  end

  # 季節を有効にする
  def set_season_enable
    @season_enable = true
    @ground_season_w = [true,true,true]
    @ground_season_b = [true,true,true]
  end

  # クラスを結合する
  def <<(other)

    # 季節あり優先
    if other.season_enable == true then
      set_season_enable()
    end

    # 地形タイプのマージ
    @white_ground_type =
      ground_type_merge( @white_ground_type ,other.white_ground_type )
    @black_ground_type =
      ground_type_merge( @black_ground_type ,other.black_ground_type )
   
    # データの結合
    concat_data( @white_data ,other.white_data )
    concat_data( @black_data ,other.black_data )

  end

  # 地形タイプのマージ
  def ground_type_merge(gtype,other_gtype)
    # 地形タイプを統合する
    if  gtype == GROUND_TYPE_NONE then
      gtype = other_gtype
    elsif other_gtype != GROUND_TYPE_NONE then
      # 混合タイプチェック
      unless gtype == other_gtype then
        gtype = GROUND_TYPE_MULTI
      end
    end

    return gtype
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
  
  # 季節の有効フラグをチェック
  def season_enable_check
    if @season_enable == true then
      GTYPE_MAX.times{|g_type|
        @ground_season_b[g_type] = check_season( g_type ,@black_data )
        @ground_season_w[g_type] = check_season( g_type ,@white_data )
      }
    end
  end

  # 季節チェック
  def check_season( g_type ,data )
    flg = false
    # 四季が全て存在するかチェック
    if data[g_type].size == 4 then
      for key_season in SEASON_SPRING..SEASON_AUTUMN
        for season in (key_season+1)..SEASON_WINTER
          if data[g_type][season].size == data[g_type][key_season].size then
            data[g_type][season].each{|key|
              if data[g_type][key_season].index(key) == nil then
                # 見つからない = 季節有り
                return true
              end
            }
          else
            # サイズが違う = 季節有り
            return true
          end
        end
      end
    else
      # 季節は存在するが、特定の季節のみエンカウントがなくなる場合
      unless data[g_type].size == 0 then
            return true
      end
    end

    # 四季なし
    return false
  end
end

class CFormData
  attr_accessor :monsno ,:formno ,:is_white ,:season ,:type_id
end

