####################################################################################
#
# @brief 子技テーブル
# @file kowaza_table.rb
# @author obata
# @date   2009.10.09
#
####################################################################################
require "hash"


#===================================================================================
# ■子技テーブル
#===================================================================================
file = File.open( "kowaza_table.txt", "r" )
$kowaza_table_data = file.readlines
file.close 


#===================================================================================
# ■定数
#===================================================================================
DATA_INDEX_MONS_NO      = 0   # モンスターナンバー
DATA_INDEX_MONS_NAME    = 2   # モンスター名
DATA_INDEX_EGG_GROUP_1  = 5   # 子作りグループ1
DATA_INDEX_EGG_GROUP_2  = 6   # 子作りグループ2
DATA_INDEX_KOWAZA_START = 7   # 子技リスト開始位置


#-----------------------------------------------------------------------------------
# @brief 子技リストを取得する
# @param mons_no モンスターナンバー
# @return 指定モンスターの子技リスト
#-----------------------------------------------------------------------------------
def GetKowazaList_byMonsNo( mons_no )
  # 配列を作成
  list = Array.new

  # 該当する行データを取得
  line = $kowaza_table_data[mons_no]
  if line==nil then return list end
  data = line.split(/\s/)

  # すべての子技をリストに追加
  if data!=nil then
    DATA_INDEX_KOWAZA_START.upto( data.size-1 ) do |index|
      waza = data[index]
      if waza!=nil then
        list << waza
      end
    end
  end
  return list
end


#-----------------------------------------------------------------------------------
# @brief 子技リストを取得する
# @param mons_name モンスター名
# @return 指定モンスターの子技リスト
#-----------------------------------------------------------------------------------
def GetKowazaList( mons_name )

  # モンスター番号を取得
  mons_no = GetMonsNo( mons_name )

  # モンスター番号からリストを取得
  return GetKowazaList_byMonsNo( mons_no ) 
end


#-----------------------------------------------------------------------------------
# @brief 指定した子作りグループを持つポケモン名をリストアップする
# @param group 子作りグループ1を指定
# @return 子作りグループ group を持つポケモン名配列
#-----------------------------------------------------------------------------------
def GetPokemonListByEggGroup( group )

  # 配列を作成
  list = Array.new

  # 子技テーブルを検索
  1.upto( $kowaza_table_data.size - 1 ) do |i|
    data = $kowaza_table_data[i].split(/\s/)
    # 指定グループを持つポケモン名を登録
    if data[DATA_INDEX_MONS_NAME]!=nil then
       if data[DATA_INDEX_EGG_GROUP_1].to_i==group ||
          data[DATA_INDEX_EGG_GROUP_2].to_i==group then
         list << data[DATA_INDEX_MONS_NAME]
       end
    end
  end 
  return list
end


#-----------------------------------------------------------------------------------
# @brief 指定した技を覚えるポケモンをリストアップする
# @param waza_name 技名
# @return 指定した技をレベルアップか技マシンで覚えるポケモン名の配列
#-----------------------------------------------------------------------------------
def GetPokemonListByWaza( waza_name ) 

  # 配列を作成
  list = Array.new

  # 指定技を覚えるポケモンを検索
  1.upto( $kowaza_table_data.size - 1 ) do |i|
    data = $kowaza_table_data[i].split(/\s/)
    name = data[DATA_INDEX_MONS_NAME]
    waza_list = $wazaoboe_hash[name]
    # 覚え技リストから指定技を検索
    if waza_list!=nil then
      waza_list.each do |waza|
        if waza==waza_name then
          list << name
        end
      end
    end
  end 

  return list
end 
