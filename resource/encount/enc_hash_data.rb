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

class CProbData
  attr_accessor :sum_prob
  def initialize(prob)
    @sum_prob = prob
  end
  def add_prob(prob)
    @sum_prob += prob
  end
  def  <<(other)
    @sum_prob += other.sum_prob
  end
end

class CEncHashData
  attr_accessor :white_data ,:black_data ,:black_enable ,:season_enable ,:black_ground_type ,:white_ground_type ,:ground_season_w ,:ground_season_b ,:white_prob ,:black_prob ,:zone_id_list

  # 初期化
  def initialize(zone_id)
    @white_data = {GTYPE_GROUND=>{},GTYPE_WATER=>{},GTYPE_FISHING=>{}}
    @black_data = {GTYPE_GROUND=>{},GTYPE_WATER=>{},GTYPE_FISHING=>{}}

    @white_prob = {GTYPE_GROUND=>{},GTYPE_WATER=>{},GTYPE_FISHING=>{}}
    @black_prob = {GTYPE_GROUND=>{},GTYPE_WATER=>{},GTYPE_FISHING=>{}}

    @white_ground_type = GROUND_TYPE_NONE
    @black_ground_type = GROUND_TYPE_NONE
    @ground_season_w = [false,false,false]
    @ground_season_b = [false,false,false]
    @season_enable = false
    @zone_id_list = Array.new
    @zone_id_list << zone_id
  end

  # 季節を有効にする
  def set_season_enable
    @season_enable = true
    @ground_season_w = [true,true,true]
    @ground_season_b = [true,true,true]
  end

  # クラスを結合する
  def combine_data(other ,zone_id)

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

    # 確率のマージ
    concat_prob_data( @white_prob ,other.white_prob )
    concat_prob_data( @black_prob ,other.black_prob )

    # ゾーンIDのリストを生成
    @zone_id_list << zone_id
    @zone_id_list.uniq!

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

  # 確率データ結合
  def concat_prob_data dest , data
    data.each_key{|type|
      data[type].each_key{|season|
        # 季節リストを生成
        if dest[type].key?(season) == false then
          dest[type][season] = Hash.new
        end
        data[type][season].each_key{|table_no|
          # テーブルリストを生成
          if dest[type][season].key?(table_no) == false then
            dest[type][season][table_no] = Hash.new
          end
          data[type][season][table_no].each_key{|no|
            # モンスターナンバーリストを生成
            if dest[type][season][table_no].key?(no) == false then
              dest[type][season][table_no][no] = data[type][season][table_no][no].clone
            else
              dest[type][season][table_no][no] << data[type][season][table_no][no]
            end
          }
        }
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

  def GetZoneCnt
    @zone_id_list.size
  end
end

class CFormData
  attr_accessor :monsno ,:formno ,:is_white ,:season ,:type_id
end

