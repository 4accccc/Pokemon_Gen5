#//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#/**
# *
# *	@file		zkn_habitat_make.rb
# *	@brief  生息地データ出力
# *	@author	Masayuki Onoue
# *	@date		2011.09.02
# *
# */
#//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

# データハッシュ型を取得
load    File.dirname(__FILE__) + '/../encount/encount_common.def'
require File.dirname(__FILE__) + '/../encount/enc_hash_data.rb'
require File.dirname(__FILE__) + '/../fldmapdata/zonetable/zonegroup_hash/zone_group_hash.rb'

# 出力形式用モンスターデータ
class CMonsterData
  attr_accessor :season_enable ,:id ,:is_appear
  
  def initialize

    # is_appear[季節][地形]
    @is_appear = Array.new( 4, Array.new(3,false) )
    # 季節の存在情報
    @season_enable = false
  end

end

# 出力用モンスターデータ配列を作成する
def set_mons_array_data( group_data ,mons_array ,mons_data )
  if group_data.season_enable == false then
    # 季節なしデータ設定
    GTYPE_MAX.times{|g_type|
      if mons_data[g_type].size <= 0 then next end
      mons_data[g_type][SEASON_NONE].each{|id|
        # 登録されていない場合は生成
        unless mons_array.key?(id) then
          mons_array[id] =  CMonsterData.new
        end
        for season in SEASON_SPRING..SEASON_WINTER
          mons_array[id].is_appear[season-SEASON_SPRING][g_type] = true
        end
      }
    }
  else
    # 季節ありデータ生成
    GTYPE_MAX.times{|g_type|
      if mons_data[g_type].size <= 0 then next end
      mons_data[g_type].each_key{|season|
        mons_data[g_type][season].each{|id|
          # 登録されていない場合は生成
          unless mons_array.key?(id) then
            mons_array[id] =  CMonsterData.new
          end
          mons_array[id].is_appear[season-SEASON_SPRING][g_type] = true
        }
      }
    }
  end
end

# バイナリデータを出力する
def put_bin_file( f_name ,group_data ,mons_array ,group_type )
  bin = open(f_name,"w")
  
  # 季節の有効状態
  if group_data.season_enable == true then
    bin << [ 1 ].pack( "C" )
  else
    bin << [ 0 ].pack( "C" )
  end

  # 地形の有効状態を設定
  landtype_enable = [0,0,0] 
  mons_array.each_key{|key|
    for season in SEASON_SPRING..SEASON_WINTER
      GTYPE_MAX.times{|g_type|
        if mons_array[key].is_appear[season-SEASON_SPRING][g_type] == true then
          landtype_enable[ g_type ] = 1
        end
      }
    end
  }
  bin << [ landtype_enable[GTYPE_GROUND] ].pack( "C" )  # 地面
  bin << [ landtype_enable[GTYPE_WATER] ].pack( "C" )   # 海
  bin << [ landtype_enable[GTYPE_FISHING] ].pack( "C" ) # 釣り
  
  # 地形タイプ
  bin << [ group_type.to_i ].pack( "S" )
 
  # データ数
  bin << [ mons_array.size ].pack( "S" )

  # 出現データを出力
  mons_array.each_key{|key|
    # モンスター番号
    bin << [ key.to_i ].pack( "s" )
    for season in SEASON_SPRING..SEASON_WINTER
      GTYPE_MAX.times{|g_type|
        flg = mons_array[key].is_appear[season - SEASON_SPRING][g_type]
        if flg == true then
          bin << [ 1 ].pack( "c" ) 
        else
          bin << [ 0 ].pack( "c" ) 
        end
      }
    end
    # パディング
    bin << [ 0,0 ].pack( "c2" ) 
  }
  bin.close
end


# 季節名
season_name = ["四季なし","春","夏","秋","冬"]
# 地形名
ground_type_name = ["土","海","釣"]
# 地形名詳細
info_ground_type_name = ["草地","砂地","沼","洞窟","混合","なし"]

# エンカウントデータの読み込み
encount_data          = nil
FNAME_MARSHALL        = ARGV[0].to_s
FNAME_DATA_INDEX      = ARGV[1].to_s
ARC_FILE_LIST_W_NAME  = ARGV[2].to_s
ARC_FILE_LIST_B_NAME  = ARGV[3].to_s
open(FNAME_MARSHALL,"rb"){|file|
  str =  file.read
  encount_data = Marshal.load(str)
}

# ゾーングループごとの格納先
zone_group_data = Hash.new

# ゾーングループごとにデータをマージする
encount_data.each_key{|zone_id|
  # エラーチェック
  unless $zohe_id_to_zone_group.key?(zone_id.upcase) then raise "不明なゾーンID" end
  zone_group = $zohe_id_to_zone_group[zone_id.upcase]
  # キーが存在しない場合は、データ格納先を作成する
  unless zone_group_data.key?(zone_group) then
    # コピーを生成
    zone_group_data[zone_group] = Marshal.load(Marshal.dump(encount_data[zone_id]))
  else
    # マージ処理を行う
    zone_group_data[zone_group] << encount_data[zone_id]
  end
}

# 四季あり、四季なしデータが共存している場合をまとめる
zone_group_data.each_key{|zone_group|
  zone_group_data[zone_group].merge_season
}


=begin

# マージしたデータを格納する
zone_group_data.each_key{|zone_group|
  data = zone_group_data[zone_group]

  puts "=============================================================="
  puts "【 %s(ホワイト) 】"%[zone_group]
  puts " 四季 :%s"%[data.season_enable]
  puts " 黒有 :%s"%[data.black_enable]
  puts " 白地形 :%s"%[info_ground_type_name[data.white_ground_type]]
  puts " 黒地形 :%s"%[info_ground_type_name[data.black_ground_type]]

  # ホワイト
  GTYPE_MAX.times{|g_type|
    unless data.white_data[g_type] == nil then
      puts "【 %s 】"%[ground_type_name[g_type]]
      data.white_data[g_type].each_key{|season|
        print "%s(%2d種類):"%[season_name[season],data.white_data[g_type][season].size]
        if data.white_data[g_type][season].size > 20 then
          #raise "20種類以上使用されています"
        end
        data.white_data[g_type][season].each{|mons_data|
          print "%s,"%[mons_data]
        }
        print "\n"
      }
    end
  }
  print "\n"
  
  puts "--------------------------------------------------------------"

  # ブラック
  puts "【 %s(ブラック) 】"%[zone_group]
  GTYPE_MAX.times{|g_type|
    unless data.black_data[g_type] == nil then
      puts "【 %s 】"%[ground_type_name[g_type]]
      data.black_data[g_type].each_key{|season|
        print "%s(%2d種類):"%[season_name[season],data.black_data[g_type][season].size]
        data.black_data[g_type][season].each{|mons_data|
          print "%s,"%[mons_data]
        }
        print "\n"
        if data.black_data[g_type][season].size > 20 then
          #raise "20種類以上使用されています"
        end
      }
    end
  }

  print "\n"
}

=end

# バイナリデータの出力
zone_group_data.each_key{|zone_group|
  b_mons_array = Hash.new
  w_mons_array = Hash.new
  group_data =  zone_group_data[zone_group]
  b_mons_data = group_data.black_data
  w_mons_data = group_data.white_data

  # ブラック用データ作成
  set_mons_array_data( group_data,b_mons_array ,b_mons_data )
  put_bin_file( "hb_b_%s.bin"%[zone_group.downcase] ,group_data ,b_mons_array,group_data.black_ground_type )


  # ホワイト用データ作成
  set_mons_array_data( group_data,w_mons_array ,w_mons_data)
  put_bin_file( "hb_w_%s.bin"%[zone_group.downcase] ,group_data ,w_mons_array,group_data.white_ground_type )
}

# データインデックステーブルを作成
file = open(FNAME_DATA_INDEX,"w")
list_file_w = open(ARC_FILE_LIST_W_NAME,"w")
list_file_b = open(ARC_FILE_LIST_B_NAME,"w")
header =<<EOS
#//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
#/**
# *
# *	@file		%s
# *	@brief  生息地リスト用エンカウント解析データアーカイブインデックス
# *	@author	Masayuki Onoue
# *	@date		2011.09.04
# *
# */
#//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

$habitat_arc_idx_table = {
EOS

# ヘッダ部分出力
file.puts header%[FNAME_DATA_INDEX,zone_group_data.size]
index = 0
zone_group_data.each_key{|zone_group|
  file.puts '  "ZONE_ID_%s"=>%d,'%[zone_group,index]
  list_file_w.puts '"hb_w_%s.bin"'%[zone_group.downcase]
  list_file_b.puts '"hb_b_%s.bin"'%[zone_group.downcase]
  index += 1
}
file.puts "}"

file.close
list_file_w.close
list_file_b.close

