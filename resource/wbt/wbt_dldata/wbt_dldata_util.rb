#!ruby -Ks
#---------------------------
# wbt_dldata_util.rb
#---------------------------
require 'kconv'

require "my_tab_out_direct_sheet.rb"

# パス要変更
HASHPATH = "../../../../swan/tools/hash/"
#
require HASHPATH + "monsno_hash.rb"	# ポケモン番号
require HASHPATH + "item_hash.rb"	# アイテム番号
require HASHPATH + "wazano_hash.rb"	# 技番号
require HASHPATH + "seikaku_hash.rb"	# 性格
require HASHPATH + "tokusei_hash.rb"	# 特性
require HASHPATH + "trtype_hash.rb"	# トレーナーID

# OK/NGのハッシュ
$ng_hash      = { "NG"=>0, "ＮＧ"=>1, "OK"=>2, "ＯＫ"=>3 }

# ON/OFFのハッシュ
$onoff_hash   = { "OFF"=>0, "ＯＦＦ"=>1, "ON"=>2, "ＯＮ"=>3 }

# バトルルールのハッシュ
$battle_rule_hash = { "シングル"=>0, "ダブル"=>1, "トリプル"=>2, "ローテーション"=>3 }
#
# レベル範囲のハッシュ
$level_range_hash = { "なし"=>0, "以上"=>1, "以下"=>2, "以上補正"=>3,"以下補正"=>5,"全補正"=>4,  }
#
# 性別のハッシュ
$sex_hash   = { "♂"=>0, "♀"=>1, "性別なし"=>2 }
#
# ○×のハッシュ
$ox_hash   = { "×"=>0, "○"=>1 }
#
# スキップのハッシュ
$skip_hash   = { "-"=>0, "ー"=>1, "━"=>2, "─"=>3 }
#
# 必須タイプのハッシュ
$type_hash   = {
                 "ノーマル"=>0,
                 "ほのお"  =>1,
                 "みず"    =>2,
                 "でんき"  =>3,
                 "くさ"    =>4,
                 "こおり"  =>5,
                 "かくとう"=>6,
                 "どく"    =>7,
                 "じめん"  =>8,
                 "ひこう"  =>9,
                 "エスパー"=>10,
                 "むし"    =>11,
                 "いわ"    =>12,
                 "ゴースト"=>13,
                 "ドラゴン"=>14,
                 "あく"    =>15,
                 "はがね"  =>16,
                 "指定なし"=>17,
}
#
# 設定方法のハッシュ
$match_hash   = { "ランダム(戦える)"=>0, 
                  "ランダム(戦えない)"=>1,
                  "データ設定"=>2,
                  "ID指定"=>3,
}
# ppupのハッシュ
$ppup_hash   = { "PPUP×0"=>0, "PPUP×1"=>1, "PPUP×2"=>2, "PPUP×3"=>3 }
#
# 対応カセットバージョンのハッシュ
$version_hash   = { "W2がOK"=>0, "B2がOK"=>1, "両方OK"=>2 }
#
# 国のハッシュ
$lang_hash   = { "日"=>1, "英"=>2, "仏"=>3, "伊"=>4, "独"=>5, "西"=>7, "韓"=>8 }
#
# エラー文字列(全まとめ)
$error_str = [ "の指定が正しくありません",
               "が空白です",
]

# 国のテーブル参照用ハッシュ
$lang_tbl_hash   = { "日"=>0, "英"=>1, "仏"=>2, "伊"=>3, "独"=>4, "西"=>5, "韓"=>6 }

#---------------------------------------------------------
#---------------------------------------------------------
#-------------
# ハッシュ
#-------------
def hash2no( hash, str )
  if hash[str]==nil then
    return -1
  end
  return hash[str]
end

#-------------------------
# 文字列のパック(要確認)
#-------------------------
def str2pack( buffer, num )
  array = Array.new
  #
  j=0
  len = num*2
  len.times{|i|
    if(buffer[i] == nil)
      array << 0xffff;
    else
      # 改行処理
      # (0x0a, 0x00) => (0xfe, 0xff)に変換
      if(buffer[i] == 0x0a)
        # 念のため
        if(i < len-2)
           j = i+1
           if(buffer[j] == 0)
             buffer[i]   = 0xfe
             buffer[i+1] = 0xff
           end
        end
      end
      array << buffer[i];
#      p sprintf("utf : %x",buffer[i])
    end
  }
  # サイズ注意
  return array.pack("C*")
#  return array.pack("S*")
end

#-------------------------
# ブランクチェック
#
# check文字列、エクセルのcol、エラー表示文字列
#-------------------------
def chkblank( str, num, mstr )
  if(str == nil)
    p sprintf("[%s](%d) %s", mstr,num,$error_str[1])
    return(-1);
  end
  return(0);
end

#-------------------------
# ブランクチェック
#
# check文字列、エクセルのcol、エラー表示文字列
#-------------------------
def chkblank_no_print( str )
  if(str == nil)
    return(-1);
  end
  return(0);
end

#-------------------------
# エラー表示まとめ
#-------------------------
def print_error( str, num )
  p sprintf("[%s] %s", str, $error_str[num])
end

#-------------------------
# 指定の箇所からの個数を求める
#-------------------------
def get_array_num( str, src, maxnum )
  # 個数を調べる
  i=0
  j=0;
  maxnum.times{|i|
#    p sprintf("[%d] %s", i, str[src+i])
    if(str[src+i] == nil)
      break;
    end
    j+=1
  }
#   p sprintf("[%d,%d]", i,j)
  return(j)
end

#-------------------------------------------------------
#-------------------------------------------------------

