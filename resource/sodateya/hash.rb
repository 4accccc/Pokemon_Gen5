#! ruby -Ks
require "C:/home/pokemon_wb/tools/hash/monsno_hash"   # モンスターNo.
require "C:/home/pokemon_wb/tools/hash/wazaoboe_hash" # 技覚え

#####################################################################################
#
# @brief  各種ハッシュと そのユーティリティー関数群
# @file   hash.rb
# @author obata
# @date   2009.10.09
#
####################################################################################

#===================================================================================
# ■技番号ハッシュ 
# $wazano_hash[技名]で技番号を取得
#===================================================================================
$wazano_hash = Hash::new

# 技ハッシュ作成
file = File.open( "../../prog/include/waza_tool/wazano_def.h", "r" )
name = String::new
file.each do |line|
  if line.index("WAZANO_MAX")!=nil then break end
  if line.index("//")!=nil then
    i0 = line.index("//") + 2
    i1 = line.length - 1
    name = line[i0..i1]
    name.strip!
  end
  if line.index("define")!=nil then
    i0 = line.index("(") + 1
    i1 = line.index(")") - 1
    str = line[i0..i1]
    no = str.to_i
    $wazano_hash[name] = no
  end
end
file.close


#-----------------------------------------------------------------------------------
# @brief 技ナンバーを取得する
# @param name 技名
# @return 技ナンバー
#-----------------------------------------------------------------------------------
def GetWazaNo( name )
  no = $wazano_hash[name]
  if no==nil then
    abort( "[#{name}]は技ハッシュに登録されていません。" )
  end
  return no
end



#===================================================================================
# ■モンスター名, モンスター番号
#===================================================================================

#-----------------------------------------------------------------------------------
# @brief モンスター名をモンスターナンバーに変換する
# @param name モンスター名
# @return モンスターナンバー
#-----------------------------------------------------------------------------------
def GetMonsNo( name )
  monsno = $monsno_hash[ name ]
  if monsno==nil then
    abort( "モンスターネーム[#{name}]はハッシュに登録されていません。" )
  end
  return monsno
end

#-----------------------------------------------------------------------------------
# @brief モンスターナンバーの最大値を取得する
# @return ポケモン数
#-----------------------------------------------------------------------------------
def GetMaxMonsNo()
  file = File.open( "../personal/monsno_def.h", "r" )
  file.each do |line|
    if line.index("MONSNO_END")!=nil then
      i0 = line.index("(") + 1
      i1 = line.index(")") - 1
      str = line[i0..i1]
      no = str.to_i
      return no
    end
  end
  file.close
  return 0
end
