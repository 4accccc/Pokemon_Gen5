#[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#
#	@file		btl_subway_poke_waza_check_main.rb
#	@brief  バトルサブウェイポケモンデータの技をチェックする
#	@author	Masayuki Onoue
#	@date		2012.01.17
#
#]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
$KCODE = 'SJIS'
require File.dirname(__FILE__) + "/hash/monsno_hash.rb"
require File.dirname(__FILE__) + "/btl_subway/btl_subway_poke_purser.rb"

require "personal_accessor.rb"
require "trdata_parser.rb"
require "oshiewaza_hash.rb"
require "waza_oshie_hash.rb"
require "fr_old_owaza_hash.rb"
require "gs_old_owaza_hash.rb"
require "pl_old_owaza_hash.rb"
require "old_oshiewaza_hash.rb"
require "csv"

class ErrorData
  attr_reader :file_name,:index ,:mons_name ,:waza
  def initialize( file_name,index ,mons_name ,waza )
    @file_name  = file_name
    @index      = index
    @mons_name  = mons_name
    @waza       = waza
  end
end

# 教え技か調べる
def is_oshiewaza( waza_name )
  if $oshie_waza_hash_red.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_blue.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_yellow.values.include?( waza_name ) then return true end
  if $oshie_waza_hash_green.values.include?( waza_name ) then return true end

  return false
end

# 教え技か調べる
def is_old_oshiewaza( mons_name,waza_name )
  # 過去教え技
  if check_old_waza_hash($old_oshiewaza_hash,mons_name,waza_name,waza_name ) then
    return true
  end
  return false
end

# 過去技か調べる
def is_old_waza( mons_name ,waza_name )

  # fr
  if check_old_waza_hash($fr_old_owaza_hash ,mons_name ,waza_name , waza_name ) then
    return true
  end
  # gs
  if check_old_waza_hash($gs_old_owaza_hash ,mons_name ,waza_name , waza_name ) then 
    return true
  end
  # pl
  if check_old_waza_hash($pl_old_owaza_hash ,mons_name ,waza_name , waza_name ) then 
    return true 
  end

  return false
end

# ハッシュ内をチェック
def check_old_waza_hash( hash, mons_name ,waza_name , waza )
  if hash.key?(mons_name) then
    if hash[mons_name].include?(waza) then return true end
  end
  return false
end


# メッセージファイルの位置
MSG_FILES = ENV["PROJECT_ROOT"]+"resource/**/*.pdm" 
subway_poke_file_names = Dir[ MSG_FILES ]

# パーソナルデータ参照
personal_accessor = PersonalAccessor.new

# エラーリスト
error_list = Array.new

# 登録されているポケモンデータ全てを検査
subway_poke_file_names.each{|file_name|
  poke_data = BSUB_POKE.new(file_name)
  poke_data.mons_list.each{|data|

    # 全技をチェック
    data.waza_list.each{|waza|
      
      #puts data.full_name+":"+waza
      # 覚え技かレベル技
      oshiewaza_list = $oshiewaza_hash[ data.full_name ]
      # レベル制限はかけない
      unless personal_accessor.check_waza_learning( data.full_name, 100, waza ) ||
        (is_oshiewaza( waza ) == true && oshiewaza_list.include?( waza ) == true) ||
        (is_old_waza( data.full_name ,waza ) == true ) ||
        (is_old_oshiewaza( data.full_name,waza) == true) then
        error_list << ErrorData.new( file_name,data.index ,data.mons_name ,waza )
      end
    }
  }
}

  # データ出力
CSV.open("../btl_subway_waza_check_result.csv", 'w'){|writer|
  writer << [ "file_name", "index", "monster_name" ,"waza_name" ]
  # 検索結果出力
  error_list.each{|data|
    writer << [ data.file_name,(data.index+1), data.mons_name , data.waza ]
  }
}
