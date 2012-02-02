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

require "csv"

class MonsData
  attr_reader :file_name,:data
  # モンスター名
  def initialize( file_name, data )
    @file_name = file_name
    @data = data
  end
end

class ErrorData
  attr_reader :a_file,:a_index ,:b_file ,:b_index
  def initialize( a_file,a_index ,b_file,b_index )
    @a_file  = a_file
    @a_index = a_index
    @b_file  = b_file
    @b_index = b_index
  end
end

# エラーリスト
$error_list = Array.new

def check_data( file_list )

  # 全モンスターリスト
  all_mons_array =Array.new

  # 登録されているポケモンデータ全てをリスト化
  file_list.each{|file_name|
    poke_data     = BSUB_POKE.new(file_name)
    poke_data.mons_list.each{|data|
      all_mons_array << MonsData.new(file_name,data)
    }
  }

  # 全ポケモンで同じ技構成のポケモンを検索
  all_size = all_mons_array.size

  # 総比較
  for i in 0..(all_size-2)
    for j in (i+1)..(all_size-1)

      a = all_mons_array[i].data
      b = all_mons_array[j].data
      a_file = all_mons_array[i].file_name
      b_file = all_mons_array[j].file_name

      # 同じ名前,同じフォルムのポケモン同士を比較
      if a.full_name == b.full_name then
        # 所持技数が同じ
        if a.waza_list.size == b.waza_list.size then

          same = true
          # 全技を持っているか
          a.waza_list.each{|waza_name|
            # 違う技を持っているか
            unless b.waza_list.include?(waza_name) then
              same = false
              break
            end
          }

          # 全技が同じ場合
          if same == true then
            $error_list << ErrorData.new(a_file,a.index,b_file,b.index)
          end
        end
      end
    end
  end

end


# 無限
inf_file = [ENV["PROJECT_ROOT"]+'resource\infinity_dungeon\inf_pm.pdm',]
check_data( inf_file )
# PWT
pwt_file = Dir[ ENV["PROJECT_ROOT"]+'resource/wbt/**/*.pdm' ]
check_data( pwt_file )


# データ出力
CSV.open("../btl_subway_same_waza_check_result.csv", 'w'){|writer|
  writer << [ "file_name_a", "index_a", "file_name_b", "index_b" ]
  # 検索結果出力
  $error_list.each{|data|
    # インデックスは１オリジンで表示
    writer << [ File.basename(data.a_file),data.a_index+1 ,File.basename(data.b_file) ,data.b_index+1 ]
  }
}
