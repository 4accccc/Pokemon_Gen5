#!ruby -Ks
#---------------------------
# pkwdconv.rb
#---------------------------
require 'csv'

# パス要変更
HASHPATH = "../../../swan/tools/hash/"
#
require HASHPATH + "monsno_hash.rb"	# ポケモン番号
require HASHPATH + "seikaku_hash.rb"	# 性格
#require HASHPATH + "tokusei_hash.rb"	# 特性
require HASHPATH + "item_hash.rb"	# 道具
require HASHPATH + "wazano_hash.rb"	# 技
#
#require "../../../swan/tools/hash/monsno_hash.rb"	# ポケモン番号
#require "../../../swan/tools/hash/seikaku_hash.rb"	# 性格
#require "../../../swan/tools/hash/tokusei_hash.rb"	# 特性
#require "../../../swan/tools/hash/item_hash.rb"	# 道具
#require "../../../swan/tools/hash/wazano_hash.rb"	# 技

#--------------
# 定義いろいろ
#--------------
# 上側
PKWD_UP_POKE_NUM	= 6		# 最大ポケモン数
PKWD_UP_START_ROW	= 1		# パラメータ開始位置
PKWD_UP_START_COL	= 4
#
# 下側
PKWD_DOWN_POKE_NUM	= 6		# 最大ポケモン数
PKWD_DOWN_START_ROW	= 1		# パラメータ開始位置
PKWD_DOWN_START_COL	= 29
#
# 共通項目
PKWD_INFO_NUM	        = 24		# パラメータ数
PKWD_SKILL_NUM		= 4		# 最大スキル
PKWD_PARAM_NUM		= 12		# 最大パラメータ
# 各項目
PKWD_REC_POKE		= 0
PKWD_PLAY_POKE		= 1
PKWD_LEVEL		= 2
PKWD_FORM		= 3
PKWD_SEX		= 4
PKWD_MIND		= 5
PKWD_STATE		= 6
PKWD_GOODS		= 7
PKWD_SKILL		= 8
PKWD_PARAM		= 12

# 性別
$sex_hash      = {"オス"=>0, "メス"=>1, "なし"=>2 }

# 上下判定用
$updown_num    = [PKWD_UP_POKE_NUM,  PKWD_DOWN_POKE_NUM ]
$updown_row    = [PKWD_UP_START_ROW, PKWD_DOWN_START_ROW]
$updown_col    = [PKWD_UP_START_COL, PKWD_DOWN_START_COL]
$updown_str    = ["上段","下段"]
#
$error_str     = ["撮影ポケモン名",
                  "上映ポケモン名",
                  "レベル",
                  "フォルム",
                  "性別",
                  "性格",
                  "特性",
                  "道具",
                  "技１",
                  "技２",
                  "技３",
                  "技４",
                  "ＨＰ固体値",
                  "ＨＰ努力値",
                  "攻撃固体値",
                  "攻撃努力値",
                  "防御固体値",
                  "防御努力値",
                  "早さ固体値",
                  "早さ努力値",
                  "特攻固体値",
                  "特攻努力値",
                  "特防固体値",
                  "特防努力値",
]

# (パラメータの)エラーコード
$error_str2 = ["固体値パラメータの合計が510を超えています",
               "努力値パラメータの合計が510を超えています",
]

# スキップのコード
$skip_hash = {"━"=>0, "-"=>1, "－"=>2, "ー"=>3, }

# debug
$debug_code = 1

#-------------
# ハッシュ
#-------------
def hash2no( hash, str )
  if hash[str]==nil then
    return -1
  end
  return hash[str]
end

#-------------
# PKWD_INFO
#-------------
class PKWD_INFO
  #--------
  # 初期化
  #--------
  def initialize( data )
    @data = data;
    #
    @namebuf 	= [0,0]
    @level	= 0
    @form	= 0
    @sex	= 0
    @mind	= 0
    @state	= 0
    @goods	= 0
    @skill	= [0,0,0,0]
    @param 	= []
    @updown	= 0        # 0:上段、1:下段
    #
    @pokenum    = [0,0]    # ポケモン数
  end
  #--------------
  # 上下セット
  #--------------
  def set_updown( num )
    @updown = num
  end
  #--------------
  # ポケモン数のチェック
  #--------------
  def set_pokenum()
    2.times{|k|
      j=0
      # ポケモン数をカウント
      # (撮影ポケモンの有無で判断)
      col = $updown_col[k]
      row = $updown_row[k]
      $updown_num[k].times{|i|
#        id = hash2no($monsno_hash, @data[col+PKWD_REC_POKE][row+1+i])
#        if(id == -1)
        if(@data[col+PKWD_REC_POKE][row+1+i] == nil)
          break
        end
        #
        j = j+1
      }
      @pokenum[k] = j
    }
    # debug
    if($debug_code == 1)
      p sprintf("pokenum : %d,%d",@pokenum[0],@pokenum[1])
    end
  end
  #--------------
  # nullチェック
  #--------------
  def chk_nullinfo()
    2.times{|k|
      # 項目に空白がないかチェック
      col = $updown_col[k]
      row = $updown_row[k]
      #
      @pokenum[k].times{|i|
        PKWD_INFO_NUM.times{|j|
          if(@data[col+j][row+1+i] == nil)
            #
            p sprintf("%s : %d体目の%sの指定が正しくありません (null):", $updown_str[k],i,$error_str[j])
            exit(1);
          end
        }
      }
    }
  end
  #--------------
  # 名前チェック
  #--------------
  def chk_name( num )
    # 番号に変換
    col = $updown_col[@updown]
    row = $updown_row[@updown]
    @namebuf[0] = hash2no($monsno_hash, @data[col+PKWD_REC_POKE][ row+1+num])
    @namebuf[1] = hash2no($monsno_hash, @data[col+PKWD_PLAY_POKE][row+1+num])
    # debug
    if($debug_code == 1)
      p sprintf("-------------")
      p sprintf("name0   : %d,%s",@namebuf[0],@data[col+PKWD_REC_POKE][ row+1+num])
      p sprintf("name1   : %d,%s",@namebuf[1],@data[col+PKWD_PLAY_POKE][ row+1+num])
    end
    # チェック
    if(@namebuf[0] == -1)
      p sprintf("%s : %d体目の%sの指定が正しくありません : %s",
                 $updown_str[@updown],num,$error_str[PKWD_REC_POKE],@data[col+PKWD_REC_POKE][ row+1+num])
      exit(1)
    end
    if(@namebuf[1] == -1)
      p sprintf("%s : %d体目の%sの指定が正しくありません : %s",
                 $updown_str[@updown],num,$error_str[PKWD_PLAY_POKE],@data[col+PKWD_PLAY_POKE][ row+1+num])
      exit(1)
    end
  end
  #----------------
  # レベルチェック
  #----------------
  def chk_level( num )
    col = $updown_col[@updown]
    row = $updown_row[@updown]
    @level = @data[col+PKWD_LEVEL][row+1+num].to_i
    # debug
    if($debug_code == 1)
      p sprintf("level   : %d",@level)
    end
    # チェック
    if((@level < 0)||(@level >= 100))
      p sprintf("%s : %d体目の%sの指定が正しくありません : %d", $updown_str[@updown],num,$error_str[PKWD_LEVEL],@level)
      exit(1)
    end
  end
  #----------------
  # フォルムチェック
  #----------------
  def chk_form( num )
    col = $updown_col[@updown]
    row = $updown_row[@updown]
    @form = @data[col+PKWD_FORM][row+1+num].to_i
    # チェック
    # 最大のフォルムよりも大きいとエラー
    formmax = hash2no($formmax_hash, @data[col+PKWD_REC_POKE][ row+1+num])
    # debug
    if($debug_code == 1)
      p sprintf("form    : %d",@form)
      p sprintf("formmax : %d",formmax)
    end
    if( @form > formmax )
      p sprintf("%s : %d体目の%sの指定が正しくありません : (%d > %d)", 
                 $updown_str[@updown],num,$error_str[PKWD_FORM],@form,formmax)
      exit(1)
    end
  end
  #----------------
  # 性別チェック
  #----------------
  def chk_sex( num )
    col = $updown_col[@updown]
    row = $updown_row[@updown]
    @sex = hash2no($sex_hash, @data[col+PKWD_SEX][row+1+num])
    # debug
    if($debug_code == 1)
      p sprintf("sex     : %d,%s",@sex,@data[col+PKWD_SEX][row+1+num])
    end
    if((@sex < 0)||(@sex > 2))
      p sprintf("%s : %d体目の%sの指定が正しくありません : %s",
                 $updown_str[@updown],num,$error_str[PKWD_SEX],@data[col+PKWD_SEX][row+1+num])
      exit(1)
    end
  end
  #----------------
  # 性格チェック
  #----------------
  def chk_mind( num )
    col = $updown_col[@updown]
    row = $updown_row[@updown]
    @mind = hash2no($seikaku_hash, @data[col+PKWD_MIND][row+1+num])
    # debug
    if($debug_code == 1)
      p sprintf("mind    : %d,%s",@mind,@data[col+PKWD_MIND][row+1+num])
    end
    if(@mind == -1)
      p sprintf("%s : %d体目の%sの指定が正しくありません : %s",
                 $updown_str[@updown],num,$error_str[PKWD_MIND],@data[col+PKWD_MIND][row+1+num])
      exit(1)
    end
  end
  #----------------
  # 特性チェック
  #----------------
  def chk_state( num )
    col = $updown_col[@updown]
    row = $updown_row[@updown]
#    @state = hash2no($tokusei_hash, @data[col+PKWD_STATE][row+1+num])
    @state = @data[col+PKWD_STATE][row+1+num].to_i
    # debug
    if($debug_code == 1)
      p sprintf("state   : %d,%s",@state,@data[col+PKWD_STATE][row+1+num])
    end
    #
    if(@state == -1 || @state>=2)
      p sprintf("%s : %d体目の%sの指定が正しくありません : %d",
                 $updown_str[@updown],num,$error_str[PKWD_STATE],@data[col+PKWD_STATE][row+1+num])
      exit(1)
    end
  end
  #----------------
  # 道具チェック
  #----------------
  def chk_goods( num )
    col = $updown_col[@updown]
    row = $updown_row[@updown]
    # スキップの場合
    if(hash2no($skip_hash, @data[col+PKWD_GOODS][row+1+num]) >= 0)
      # 項目なしは[0]としておく
      @goods = 0
    else
      @goods = hash2no($item_hash, @data[col+PKWD_GOODS][row+1+num])
    end
    # debug
    if($debug_code == 1)
      p sprintf("goods   : %d,%s",@goods,@data[col+PKWD_GOODS][row+1+num])
    end
    #
    if(@goods == -1)
      p sprintf("%s : %d体目の%sの指定が正しくありません : %s",
                 $updown_str[@updown],num,$error_str[PKWD_GOODS],@data[col+PKWD_GOODS][row+1+num])
      exit(1)
    end
  end
  #----------------
  # 技チェック
  #----------------
  def chk_skill( num )
    flg  = 0
    eflg = 0		# エラー
    col = $updown_col[@updown]
    row = $updown_row[@updown]
    PKWD_SKILL_NUM.times{|i|
      if(i != 0) then
        # スキップの場合
        if(hash2no($skip_hash, @data[col+PKWD_SKILL+i][row+1+num]) >= 0)
          # 項目なしは[0]としておく
          @skill[i] = 0
          flg = 1
        else
          @skill[i] = hash2no($wazano_hash, @data[col+PKWD_SKILL+i][row+1+num])
          # すでに項目はない=>穴あき状態
          if(flg == 1)
            eflg = 1
          end
        end
      else
        @skill[i] = hash2no($wazano_hash, @data[col+PKWD_SKILL+i][row+1+num])
      end
      # debug
      if($debug_code == 1)
        p sprintf("skill   : %d,%d,%s",i,@skill[i],@data[col+PKWD_SKILL+i][row+1+num])
      end
      # チェック
      if(@skill[i] == -1)
        eflg = 1
      end
      #
      if(eflg == 1)
        p sprintf("%s : %d体目の%sの指定が正しくありません : %s",
                 $updown_str[@updown],num,$error_str[PKWD_SKILL+i],@data[col+PKWD_SKILL+i][row+1+num])
        exit(1)
      end
    }
  end
  #----------------
  # パラメータチェック
  #----------------
  def chk_param( num )
    # 合計値計算(0:固体値、1:努力値)
    rate = [0,0]
    col = $updown_col[@updown]
    row = $updown_row[@updown]
    PKWD_PARAM_NUM.times{|i|
      @param[i] = @data[col+PKWD_PARAM+i][row+1+num];
      if(i%2 == 0)
        rate[0] = rate[0] + @param[i].to_i
      else
        rate[1] = rate[1] + @param[i].to_i
      end
      # debug
      if($debug_code == 1)
        p sprintf("param   : %d,%d",i,@param[i])
      end
      # 個別のチェック
      if((@param[i].to_i < 0)||(@param[i].to_i > 255))
        p sprintf("%s : %d体目の%sの指定が正しくありません : %s",
                 $updown_str[@updown],num,$error_str[PKWD_PARAM+i],@data[col+PKWD_PARAM+i][row+1+num])
      end
    }
    # 合計値のチェック
    if($debug_code == 1)
      p sprintf("total   : %d,%d",rate[0],rate[1])
    end
    #
    if(rate[0] > 510)
      p sprintf("%s : %d体目の%s : %d", $updown_str[@updown],num,$error_str2[0],rate[0])
      exit(1)
    end
    if(rate[1] > 510)
      p sprintf("%s : %d体目の%s : %d", $updown_str[@updown],num,$error_str2[1],rate[1])
      exit(1)
    end
  end
  
  #----------------
  # パラメータまとめ
  #----------------
  def set_poke_param( num )
    # 名前(撮影、上映)
    chk_name( num )
    # レベル
    chk_level( num )
    # フォルム
    chk_form( num )
    # 性別
    chk_sex( num )
    # 性格
    chk_mind( num )
    # 特性
    chk_state( num )
    # 道具
    chk_goods( num )
    # 技
    chk_skill( num )
    # パラメータ
    chk_param( num )
  end
  
  #----------------
  # データのパック
  #----------------
  def pack()
    array = Array.new
    
    # ポケモン数を調べる
    set_pokenum()
    # nullチェック
    chk_nullinfo();
    
    # ヘッダの情報(個数を入れておく)
    array << @pokenum[0]
    array << @pokenum[1]
    
    # 上下を処理する
    2.times{|k|
      # 対象の切り替え
      set_updown( k )
      # それぞれの個数分調べる
      @pokenum[@updown].times{|j|
        # まとめ
        set_poke_param( j )
        #
        # パラメータの処理
        array << @namebuf[0].to_i
        array << @namebuf[1].to_i
        array << @level.to_i
        array << @form.to_i
        array << @sex.to_i
        array << @mind.to_i
        array << @state.to_i
        array << @goods.to_i
        PKWD_SKILL_NUM.times{|m|
          array << @skill[m].to_i
        }
        PKWD_PARAM_NUM.times{|m|
          array << @param[m].to_i
        }
      }
    }
    
#    p array
    return array.pack("S*")
  end
  
end

#-------------------------------------------------------
#-------------------------------------------------------
#-----------------
# 開始
#-----------------
datas = CSV.readlines( ARGV[0] )
#p datas

#----------------
# 上側情報の処理
#----------------
# 情報の参照
pkwd_info = PKWD_INFO.new( datas )

#バイナリ出力
File.open( ARGV[1],"w"){|file|
#File.open( "01.bin","w"){|file|	# debug
  file.binmode
  file.write( pkwd_info.pack )
}

#---------------
# 終了
#---------------
#p sprintf("end : %s -> %s",ARGV[0], ARGV[1])

#-------------------------------------------------------
#-------------------------------------------------------

