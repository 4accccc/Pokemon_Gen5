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

$KCODE = "SJIS" #文字コードをSJISに設定。monsno_hash.rbの読み込みに必要

# データハッシュ型を取得
load    File.dirname(__FILE__) + '/../encount/encount_common.def'
require File.dirname(__FILE__) + '/../encount/enc_hash_data.rb'
require File.dirname(__FILE__) + '/../fldmapdata/zonetable/zonegroup_hash/zone_group_hash.rb'
require File.dirname(__FILE__) + '/../personal/monsno_hash.rb'
load    File.dirname(__FILE__) + '/../personal/chihou_zukanno_hash.rb'

# 季節名
$season_name = ["四季なし","春","夏","秋","冬"]
# 地形名
$ground_type_name = ["土","海","釣"]
# 地形名詳細
$info_ground_type_name = ["草地","砂地","沼","洞窟","橋","混合","なし"]

$ZENKOKU_POKE_ZONE_FILE_NAME = "zenkoku_poke_zone.csv"
$ZONE_POKE_INFO_FILE_NAME = "zone_poke_info.txt"

# 出力形式用モンスターデータ
class CMonsterData
  attr_accessor :season_enable ,:id ,:is_appear
  
  def initialize
    # is_appear[季節][地形]
    @is_appear = Array.new
    SEASON_WINTER.times{
      @is_appear << [false,false,false]
    }
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
            mons_array[id].season_enable = true
          end
          mons_array[id].is_appear[(season-SEASON_SPRING)][g_type] = true
        }
      }
    }
  end
end

# バイナリデータを出力する
def put_bin_file( f_name ,group_data ,mons_array ,group_type ,zone_group,season_enable,is_white)
  bin = open(f_name,"w")
  
  # 季節の有効状態
  GTYPE_MAX.times{|g_type|
    if season_enable[g_type] == true then
      bin << [ 1 ].pack( "C" )
    else
      bin << [ 0 ].pack( "C" )
    end
  }

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
    # 出現条件
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
    # フォルム
    for season in SEASON_SPRING..SEASON_WINTER
      GTYPE_MAX.times{|g_type|
        flg = mons_array[key].is_appear[season - SEASON_SPRING][g_type]
        if flg == true then
          bin << [ search_form( key,season,g_type,zone_group ,is_white ,group_data.season_enable) ].pack("c")
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

# フォルム取得
def search_form( monsno,season,g_type ,zone_group,is_white,season_enable)
  $zone_group_fom_data[zone_group].each{|data|
    if data.monsno == monsno then
      if data.is_white == is_white then
        if (season == SEASON_NONE) || (data.season == SEASON_NONE) || (data.season == season) || (season_enable == false) then
          if data.type_id == g_type then
            return data.formno
          end
        end
      end
    end
  }

  puts "%s"%zone_group
  puts "検索対象:%s %d %s %d"%[$monsname[monsno],season,is_white,g_type]
  puts "季節判定%s"%season_enable
  $zone_group_fom_data[zone_group].each{|data|
    if monsno == data.monsno then
      puts "%s %d %s %d"%[$monsname[data.monsno],data.season,data.is_white,data.type_id]
    end
  }
  raise "フォルムを探せませんでした"
end

# 全国ポケモンが登録されているゾーンを調べる
def check_zenkoku_mons( mons_array ,zone_group ,file , version)
  if_fined = FALSE
  mons_array.each_key{|key|
    if $chihou_zukanno_hash[key] == $NO_CHIHOU_ID then
      if if_fined == FALSE then
        if_fined = TRUE
        file.print "\n%s(%s),"%[zone_group,version]
      end
      file.print "%s,"%[$monsname[key]]
    end
  }
end

#=================================================
# メイン処理
#=================================================

# エンカウントデータの読み込み
encount_data          = nil
form_hash             = nil
FNAME_MARSHALL        = ARGV[0].to_s
FNAME_DATA_INDEX      = ARGV[1].to_s
ARC_FILE_LIST_W_NAME  = ARGV[2].to_s
ARC_FILE_LIST_B_NAME  = ARGV[3].to_s
FORM_FNAME_MARSHALL   = ARGV[4].to_s
open(FNAME_MARSHALL,"rb"){|file|
  str =  file.read
  encount_data = Marshal.load(str)
}
open(FORM_FNAME_MARSHALL,"rb"){|file|
  str =  file.read
  form_hash = Marshal.load(str)
}

# ゾーングループごとの格納先
zone_group_data = Hash.new
$zone_group_fom_data = Hash.new

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
  #puts zone_group
  zone_group_data[zone_group].merge_season
  zone_group_data[zone_group].season_enable_check
}


# ゾーングループごとにデータをマージする(フォルム)
form_hash.each_key{|zone_id|
  # エラーチェック
  unless $zohe_id_to_zone_group.key?(zone_id.upcase) then raise "不明なゾーンID" end
  zone_group = $zohe_id_to_zone_group[zone_id.upcase]
  # キーが存在しない場合は、データ格納先を作成する
  unless $zone_group_fom_data.key?(zone_group) then
    # コピーを生成
    $zone_group_fom_data[zone_group] = Marshal.load(Marshal.dump(form_hash[zone_id]))
  else
    # 単純追加
    $zone_group_fom_data[zone_group].concat(form_hash[zone_id])
  end
}


#=begin

pi_f = open($ZONE_POKE_INFO_FILE_NAME,"w")
# マージしたデータを格納する
zone_group_data.each_key{|zone_group|
  data = zone_group_data[zone_group]

  pi_f.puts "=============================================================="
  pi_f.puts "【 %s(ホワイト) 】"%[zone_group]
  pi_f.puts " 黒有 :%s"%[data.black_enable]
  pi_f.puts " 白地形 :%s"%[$info_ground_type_name[data.white_ground_type]]
  pi_f.puts " 黒地形 :%s"%[$info_ground_type_name[data.black_ground_type]]

  # ホワイト
  GTYPE_MAX.times{|g_type|
    pi_f.puts " 四季 :%s"%[data.ground_season_w[g_type]]
    pi_f.puts "【 %s 】"%[$ground_type_name[g_type]]
    unless data.white_data[g_type] == nil then
      data.white_data[g_type].each_key{|season|
        pi_f.print "%s(%2d種類):"%[$season_name[season],data.white_data[g_type][season].size]
        if data.white_data[g_type][season].size > 20 then
          raise "20種類以上使用されています"
        end
        data.white_data[g_type][season].each{|mons_data|
          pi_f.print "%s,"%[$monsname[mons_data]]
        }
        pi_f.print "\n"
      }
    end
  }
  pi_f.print "\n"
  
  pi_f.puts "--------------------------------------------------------------"

  # ブラック
  pi_f.puts "【 %s(ブラック) 】"%[zone_group]
  GTYPE_MAX.times{|g_type|
    pi_f.puts " 四季 :%s"%[data.ground_season_b[g_type]]
    pi_f.puts "【 %s 】"%[$ground_type_name[g_type]]
    unless data.black_data[g_type] == nil then
      data.black_data[g_type].each_key{|season|
        pi_f.print "%s(%2d種類):"%[$season_name[season],data.black_data[g_type][season].size]
        data.black_data[g_type][season].each{|mons_data|
          pi_f.print "%s,"%[$monsname[mons_data]]
        }
        pi_f.print "\n"
        if data.black_data[g_type][season].size > 20 then
          raise "20種類以上使用されています"
        end
      }
    end
  }

  pi_f.print "\n"
}
pi_f.close


#=end
# 全国ポケモン登録ゾーングループ出力ファイル
zenkoku_poke_zone_file = open($ZENKOKU_POKE_ZONE_FILE_NAME,"w")

# バイナリデータの出力
zone_group_data.each_key{|zone_group|
  b_mons_array = Hash.new
  w_mons_array = Hash.new
  group_data =  zone_group_data[zone_group]
  b_mons_data = group_data.black_data
  w_mons_data = group_data.white_data

  # ブラック用データ作成
  set_mons_array_data( group_data,b_mons_array ,b_mons_data )
  check_zenkoku_mons( b_mons_array ,zone_group ,zenkoku_poke_zone_file ,"B")
  put_bin_file( "hb_b_%s.bin"%[zone_group.downcase] ,group_data ,b_mons_array,group_data.black_ground_type ,zone_group ,group_data.ground_season_b ,false)

  # ホワイト用データ作成
  set_mons_array_data( group_data,w_mons_array ,w_mons_data)
  check_zenkoku_mons( w_mons_array ,zone_group ,zenkoku_poke_zone_file,"W")
  put_bin_file( "hb_w_%s.bin"%[zone_group.downcase] ,group_data ,w_mons_array,group_data.white_ground_type ,zone_group ,group_data.ground_season_w ,true)
}

# ファイル終了
zenkoku_poke_zone_file.close

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

