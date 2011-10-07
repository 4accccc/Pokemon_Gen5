#!ruby -Ks
#---------------------------
# wbt_tokusei_conv.rb
#
# 特性のコンバータ
#---------------------------
$KCODE = "SJIS"

require 'csv'

# 特性のファイル(入力)
CSVPATH = "../../personal/"
$csv_file = CSVPATH + "personal_sw.csv"
#
# 出力
$out_file = "wbt_tokusei_data.rb"
#
# ポケモン名のテーブル
# (念のため準備だけ)
HASHPATH = "../../../../swan/tools/hash/"
require HASHPATH + "monsno_hash.rb"	# ポケモン番号
require HASHPATH + "tokusei_hash.rb"	# 特性

# 開始位置(読み込むと(0,0)が基準となる)
PNL_START_COL  = 0
PNL_START_ROW  = 0
# 特性情報
TOKUSEI_START_ROW = 36
#
# ヘッダ
$wbt_header = [
"#!ruby -Ks",
"#------------------------------------",
"# (monsno, formno, tokusei(0,1,2), 0)",
"#------------------------------------",
"$wbt_tokusei_tbl = [",
]

# フッタ
$wbt_footer = [
"",
"\t0,0,0,0,0,0,",
#"\t\"\",\"\",\"\",\"\",",
"]",
]

# debug
$debug_flag = 0

#------------------------------------------------
#------------------------------------------------
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

#------------------
# 組み合わせ(単体)
#------------------
class WBT_TOKUSEI_CONV
  #--------
  # 初期化
  #--------
  def initialize()
    @data = CSV.readlines( $csv_file );
  end
  
  #--------------------------------
  # 必要な情報を取得
  #--------------------------------
  def put_data()
    File.open($out_file,"w"){|file|
      #--------
      # ヘッダ
      #--------
      size = $wbt_header.size
      size.times{|i|
        file.printf("%s\n",$wbt_header[i]);
      }
      
      #--------
      # 本体
      #--------
      # サイズ分
      size = @data.size
      if($debug_flag == 1)
        p sprintf("size :%d",size);
      end
      size.times{|i|
        mstr = []
        mbuf = []
        str = @data[PNL_START_COL+i][PNL_START_ROW];
#       なぜか空白のセル(キャラ情報)がある
#        if( str == "" )
#          p sprintf("null :%d",i);
#          break;
#        end
        #
        # monsnoを求める
        id = hash2no( $monsno_hash, str );
        if(id >= 0)
          # 特性を取得する
          3.times{|j|
            mstr[j] = @data[PNL_START_COL+i][TOKUSEI_START_ROW+j];
            if(mstr[j] == "")
              mbuf[j] = -1
            else
              mbuf[j] = hash2no( $tokusei_hash, mstr[j] );
            end
          }
          if($debug_flag == 1)
            p sprintf("%d : %s : %s, %s, %s",(i+1), str,mstr[0],mstr[1],mstr[2]);
          end
          # form番号をセット(上から加算)
          # =>何個あるかを調べ =>(-1)してform番号とする
          cnum = chk_poke_number( i+1, id );
          if(cnum == 0)
            p sprintf("poke num error\n");
          end
#         file.printf("\t\"%s\", \"%s\", \"%s\", \"%s\",\n", str,mstr[0],mstr[1],mstr[2]);
         file.printf("\t%3d,%3d,%3d,%3d,%3d,  0,\t# (%s, %s, %s, %s)\n",
                      id, (cnum-1), mbuf[0], mbuf[1], mbuf[2], str, mstr[0], mstr[1], mstr[2]);
        end
      }
      #--------
      # フッタ
      #--------
      size = $wbt_footer.size
      size.times{|i|
        file.printf("%s\n",$wbt_footer[i]);
      }
    }
  end
  
  #--------------------------------
  # 同じポケモンがあった場合何番目か
  #
  # 配列の番号
  # id
  #--------------------------------
  def chk_poke_number( num, monsno )
    mnum = 0
    # サイズ分
#    p sprintf("num :%d",num);
    num.times{|k|
      str = @data[PNL_START_COL+k][PNL_START_ROW];
      # 調べる
      id = hash2no( $monsno_hash, str );
#      p sprintf("str :%s,%d,%d,%d",str,k,id,monsno);
      if(id == monsno)
        mnum = mnum+1
      end
    }
    return( mnum );
  end
  #
end

#------------------------------------------------------------------
#------------------------------------------------------------------
#---------
# 開始
#---------
wbt_tokusei = WBT_TOKUSEI_CONV.new()
wbt_tokusei.put_data()
#
puts "end"

