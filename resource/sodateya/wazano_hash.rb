#===================================================================================
# 技番号ハッシュ
#
# $wazano_hash[ 技名 ] で技番号を取得
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
  elsif line.index("define")!=nil then
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
