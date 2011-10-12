#!ruby -Ks
#---------------------------
# wbt_dldata_util.rb
#---------------------------
require 'kconv'

require "version.rb"

require "my_tab_out_direct_sheet.rb"

# パス要変更
if($exe_mode == 0)
  HASHPATH = "../../../../swan/tools/hash/"
else
  HASHPATH = ""
end
#
require HASHPATH + "monsno_hash.rb"	# ポケモン番号
require HASHPATH + "item_hash.rb"	# アイテム番号
require HASHPATH + "wazano_hash.rb"	# 技番号
require HASHPATH + "seikaku_hash.rb"	# 性格
require HASHPATH + "tokusei_hash.rb"	# 特性
require HASHPATH + "trtype_hash.rb"	# トレーナーID
#
require "wbt_tokusei_data.rb"           # 特性のデータ

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
# (poketype_def.hと対応)
$type_hash   = {
                 "ノーマル"=>0,
                 "かくとう"=>1,
                 "ひこう"  =>2,
                 "どく"    =>3,
                 "じめん"  =>4,
                 "いわ"    =>5,
                 "むし"    =>6,
                 "ゴースト"=>7,
                 "はがね"  =>8,
                 "ほのお"  =>9,
                 "みず"    =>10,
                 "くさ"    =>11,
                 "でんき"  =>12,
                 "エスパー"=>13,
                 "こおり"  =>14,
                 "ドラゴン"=>15,
                 "あく"    =>16,
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
# ボールID(item.hと対応)
$ball_hash = {
	"マスターボール"  =>1,
	"ハイパーボール"  =>2,
	"スーパーボール"  =>3,
	"モンスターボール"=>4,
	"サファリボール"  =>5,
	"ネットボール"    =>6,
	"ダイブボール"    =>7,
	"ネストボール"    =>8,
	"リピートボール"  =>9,
	"タイマーボール"  =>10,
        
	"ゴージャスボール"=>11,
	"プレミアボール"  =>12,
	"ダークボール"    =>13,
	"ヒールボール"    =>14,
	"クイックボール"  =>15,
	"プレシャスボール"=>16,
	"スピードボール"  =>17,
	"レベルボール"    =>18,
	"ルアーボール"    =>19,
	"ヘビーボール"    =>20,
        
	"ラブラブボール"  =>21,
	"フレンドボール"  =>22,
	"ムーンボール"    =>23,
	"コンペボール"    =>24,
	"ドリームボール"  =>25,
}

# エラー文字列(全まとめ)
$error_str = [ "の指定が正しくありません",
               "が空白です",
               "文字入力が正しくありません",
]

# 国のテーブル参照用ハッシュ
$lang_tbl_hash   = { "日"=>0, "英"=>1, "仏"=>2, "伊"=>3, "独"=>4, "西"=>5, "韓"=>6 }

# コード表示用(test用)
$codes = {
  NKF::JIS      => "JIS",
  NKF::EUC      => "EUC",
  NKF::SJIS     => "SJIS",
  NKF::BINARY   => "BINARY",
  NKF::UNKNOWN  => "UNKNOWN(ASCII)",
  NKF::ASCII    => "ASCII",
  NKF::UTF8     => "UTF8",
  NKF::UTF16    => "UTF16",
};

# 文字数制限(1行用、複数行用(改行分加算))
$str_limit = [36, 18, 36+1, 18+1]

# シート名
$sheet_str = [ "トーナメントリスト",
               "レギュレーション",
               "レギュレーション管理",
               "トレーナー管理",
               "トレーナーデータ",
               "トレーナータイプ指定",
               "手持ちデータ",
]

# セルのROW
$cell_row = ["A","B","C","D","E","F","G","H","I","J",
             "K","L","M","N","O","P","Q","R","S","T",
             "U","V","W","X","Y","Z",
]

#---------------------------------------------------------
#---------------------------------------------------------
#--------------------------------
# ハッシュ
#
# hash:     ハッシュ
# str:      調べる文字列
#
# ret:      (-1:なし、その他:ハッシュ値)
#--------------------------------
def hash2no( hash, str )
  if hash[str]==nil then
    return -1
  end
  return hash[str]
end

#-------------------------------
# 文字列のパック
# 
# buffer:  バッファ(char型の配列)
# num:     最大数
# 
# ret:     packしたデータ
#-------------------------------
def str2pack( buffer, num )
  array = Array.new	# 格納用
  #
  buffersize = buffer.size
  #
  # bufferはここに来る前に
  # char型のArrayでセットされているはず
#  buffer2 = buffer
  #
  cnt = 0
  j=0
  len = num*2
  # ループ
  len.times{|i|
    #-----------
    # 念のため
    #-----------
    if(j >= len)
      break;
    end
    #
    #------------
    # skipの処理
    #------------
    if(cnt > 0)
      cnt = cnt-1
      next;
    end
    #
    #---------------------------------
    # サイズ以上の場合=>0xffをセット
    #---------------------------------
    if(buffersize <= i)
      array << 0xff
      j = j+1
      next;
    end
    
    # 改行処理
    # (0x0a, 0x00) => (0xfe, 0xff)に変換
    if(buffer[i] == 0x0a)
      # 念のため
      if(i < len-1)
         if(buffer[i+1] == 0x00)
           array << 0xfe
           array << 0xff
           cnt = 1
           j = j+2
           next;
        end
      end
    end
    #
    # 改ページ処理
    # (0xbc, 0x25) => (0x00,0xf0,0x01,0xbe,0x00,0x00)に変換
    if(buffer[i] == 0xbc)
      # 念のため
      if(i < len-1)
         if(buffer[i+1] == 0x25)
           # 足りない場合は出力しない
           if(j <= len-8)
             array << 0x00
             array << 0xf0
             array << 0x01
             array << 0xbe
             array << 0x00
             array << 0x00
#             p sprintf("page :%d,%d\n",j,len-8)
             j = j+6
           else
             p sprintf("page error:%d,%d\n",j,len-8)
           end
           cnt = 1
           next;
        end
      end
    end
    #
    # 改ページ処理(行送り)
    # (0xbd, 0x25) => (0x00,0xf0,0x00,0xbe,0x00,0x00)に変換
    if(buffer[i] == 0xbd)
      # 念のため
      if(i < len-1)
         if(buffer[i+1] == 0x25)
           # 足りない場合は出力しない
           if(j <= len-8)
             array << 0x00
             array << 0xf0
             array << 0x00
             array << 0xbe
             array << 0x00
             array << 0x00
#             p sprintf("page :%d,%d\n",j,len-8)
             j = j+6
           else
             p sprintf("page error:%d,%d\n",j,len-8)
           end
           cnt = 1
           next;
        end
      end
    end
    array << buffer[i];
    j = j+1
#   p sprintf("utf : %x",buffer[i])
  }
  #
  # サイズのチェック
#  p sprintf("** size ** : %d,%d,(%d)",buffersize,j,num*2)
  array[num*2-2] = 0xff;
  array[num*2-1] = 0xff;
  #
  # サイズ注意
  return array.pack("C*")
end

#-------------------------
# ブランクチェック
#
# str:   check文字列
# mstr:  エラー表示文字列
#
# ret:   (0:ある、-1:なし)
#-------------------------
def chkblank( str, mstr )
  if(str == nil)
#    p sprintf("[%s] %s", mstr,$error_str[1])
    return(-1);
  end
  return(0);
end

#-------------------------
# ブランクチェック(判定のみ)
#
# str:   check文字列
#
# ret:   (0:ある、-1:なし)
#-------------------------
def chkblank_no_print( str )
  if(str == nil)
    return(-1);
  end
  return(0);
end

#-------------------------
# 数字の範囲チェック
#
# input:    入力値
# minnum:   min(含む)
# maxnum:   max(含む)
#
# ret:      (0:ある、-1:なし)
#-------------------------
def chk_num_range( input, minnum, maxnum )
  flg = 0
  if(input < minnum)
    # minエラー
    flg = -1
  elsif(input > maxnum)
    flg = -1
  end
  #
  return( flg );
end

#-------------------------
# エラー表示まとめ(数字)
#
# str:     文字列
# num:     入力値
# min:     min(含む)
# max:     max(含む)
#-------------------------
def print_error_num( str, num, min, max )
  p sprintf("[%s] %s : %d (%d - %d)", str, $error_str[0], num, min, max )
end

#-------------------------
# エラー表示まとめ(文字列)
#
# str:     文字列
# mstr:    
#-------------------------
def print_error_str( str, mstr )
  p sprintf("[%s] %s : %s", str, $error_str[0], mstr )
end

#-------------------------
# エラー表示(詳細情報)
#
# filename: ファイル名
# sheetnum: シート番号
# col:      col
# row:      row
#-------------------------
def print_error_info( filename, sheetnum, col, row )
  p sprintf("[%s] : [%s] : [ %d, %d ]", filename, $sheet_str[sheetnum], col, row )
end

#------------------------------
# 指定の箇所からの個数を求める
# (設定ファイルで使用)
#
# str:      文字列
# src:      番号(index)
# maxnum:   最大数
#
# ret:      個数
#------------------------------
def get_array_num( str, src, maxnum )
  # 個数を調べる
  i=0
  j=0;
  maxnum.times{|i|
#    p sprintf("[%d] %s", i, str[src+i])
    # 項目がなかった
    if(str[src+i] == nil)
      break;
    end
    #
    # skipだった
    num = hash2no( $skip_hash, str[src+i] );
    if(num >= 0)
#      p sprintf("skip :%s",str[src+i])
      break;
    end
    j+=1
  }
#   p sprintf("[%d,%d]", i,j)
  return(j)
end

#-------------------------------
# unicodeをセット
#
# conv:     excelアクセス用
# index:    シート番号
# col:      col
# row:      row
#
# ret:      UTF16の文字バッファ
#-------------------------------
def get_unicode( conv, index, col, row )
  # UTF8で取得
  str = conv.get_cell2(index, col, row)
  #----------------
  # UTF8 -> UTF16へ
  #----------------
  # 正しく変換されないことがあったので保留
#  str2 = NKF.nkf('-m0 -w16L0', str)
  #
  #UTF8をUTF16に変換
  str2 = str.kconv(Kconv::UTF16, Kconv::UTF8)
  #1byte単位に分割
  # u16に変換後u8単位にしているので、
  # (必ず2の倍数となるはず)
  strArr = str2.unpack("C*")
  # 
  pos = 0
  # kconvの場合は
  # 上位、下位を変更しないとだめらしい..
  while pos < strArr.size/2
    temp = strArr[pos*2]
    strArr[pos*2] = strArr[pos*2+1]
    strArr[pos*2+1] = temp
    pos = pos+1
  end
  #
  return( strArr )
end

#-------------------------
# 文字の情報をセット
#
# buffer:   文字のバッファ
#
# ret:      情報のバッファ
# (行数、それぞれの文字数)
#-------------------------
def chk_strinfo( buffer )
  array = [0,0,0,0,0]
  #
  # サイズを取得
  buffersize = buffer.size
  #
  j    = 0
  cnt  = 0
  skip = 0
  buffersize.times{|i|
    #------------
    # skipの処理
    #------------
    if(skip > 0)
      skip = skip-1
      next;
    end
    #
    # 改行処理
    # (0x0a, 0x00)
    if(buffer[i] == 0x0a)
      # 念のため
      if(i < buffersize-1)
         if(buffer[i+1] == 0x00)
           skip = 1
           array[1+cnt] = (j+2)/2
           cnt = cnt+1
           j = 0
#           p sprintf("skip 0\n")
           next
        end
      end
    end
    #
    # 改ページ処理
    # (0xbc, 0x25)
    if(buffer[i] == 0xbc)
      if(buffer[i+1] == 0x25)
        skip = 1
        j = j+6
#        p sprintf("skip 1\n")
        next
     end
    end
    #
    # 改ページ処理(行送り)
    # (0xbd, 0x25)
    if(buffer[i] == 0xbd)
      if(buffer[i+1] == 0x25)
        skip = 1
        j = j+6
#        p sprintf("skip 2\n")
        next
      end
    end
    # カウント
    j = j+1
  }
  #
  if(j != 0)
    array[1+cnt] = j/2
  end
  #
  # 行数
  array[0] = cnt+1
  #
  return( array )
end

#------------------------------------
# 文字の個数制限
#
# buffer:   バッファ
# lnum:     行数
# num:      文字数
# str:      文字列
#
# ret:      (0:正しい、-1:正しくない)
#------------------------------------
def chk_str_limit( buffer, lnum, num, str )
  # チェックの有無
  if( $debug_chkstr == 0 )
    return( 0 );
  end
  #
  flg = 0
  array = chk_strinfo( buffer )
  #
#  p sprintf("%d :%d,%d,%d,%d :\n",array[0],array[1],array[2],array[3],array[4])
  #
  # 行数の制限
  if(array[0] > lnum)
    p sprintf("%s : %s : (行数:%d > %d)",str, $error_str[2], array[0], lnum)
    flg = -1
  else
    # 文字数の制限
    3.times{|i|
      if( array[1+i] > num )
        p sprintf("%s : %s : (文字数: %d > %d)",str, $error_str[2], array[1+i], num)
        flg = -1;
        break;
      end
    }
  end
  #
  return( flg )
end

#------------------------------------
# 文字の個数制限
#
# buffer:   バッファ
# lnum:     行数
# num:      文字数
# str:      文字列
#
# ret:      array
#------------------------------------
def chk_str_limit_array( buffer, lnum, num, str )
  # チェックの有無
#  if( $debug_chkstr == 0 )
#    return(0);
#  end
  #
  flg = 0
  array = chk_strinfo( buffer )
  #
#  p sprintf("%d :%d,%d,%d,%d :\n",array[0],array[1],array[2],array[3],array[4])
  #
  # 行数の制限
  if(array[0] > lnum)
#    p sprintf("%s : %s : (行数:%d > %d)",str, $error_str[2], array[0], lnum)
    flg = -1
  else
    # 文字数の制限
    3.times{|i|
      if( array[1+i] > num )
#        p sprintf("%s : %s : (文字数: %d > %d)",str, $error_str[2], array[1+i], num)
        flg = -1;
        break;
      end
    }
  end
  #
  array[4] = flg;
  return( array );
#  return( flg )
end

#-------------------------------------------------------
#-------------------------------------------------------

