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

# OK/NGのハッシュ
$ng_hash      = { "NG"=>0, "ＮＧ"=>1, "OK"=>2, "ＯＫ"=>3 }

# バトルルールのハッシュ
$battle_rule_hash = { "SINGLE"=>0, "DOUBLE"=>1, "TRIPLE"=>2, "ROTATION"=>3 }

#
# エラー文字列(全まとめ)
$error_str = [ "の指定が正しくありません",
               "が空白です",
]

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
# エラー表示まとめ
#-------------------------
def print_error( str, num )
  p sprintf("[%s] %s", str, $error_str[num])
end

#-------------------------------------------------------
#-------------------------------------------------------

