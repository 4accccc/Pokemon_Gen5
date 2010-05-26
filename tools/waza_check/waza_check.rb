require "wazaoboe_hash.rb"
require "wazaoboe_lv_hash.rb"

# レベルアップで技を覚えるか？
def CheckWazaOboe_byLv( mons_name, waza_name )
  wazaoboe_table = $wazaoboe_hash[ mons_name ]
  if wazaoboe_table.include?( waza_name ) then
    return true
  else
    return false
  end
end 

# 何レベルで覚えるか？
def GetWazaOboeLv( mons_name, waza_name )
  key = mons_name + "_" + waza_name
  return $wazaoboe_lv_hash[ key ]
end

# デフォルト技 取得
def GetDefaultWaza( mons_name, level )
  waza_list = Array.new # 最終的な技リスト
  wazaoboe_list = $wazaoboe_hash[ mons_name ]
  wazaoboe_list.each do |waza_name|
    waza_lv = GetWazaOboeLv( mons_name, waza_name )
    if waza_lv <= level then waza_list << waza_name end
  end
  while 4 < waza_list.size
    waza_list.slice!(0)
  end
  return waza_list
end
