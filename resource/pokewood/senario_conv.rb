#!ruby -Ks
#----------------------------------------------------------------------------
# ポケウッドシナリオコンバーター
# 2011/03 by mori
# 
# 入力：ruby senario_conv.rb <入力テキスト>
# 出力：<入力テキスト>の拡張子を「.bin」にしたものを出力します
#----------------------------------------------------------------------------
require 'csv'
require 'kconv'
require "jcode"
require "../../tools/hash/monsno_hash.rb"
require "../../tools/hash/wazano_hash.rb"
require "../trtype/trtype_hash.rb"
require "bgm_hash.rb"

$KECODE = 'SJIS'

ENEMY_POKE_NUM = 6       # 敵トレーナーのポケモン保持数(仮）
SELECT_MSG     = 9       # 技選択時のメッセージの種類数（自分・敵）
SITUATION_MSG  = 6       # シチュエーション別メッセージの種類数（自分・敵）
TURN_MSG       = 20      # ターンごとメッセージ数（自分・敵）
EVENT_MAX      = 12      # イベント最大数
EVENT_TALK_MAX = 10      # イベント中のメッセージの最大数(自分・敵）
EVENT_REACTION_MAX   = 4 # イベント中の選択肢データ最大数
EVENT_REACTION_ARRAY = 6 # 選択肢データ内の項目数（自分会話・相手会話・技指定・背景チェンジ）
BG_CHANGE_DATA_NUM   = 6 # ターン指定のBGチェンジのデータ個数（ターン・チェンジ指定の順に入る。実個数は÷２の値）

EVENT_CLEARTURN_KEYWORD = "クリア"
EVENT_LOSETURN_KEYWORD  = "失敗"
EVENT_NO_PROVISION      = "なし"

EVENT_LINE_HEIGHT = 18   # イベントデータの行数
EVENT1_START_LINE = 69	 # イベントデータスタート位置

EVENT_REACTION_START_OFFSET = 13

PWS_A_POKE    = 0  # AAお盆の上にポケモン
PWS_A_TRAINER = 1  # AAお盆の上にトレーナー
PWS_A_NONE    = 2  # AAお盆の上に何もない

PWS_B_POKE    = (PWS_A_POKE<<4 )     # BBお盆の上にポケモン
PWS_B_TRAINER = (PWS_A_TRAINER<<4 )  # BBお盆の上にトレーナー
PWS_B_NONE    = (PWS_A_NONE<<4 )     # BBお盆の上に何もない

PWS_BOTH_POKE           =(PWS_B_POKE|PWS_A_POKE)       # 0x00 両方ポケモン
PWS_A_POKEMON_B_TRAINER =(PWS_B_TRAINER|PWS_A_POKE)    # 0x10 敵がトレーナー自分はポケモン
PWS_A_TRAINER_B_TRAINER =(PWS_B_TRAINER|PWS_A_TRAINER) # 0x11 両方トレーナー
PWS_A_TRAINER_B_POKEMON =(PWS_B_POKE|PWS_A_TRAINER)    # 0x01 敵がポケモン自分がトレーナー
PWS_A_TRAINER_B_NONE    =(PWS_B_NONE|PWS_A_TRAINER)    # 0x21 敵側いない自分側トレーナー
PWS_A_NONE_B_TRAINER    =(PWS_B_TRAINER|PWS_A_NONE)    # 0x12 自分側いない敵側トレーナー
PWS_A_NONE_B_NONE       =(PWS_B_NONE|PWS_A_NONE)       # 0x22 両方いない
PWS_A_NONE_B_POKEMON    =(PWS_B_POKE|PWS_A_NONE)       # 0x20 敵がポケモン自分はいない
PWS_A_POKEMON_B_NONE    =(PWS_B_NONE|PWS_A_POKE)       # 0x02 敵はいない自分側はポケモン


$bg_hash       = {"チェンジ０"=>0, "チェンジ１"=>1, "チェンジ２"=>2, "チェンジ３"=>3,"チェンジ４"=>4,}
$waza_ret_hash = { "技１"=>0, "技２"=>1, "技３"=>2, "技４"=>3 }
$itemok_hash   = {"使用可"=>0, "使用不可"=>1}
$sex_hash      = {"♂"=>0, "♀"=>1, "なし"=>2 }
$stage_hash    = {"両方役者"=>PWS_A_TRAINER_B_TRAINER, 
                  "両方ポケ"=>PWS_BOTH_POKE,
                  "敵役者"=>PWS_A_POKEMON_B_TRAINER,
                  "自分役者"=>PWS_A_TRAINER_B_POKEMON,
                  "敵退出"=>PWS_A_TRAINER_B_NONE,
                  "自分退出"=>PWS_A_NONE_B_TRAINER,
                  "無人"=>PWS_A_NONE_B_NONE,
                  "敵ポケ自分退出"=>PWS_A_NONE_B_POKEMON,
                  "敵退出自分ポケ"=>PWS_A_POKEMON_B_NONE,
                  }
$gmmtag_hash   = {"%MYPOKE"=>"[1:02:ポケモンニックネーム:@]",
                  "%MYNAME"=>"[1:00:トレーナー名:@]"}
$success_event = {"イベント１"=>0, "イベント２"=>1,"イベント３"=>2,"イベント４"=>3,"イベント５"=>4,
                  "イベント６"=>5,"イベント７"=>6,"イベント８"=>7,"イベント９"=>8,"イベント１０"=>9,
                  "イベント１１"=>10,"イベント１２"=>11,"イベント１３"=>12,"イベント１４"=>13,"イベント１５"=>14,
                 }
$winprov_hash  = {"毎ターン倒す"=>0, "ターン内に相手を全滅"=>1,"特定の技を使用して全滅"=>2,
                  "特定の相手を倒す１"=>3,"特定の相手を倒す２"=>4, "特定の相手以外を倒す"=>5,
                  "生き残る１"=>6, "生き残る２"=>7, "自滅する"=>8 }

$category_hash = { "アクション"=>0, "SF"=>1, "ヒューマン"=>2, "ホラー＆サスペンス"=>3 }

$director_hash = { "OLDMAN1"=>0, "SUNGLASSES"=>1, "MIDDLEMAN2"=>2 ,"GENTLEMAN"=>3, "ADULTW"=>4 }

$bias_hash     = { "傾向：感動"=>0, "傾向：恐怖"=>1, "傾向：楽しい"=>2, "傾向：痛快"=>3, "傾向：シリアス"=>4 }

$field_hash    = {
  "ビル屋上Ａ"=>21, "ビル屋上Ａー１"=>((0<<8)|21), "ビル屋上Ａー２"=>((1<<8)|21),"ビル屋上Ａー３"=>((2<<8)|21),"ビル屋上Ａー４"=>((3<<8)|21),"ビル屋上Ａー５"=>((4<<8)|21),
  "遊園地Ａ"=>22, "遊園地Ａー１"=>((0<<8)|22),"遊園地Ａー２"=>((1<<8)|22),"遊園地Ａー３"=>((2<<8)|22),"遊園地Ａー４"=>((3<<8)|22),"遊園地Ａー５"=>((4<<8)|22),
  "都会・雑踏"=>23, "都会・雑踏ー１"=>((0<<8)|23), "都会・雑踏ー２"=>((1<<8)|23), "都会・雑踏ー３"=>((2<<8)|23), "都会・雑踏ー４"=>((3<<8)|23), "都会・雑踏ー５"=>((4<<8)|23), 
  "未来都市"=>24, "未来都市ー１"=>((0<<8)|24), "未来都市ー２"=>((1<<8)|24), 
  "砂浜"=>25,"砂浜ー１"=>((0<<8)|25),"砂浜ー２"=>((1<<8)|25),"砂浜ー３"=>((2<<8)|25),"砂浜ー４"=>((3<<8)|25),
  "波止場"=>26, "波止場ー１"=>((0<<8)|26), "波止場ー２"=>((1<<8)|26), "波止場ー３"=>((2<<8)|26), "波止場ー４"=>((3<<8)|26), 
  "ビル屋上Ｂ"=>27, 
  "魔法の世界"=>28, "魔法の世界ー１"=>((0<<8)|28),  "魔法の世界ー２"=>((1<<8)|28),  "魔法の世界ー３"=>((2<<8)|28),  "魔法の世界ー４"=>((3<<8)|28), 
  "墓地"=>29, "墓地ー１"=>((0<<8)|29),  "墓地ー２"=>((1<<8)|29),  "墓地ー３"=>((2<<8)|29),  "墓地ー４"=>((3<<8)|29),  "墓地ー５"=>((4<<8)|29), 
  "研究施設"=>30, "研究施設ー１"=>((0<<8)|30),"研究施設ー２"=>((1<<8)|30),"研究施設ー３"=>((2<<8)|30),"研究施設ー４"=>((3<<8)|30),"研究施設ー５"=>((4<<8)|30),
  "一般家屋室内"=>31, "一般家屋室内ー１"=>((0<<8)|31), "一般家屋室内ー２"=>((1<<8)|31), "一般家屋室内ー３"=>((2<<8)|31), "一般家屋室内ー４"=>((3<<8)|31), "一般家屋室内ー５"=>((4<<8)|31), 
  "湖畔"=>32, 
  "宇宙"=>33, "宇宙ー１"=>((0<<8)|33), "宇宙ー２"=>((1<<8)|33), "宇宙ー３"=>((2<<8)|33), 
  "遊園地Ｂ"=>27, "遊園地Ｂー１"=>((0<<8)|27), "遊園地Ｂー２"=>((1<<8)|27), "遊園地Ｂー３"=>((2<<8)|27), "遊園地Ｂー４"=>((3<<8)|27), 
  "都会・雑踏Ｂ"=>31, "都会・雑踏Ｂー１"=>((0<<8)|32), "都会・雑踏Ｂー２"=>((1<<8)|32), "都会・雑踏Ｂー３"=>((2<<8)|32), 
}

$situ_hash     = { "指定ポケモン瀕死→"=>0, "指定の技を使用→"=>1,
                    "ターン超過"=>2,
                    "自分が全滅"=>3,
                    "相手を１匹倒した"=>4,
                    "相手を全滅させた"=>5,
                    "指定外ポケモンが瀕死"=>6,
                    "指定のポケモンが瀕死"=>7,
                    "敵を倒せないターンが存在"=>8,
                    "指定ポケモン瀕死だが指定の技未使用"=>9,
                  }

#----------------------------------------------------
# ポケモンデータクラス
#----------------------------------------------------
class POKEWOOD_ENEMY_POKE
  def initialize( pokeno, sex, level )
    @pokeno = pokeno
    @sex    = $sex_hash[sex]
    @level  = level
  end
  attr_accessor :pokeno, :sex, :level
  
  # バイナリデータ用に配列に連結
  # s16で[4]=8
  def pack
    array = Array.new
    array << @pokeno
    array << @sex
    array << @level
    array << 0
    return array
#    return array.pack("S!4")
  end
  
end

#----------------------------------------------------
# イベントデータクラス
#----------------------------------------------------
class EVENT_DATA
  def initialize( type )
    @start_turn     = -1
    @flag_provision = -1
    @situ_provision = -1
    @situ_provision_data = -1
    @bias           = -1
    @event_point    = -1
    @talk_msg       = Array.new( EVENT_TALK_MAX*2 ).fill(-1)
    @reaction_data  = Array.new( EVENT_REACTION_MAX ).fill(-1)
    @stage_cast     = Array.new(EVENT_TALK_MAX).fill(-1)
    EVENT_REACTION_MAX.times{|i|
      @reaction_data[i] = Array.new( EVENT_REACTION_ARRAY ).fill(-1)
    }
  end
  attr_accessor :start_turn, :talk_msg, :reaction_data, :stage_cast, :flag_provision, :bias, :event_point, :situ_provision, :situ_provision_data

  #イベント時メッセージ取得
  def set_event_msg( index, msg )
    @talk_msg[index] = msg2id( msg )
  end
  
  #選択肢データとその反応データの格納
  def set_reaction_data( index, mymsg, e_msg, reaction, vfx, point )
    @reaction_data[index][0] = msg2id( mymsg )
    @reaction_data[index][1] = msg2id( e_msg )
    @reaction_data[index][2] = hash2no( $waza_ret_hash, reaction)
    @reaction_data[index][3] = hash2no( $field_hash, vfx )
    @reaction_data[index][4] = point.to_i
    @reaction_data[index][5] = -1
  end

  #イベント時メッセージ舞台設定データ
  def set_stage_cast_data( index, dat )
    @stage_cast[index] = dat;
  end

  #バイナリデータ用に配列に連結
  # s16(6+14+7+16) = 78
  def pack
    array = Array.new
    array << @start_turn
    array << @flag_provision
    array << @situ_provision
    array << @situ_provision_data
    array << @bias
    array << event_point
    array = array+@talk_msg+@stage_cast
    EVENT_REACTION_MAX.times{|i|
      array = array+@reaction_data[i]
    }
    return array
  end
end

#----------------------------------------------------
# ポケウッドシナリオデータクラス
#----------------------------------------------------
class POKEWOOD_SENARIO
  
  # 初期化:シナリオ番号、レギュレーション、ポケモン制限数、道具使用か？
  def initialize( senario, category, difficult, poke_limit, item_ok, turn_max, best_turn, director, field )
    @senario    = senario.to_i
    @category   = $category_hash[category]
    @difficult  = difficult.to_i
    @poke_limit = poke_limit.to_i
    @item_ok    = $itemok_hash[item_ok]
    @turn_max   = turn_max.to_i
    @best_turn  = best_turn.to_i
    @director   = hash2no( $director_hash, director )
    @field      = hash2no( $field_hash, field )
    @bgm        = $bgm_hash["SEQ_BGM_VS_NORAPOKE"]
    @win_bgm    = $bgm_hash["SEQ_BGM_WIN1"]
    @lose_bgm    = $bgm_hash["SEQ_BGM_PW_SCENARIO_01_F"]
    @strange_bgm    = $bgm_hash["SEQ_BGM_PW_SCENARIO_07_F"]
    @trainer_rank  = 0

    @enemy_poke = Array.new(ENEMY_POKE_NUM)
    ENEMY_POKE_NUM.times{|i|
      @enemy_poke[i] = POKEWOOD_ENEMY_POKE.new( -1, 0, 50 )
    }
    @select_msg    = Array.new(SELECT_MSG*2)
    @situation_msg = Array.new(SITUATION_MSG*2)
    @turn_msg      = Array.new(TURN_MSG*2)
    @event         = Array.new(EVENT_MAX)
    EVENT_MAX.times{|i|
      @event[i] = EVENT_DATA.new( 0 )
    }
  end
  attr_accessor :senario, :poke_limit, :item_ok, :select_msg, :situation_msg, :turn_msg, :event, 
                :turn_max, :enemy_poke, :bgm, :win_bgm, :lose_bgm, :strange_bgm, :trainer_rank

  # 初期情報セット
  def top_info( trainer_id, success_flag )
    @trainer_id   = trainer_id.to_i
    @success_flag = hash2no( $success_event, success_flag )
    @bg_change    = [-1,-1, -1,-1, -1,-1]
  end
  attr_accessor :trainer_id, :bg_change, :success_flag

  def set_bgm( bgm, win, lose, strange )
    @bgm         = hash2no_error($bgm_hash, bgm, "ＢＧＭ" )
    @win_bgm     = hash2no_error($bgm_hash, win, "ＢＧＭ" )
    @lose_bgm    = hash2no_error($bgm_hash,lose, "ＢＧＭ")
    @strange_bgm = hash2no_error($bgm_hash,strange, "ＢＧＭ")
  end

  # 勝利条件情報セット
  def set_win_provision( win_prov, rec_poke, show_poke, waza )
    @win_prov       = hash2no( $winprov_hash, win_prov )
    @rec_poke_prov  = hash2no( $monsno_hash, rec_poke  )
    @show_poke_prov = hash2no( $monsno_hash, show_poke )
    @waza_prov      = hash2no( $wazano_hash, waza )
    
    if @win_prov==-1 then
      printf("D2の勝利条件入力は必須です!\n")
      exit(1)
    end
  end
  attr_accessor :win_prov, :rec_poke, :show_poke, :waza

  def error_trainer( type )
    printf("「%s」の敵トレーナータイプが取得できません\nD5～G5の敵トレーナータイプ指定は必須です\nresource/trtype/trtype_def.hを参照してください\n",type)
  end

  # トレーナー情報セット
  def set_trainer_info( rec_m, show_m, rec_f, show_f )
    @rec_trtype_m  = hash2no( $trtype_hash, rec_m )
    @show_trtype_m = hash2no( $trtype_hash, show_m )
    @rec_trtype_f  = hash2no( $trtype_hash, rec_f )
    @show_trtype_f = hash2no( $trtype_hash, show_f )
    if @rec_trtype_m==-1 then
      error_trainer(rec_m)
      exit(1)
    end
    if @show_trtype_m==-1 then
      error_trainer(show_m)
      exit(1)
    end
    if @rec_trtype_f==-1 then
      error_trainer(rec_f)
      exit(1)
    end
    if @show_trtype_f==-1 then
      error_trainer(show_f)
      exit(1)
    end
  end
  attr_accessor :rec_trtype_m, :show_trtyp_m, :rec_trtype_f, :show_trtyp_f

  # 自分の姿情報
  def set_mytype_info( rec, show )
    @rec_mytype  = hash2no( $trtype_hash, rec )
    @show_mytype = hash2no( $trtype_hash, show )
    if @rec_trtype==-1 || @show_trtype==-1 then
      printf("「%s」,「%s」の自分トレーナータイプが取得できません\nD7,E7の自分トレーナータイプ指定は必須です\nresource/trtype/trtype_def.hを参照してください\n",rec, show)
      exit(1)
    end
  end
  
  # BG切り替えターン指定
  def set_bg_change( index, turn, change )
    @bg_change[index*2]   = turn.to_i
    @bg_change[index*2+1] = hash2no( $field_hash, change)
  end
  attr_accessor :rec_mytype, :show_mytyp
  
  # 敵トレーナーポケモン情報セット
  def set_enemy_poke( index, pokeno, sex, level )
    @enemy_poke[index].pokeno = pokeno.to_i
    @enemy_poke[index].sex    = hash2no($sex_hash, sex)
    @enemy_poke[index].level  = level.to_i
  end

  def set_select_msg( index, msg )
    @select_msg[index] = msg2id( msg )
  end
  
  def set_situation_msg( index, msg )
    @situation_msg[index] = msg2id( msg )
  end
  
  def set_turn_msg( index, msg )
    @turn_msg[index] = msg2id( msg )
  end
  
  #イベントデータセット
  def set_event_data( data, index, start, gmm )

    #発動ターンの取得か「クリア」「失敗」キーワードの取得
    if data[start][3]==EVENT_CLEARTURN_KEYWORD then
      @event[index].start_turn = 100
    elsif data[start][3]==EVENT_LOSETURN_KEYWORD then
      @event[index].start_turn = 101
    else
      @event[index].start_turn = data[start][3].to_i
    end
    
    #イベント傾向・エンディングポイントの取得
    @event[index].bias        = hash2no( $bias_hash, data[start][1])
    @event[index].event_point = data[start][9].to_i
    
    #イベント発動条件取得
    if data[start][5]!=EVENT_NO_PROVISION then
      @event[index].flag_provision = data[start][5].to_i
    end

    #イベント条件取得２
    if data[start][6].to_s!="" then
      @event[index].situ_provision      = hash2no( $situ_hash, data[start][6] )
      if @event[index].situ_provision==0 then
        @event[index].situ_provision_data = hash2no( $monsno_hash, data[start][7] )
      elsif @event[index].situ_provision==1 then
        @event[index].situ_provision_data = hash2no( $wazano_hash, data[start][7] )
      end
      if (@event[index].situ_provision==0 && @event[index].situ_provision_data==-1) ||
         (@event[index].situ_provision==1 && @event[index].situ_provision_data==-1) then
        printf( "イベント%dの条件指定に正しいポケモン名か技名が入っていません 条件=%s, 指定=%s\n", index, data[start][6], data[start][7] )
        exit(1)
      end
    end
    
    #イベント会話
    EVENT_TALK_MAX.times{|i|
      msg1 = gmm.write_each( "pw_event#{index}_hero_#{i}", data[start+i+2][1], data[start+i+2][2], data[start+i+2][3], data[start+i+2][4], start+i+2 )
      msg2 = gmm.write_each( "pw_event#{index}_enemy_#{i}", data[start+i+2][5], data[start+i+2][6], data[start+i+2][7], data[start+i+2][8], start+i+2 )
      @event[index].set_event_msg(               i, msg1 )
      @event[index].set_event_msg(i+EVENT_TALK_MAX, msg2 )
      @event[index].set_stage_cast_data(i, hash2no($stage_hash, data[start+i+2][11]))

    }
    
    #選択肢データのスタート位置
    r_start = start + EVENT_REACTION_START_OFFSET

    #選択肢データ取り込み
    EVENT_REACTION_MAX.times{|i|
      msg1 = gmm.write_each( "pw_event#{index}_react_hero_#{i}",  data[r_start+i][1], data[r_start+i][2], data[r_start+i][3], data[r_start+i][4], r_start+i )
      msg2 = gmm.write_each( "pw_event#{index}_react_enemy_#{i}", data[r_start+i][5], data[r_start+i][6], data[r_start+i][7], data[r_start+i][8], r_start+i )
      @event[index].set_reaction_data( i, msg1, msg2,
                                       data[r_start+i][9], data[r_start+i][10], data[r_start+i][11] )
    }
#    printf("%s\n", @start_turn)
  end

  # バイナリデータ用に配列に連結してエンコード
  # s16(2*12 + 2*4*3 + 9*2*2 + 6*2*2 + 20*2*2 + 78*5) = 500
  def pack
    array = Array.new
    array << @senario
    array << @category
    array << @difficult
    array << @poke_limit
    array << @director
    array << @turn_max
    array << @best_turn
    array << @success_flag
    array << @win_prov
    array << @rec_poke_prov
    array << @show_poke_prov
    array << @waza_prov
    array << @rec_trtype_m
    array << @show_trtype_m
    array << @rec_trtype_f
    array << @show_trtype_f
    array << @rec_mytype
    array << @show_mytype
    array << @field
    array << @bgm
    array << @win_bgm
    array << @lose_bgm
    array << @strange_bgm
    array << @trainer_rank
    
    BG_CHANGE_DATA_NUM.times{|i|
      array << @bg_change[i]
    }
    
    ENEMY_POKE_NUM.times{|i|
      array = array+@enemy_poke[i].pack
    }
    array = array + @select_msg
    array = array + @situation_msg
    array = array + @turn_msg
    EVENT_MAX.times{|i|
      array = array + @event[i].pack
    }
    p array
    p array.length
    return array.pack("S!"+array.length.to_s)
  end

end

# メッセージが存在するかどうか
def msg2id( msg )
  if msg==-1 then
    return -1
  else 
    return msg
  end
end

# ハッシュからキーが見つからなかった時は-1を返す
def hash2no( hash, str )
  if hash[str]==nil then
    return -1
  end
  return hash[str]
end

def hash2no_error( hash, str, type_str )
  if hash[str]==nil then
    printf("%sという%s定義は存在しません\n", str, type_str)
    exit(1)
  end
  return hash[str]
end

#----------------------------------------------------
# 文字列チェッククラス
#----------------------------------------------------
class STR_CHECKER
  def initialize
    @buffer = String.new
    file = open("font_table.lst", "r" )
    @buffer << file.read
    file.close
  end

  def checkFont( str )
    match_word = @buffer.split(//)
    check_word = str.split(//)

    check_word.each{ |c|
      ok = 0
      match_word.each{ |m|

        if c == m then
          ok = 1
          break
        end
      }
      if ok == 0 then
        puts "#{ARGV[0]}の『#{str}』の中の『#{c}』の文字は使えません！\n"
        return 0
      end
    }
    return 1
  end

  def checkLength( str )
    sankaku_num = 0
    str = str.gsub(/%MYNAME0/,"主人公００")
    str = str.gsub(/%MYNAME1/,"主人公０１")
    str = str.gsub(/%MYPOKE0/,"ポケモン０")
    str = str.gsub(/%MYPOKE1/,"ポケモン１")
    num = str.split(//).length
    temp = str.split(//)
    temp.each{|word|
      if (word=="▼" || word=="▽") then
        sankaku_num+=2
      end
    }
    if (num+sankaku_num) > 249 then
      puts "#{ARGV[0]}の『#{str}』の文字が249を超えて『#{num+sankaku_num}』文字です（#{num},#{sankaku_num}\n"
      return 0
    end
    return 1
  end
end

#----------------------------------------------------
# GMM作成クラス
#   GMMファイルはUTF-8でエンコード
#----------------------------------------------------
class MAKE_GMM
  def initialize
    @str_check    = STR_CHECKER.new
    @buffer       = String.new
    header = open("header.file","r:UTF-8")
    @buffer << header.read
    header.close
    @count = 0;
  end
  
  def write( tag, str_kana, str_kanji )
    if str_kana == "" || str_kana == nil then
      return -1
    end

    if @str_check.checkFont( str_kanji ) == 0 then
      exit(1)
    end

    if @str_check.checkLength( str_kana ) == 0 then
      exit(1)
    end
    if @str_check.checkLength( str_kanji ) == 0 then
      exit(1)
    end

    str_kana = tag_translate( str_kana )
    str_kanji = tag_translate( str_kanji )

    @buffer << Kconv.toutf8("\t<row id=\"#{tag}\">\n")
    @buffer << Kconv.toutf8("\t\t<language name=\"JPN\" width=\"256\">#{str_kana}</language>\n")
    @buffer << Kconv.toutf8("\t\t<language name=\"JPN_KANJI\" width=\"256\">#{str_kanji}</language>\n")
    @buffer << Kconv.toutf8("\t</row>\n")

    cnt = @count
    @count+=1
    return cnt
  end


  def tag_translate( str )

      #単語登録分回す
      $gmmtag_hash.each {|k,v|

        #引っかかった単語登録
        reg = Regexp.new(/#{k}\d/)
        if reg =~ str then

          #単語だけ取り出して
          tag = str.scan( reg )

          #取り出した単語分まわす
          #(単語が複数あり、指定バッファが違う場合を考慮している)
          tag.each { |t|
        
            #数値を取り出して、置換する

            t = t.split( /#{k}/ )

            v2 = v
            v2.gsub!( /@/, t[1] )

            str.gsub!( reg, v )
          }

        end
      }

      return str
  end

  def genelate( filename )

    footer = open("footer.file","r:UTF-8")
    @buffer << footer.read
    footer.close

    newfile = File.new( filename, "w" )
    @buffer.each{ |line|
      newfile.puts( line )
    }
    newfile.close
  end
  
end



#----------------------------------------------------
# 男女別々のGMMを作成するクラス
#----------------------------------------------------
class MAKE_GMM_MF
  def initialize
    @gmm_m = MAKE_GMM.new
    @gmm_f = MAKE_GMM.new
  end
  
  def write_both( tag, str_kana, str_kanji )
    cnt_m = @gmm_m.write( tag, str_kana, str_kanji )
    cnt_f = @gmm_f.write( tag, str_kana, str_kanji )

    if cnt_m != cnt_f then
      exit(1)
    end
    return cnt_m
  end

  def str_check( str_kana_m, str_kanji_m, str_kana_f, str_kanji_f, line )
    #全部nilだったらスルー
    if str_kana_m==nil && str_kanji_m==nil && str_kana_f==nil &&  str_kanji_f ==nil then
      return
    end
    
    #どれか入っていない
    if str_kana_m==nil || str_kanji_m==nil || str_kana_f==nil || str_kanji_f==nil then
      printf( "%d行目に空欄がある\n", line+1 )
      exit(1)
    end
  end

  def write_each( tag, str_kana_m, str_kanji_m, str_kana_f, str_kanji_f, line )
    # nilチェック
    str_check( str_kana_m, str_kanji_m, str_kana_f, str_kanji_f, line )
    #書き出し
    cnt_m = @gmm_m.write( tag, str_kana_m, str_kanji_m )
    cnt_f = @gmm_f.write( tag, str_kana_f, str_kanji_f )
    if cnt_m != cnt_f then
      exit(1)
    end
    return cnt_m
  end

  def genelate( filename )
    @gmm_m.genelate( File.basename(filename, ".*")+"_m"+File.extname(filename) )
    @gmm_f.genelate( File.basename(filename, ".*")+"_f"+File.extname(filename) )
  end
end

#----------------------------------------------------
# メイン処理
#----------------------------------------------------
# シナリオフォーマット読み込み
datas = CSV.readlines(ARGV[0])
#p datas

#メッセージコンバート
gmm = MAKE_GMM_MF.new

# シナリオクラス初期化
pokewood = POKEWOOD_SENARIO.new( datas[1][1], datas[2][1], datas[3][1], 
                                 datas[4][1], datas[5][1], datas[7][1],
                                 datas[8][1], datas[9][1], datas[13][1] )

#敵トレーナータイプ名入力
trainer_name_rec  = gmm.write_each( "pw_trname_rec",  datas[5][3], datas[5][3], datas[5][5], datas[5][5], 5 )
trainer_name_show = gmm.write_each( "pw_trname_show", datas[5][4], datas[5][4], datas[5][6], datas[5][6], 5 )


#タイトル情報入力
pokewood.top_info( datas[10][1], datas[6][1] )

#BGM指定入力
pokewood.set_bgm(  datas[8][3], datas[9][3], datas[10][3], datas[11][3] )

#勝利条件情報入力
pokewood.set_win_provision( datas[1][3], datas[2][3], datas[2][4], datas[3][3] )


#敵トレーナー情報セット
pokewood.set_trainer_info( datas[4][3], datas[4][4], datas[4][5], datas[4][6] )

#敵トレーナースターランクセット
pokewood.trainer_rank = datas[6][3].to_i

#自分トレーナー情報セット
pokewood.set_mytype_info( datas[7][3], datas[7][4] )


# BG切り替え情報入力
pokewood.set_bg_change( 0, datas[14][1], datas[14][2] )
pokewood.set_bg_change( 1, datas[15][1], datas[15][2] )
pokewood.set_bg_change( 2, datas[16][1], datas[16][2] )


#手持ちポケモン情報入力
ENEMY_POKE_NUM.times{|i|
  pokewood.set_enemy_poke( i, datas[19+i][1], datas[19+i][2], datas[19+i][3] )
}


#選択時メッセージ格納
SELECT_MSG.times{|i|
  msg1 = gmm.write_each( "pw_wazasel_hero_#{i}", datas[i+28][1], datas[i+28][2], datas[i+28][3], datas[i+28][4], i+28 )
  msg2 = gmm.write_each( "pw_wazasel_enemy_#{i}", datas[i+28][5], datas[i+28][6], datas[i+28][7], datas[i+28][8], i+28 )
  pokewood.set_select_msg(            i, msg1 )
  pokewood.set_select_msg( SELECT_MSG+i, msg2 )
}


#シチュエーション別メッセージ格納
SITUATION_MSG.times{|i|
  msg1 = gmm.write_each( "pw_situ_hero_#{i}", datas[i+39][1], datas[i+39][2], datas[i+39][3], datas[i+39][4], i+39 )
  msg2 = gmm.write_each( "pw_situ_enemy_#{i}", datas[i+39][5], datas[i+39][6], datas[i+39][7], datas[i+39][8], i+39 )
  pokewood.set_situation_msg(                i, msg1 )
  pokewood.set_situation_msg( SITUATION_MSG+i, msg2 )
}


#ターン毎メッセージ格納
TURN_MSG.times{|i|
  msg1 = gmm.write_each( "pw_turn_hero_#{i}", datas[i+47][1], datas[i+47][2], datas[i+47][3], datas[i+47][4], i+47 )
  msg2 = gmm.write_each( "pw_turn_enemy_#{i}", datas[i+47][5], datas[i+47][6], datas[i+47][7], datas[i+47][8], i+47 )
  pokewood.set_turn_msg(            i, msg1 )
  pokewood.set_turn_msg( TURN_MSG+i, msg2 )
}



#イベント取り込み
EVENT_MAX.times{|i|
  if datas.length>EVENT1_START_LINE+EVENT_LINE_HEIGHT*i then
    pokewood.set_event_data( datas,i,EVENT1_START_LINE+EVENT_LINE_HEIGHT*i,gmm )
  end
}


#バイナリ出力
filename = ARGV[0].sub(/\.txt/,".bin")

File.open(filename,"w"){|file|
  file.binmode
  file.write( pokewood.pack )
}

#GMM出力
gmm.genelate( ARGV[0].sub(/\.txt/,".gmm") )

