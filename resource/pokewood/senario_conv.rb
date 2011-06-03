#----------------------------------------------------------------------------
# ポケウッドシナリオコンバーター
# 2011/03 by mori
# 
# 入力：ruby senario_conv.rb <入力テキスト>
# 出力：<入力テキスト>の拡張子を「.bin」にしたものを出力します
#----------------------------------------------------------------------------
require 'csv'


ENEMY_POKE_NUM = 6       # 敵トレーナーのポケモン保持数(仮）
SELECT_MSG     = 9       # 技選択時のメッセージの種類数（自分・敵）
SITUATION_MSG  = 6       # シチュエーション別メッセージの種類数（自分・敵）
TURN_MSG       = 20      # ターンごとメッセージ数（自分・敵）
EVENT_MAX      = 7       # イベント最大数
EVENT_TALK_MAX = 7       # イベント中のメッセージの最大数(自分・敵）
EVENT_REACTION_MAX   = 4 # イベント中の選択肢データ最大数
EVENT_REACTION_ARRAY = 4 # 選択肢データ内の項目数（自分会話・相手会話・技指定・背景チェンジ）
EVENT_CLEARTURN_KEYWORD = "クリア"
EVENT_LOSETURN_KEYWORD  = "失敗"

EVENT_LINE_HEIGHT = 14   # イベントデータの行数
EVENT1_START_LINE = 69	 # イベントデータスタート位置

msgtitle_path   = "..\\..\\prog\\include\\msg\\msg_pw_movietitle.h"
msgsenario_path = "..\\..\\prog\\include\\msg\\msg_pw_senario0"

$msg_hash = Hash.new
$bg_hash       = {"チェンジ０"=>0, "チェンジ１"=>1, "チェンジ２"=>2, "チェンジ３"=>3,}
$waza_ret_hash = { "技１"=>0, "技２"=>1, "技３"=>2, "技４"=>3 }
$batt_hash     = {"シングル"=>0, "ダブル"=>1}
$regu_hash     = {"制限なし"=>0, "フラットルール"=>1}
$itemok_hash   = {"使用可"=>0, "使用不可"=>1}
$sex_hash      = {"♂"=>0, "♀"=>1, "なし"=>2 }
$stage_hash    = {"両方役者"=>0, "両方ポケ"=>1,"敵役者"=>2,"自分役者"=>3,"敵退出"=>4,"自分退出"=>6,"無人"=>7}
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
    @type           = type
    @start_turn     = -1
    @flag_provision = -1
    @padding        = -1
    @talk_msg       = Array.new( EVENT_TALK_MAX*2 ).fill(-1)
    @reaction_data  = Array.new( EVENT_REACTION_MAX ).fill(-1)
    @stage_cast     = Array.new(EVENT_TALK_MAX).fill(-1)
    EVENT_REACTION_MAX.times{|i|
      @reaction_data[i] = Array.new( EVENT_REACTION_ARRAY ).fill(-1)
    }
  end
  attr_accessor :type, :start_turn, :talk_msg, :reaction_data, :stage_cast, :flag_provision

  def set_event_msg( index, msg )
    @talk_msg[index] = msg2id( msg )
  end
  
  def set_reaction_data( index, mymsg, e_msg, reaction, vfx )
    @reaction_data[index][0] = msg2id( mymsg )
    @reaction_data[index][1] = msg2id( e_msg )
    @reaction_data[index][2] = hash2no( $waza_ret_hash, reaction)
    @reaction_data[index][3] = hash2no( $bg_hash, vfx )
  end

  def set_stage_cast_data( index, dat )
    @stage_cast[index] = dat;
  end

  #バイナリデータ用に配列に連結
  # s16(6+14+7+16) = 78
  def pack
    array = Array.new
    array << @type
    array << @start_turn
    array << @flag_provision
    array << @padding
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
  def initialize( senario, batt_mode, reguration, poke_limit, item_ok, turn_max )
    @senario    = senario.to_i
    @batt_mode  = $batt_hash[batt_mode]
    @reguration = $regu_hash[reguration]
    @poke_limit = poke_limit.to_i
    @item_ok    = $itemok_hash[item_ok]
    @turn_max   = turn_max.to_i
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
  attr_accessor :senario, :returation, :poke_limit, :item_ok, :select_msg, :situation_msg, :turn_msg, :event, :turn_max, :enemy_poke

  # 初期情報セット
  def top_info( title, trainer_id, explain, hint, batt_bg )
    @title      = msg2id(title)
    @trainer_id = trainer_id.to_i
    @explain    = msg2id(explain)
    @hint       = msg2id(hint)
    @bg_change  = [-1,-1, -1,-1, -1,-1]
  end
  attr_accessor :title, :trainer_id, :explain, :hint, :batt_bg, :bg_change
  
  # BG切り替えターン指定
  def set_bg_change( index, turn, change )
    @bg_change[index*2]   = turn.to_i
    @bg_change[index*2+1] = hash2no( $bg_hash, change)
  end
  
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
  def set_event_data( data, index, start )
    if data[start][3]==EVENT_CLEARTURN_KEYWORD then
      @event[index].start_turn = 100
    elsif data[start][3]==EVENT_LOSETURN_KEYWORD then
      @event[index].start_turn = 101
    else
      @event[index].start_turn = data[start][3].to_i
    end
    
    @event[index].flag_provision = data[start][5].to_i
    EVENT_TALK_MAX.times{|i|
       @event[index].set_event_msg(               i, data[start+i+2][1] )
       @event[index].set_event_msg(i+EVENT_TALK_MAX, data[start+i+2][3])
       @event[index].set_stage_cast_data(i, hash2no($stage_hash, data[start+i+2][7]))
    }
    EVENT_REACTION_MAX.times{|i|
      @event[index].set_reaction_data(i, data[start+9+i][1],
                                      data[start+9+i][3],data[start+9+i][5],data[start+9+i][6])
    }
#    printf("%s\n", @start_turn)
  end

  # バイナリデータ用に配列に連結してエンコード
  # s16(2*8 + 2*4*3 + 9*2*2 + 6*2*2 + 20*2*2 + 78*5) = 500
  def pack
    array = Array.new
    array << @senario
    array << @batt_mode
    array << @reguration
    array << @poke_limit
    array << @item_ok
    array << @turn_max
    array << 0
    array << 0
    
    6.times{|i|
      array << @bg_change[i]
    }
    array << 0
    array << 0
    
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

#----------------------------------------------------
# メッセージ定義置換用ハッシュ作成
#----------------------------------------------------
def make_msg_hash( msg_hash, msgtitle_path, msgsenario_path)
  File.open(msgtitle_path){|file|
    file.each{|line|
      row = line.scan(/\w+/n)
      if row[0]=="define" then
        msg_hash[row[1]] = row[2].to_i
      end
    }
  }
  
  
  File.open(msgsenario_path+ARGV[0][16..17]+".h"){|file|
    file.each{|line|
      row = line.scan(/\w+/n)
      if row[0]=="define" then
        msg_hash[row[1]] = row[2].to_i
      end
    }
  }
  p $msg_hash
end

# ハッシュ存在チェック付きでIDNOに変換
def msg2id( msg )
  if msg==nil then
    return -1
  end
  if $msg_hash.key?(msg) then
    return $msg_hash[msg]
  end
  printf("%sというメッセージIDがありません\n", msg )
  exit
end

# ハッシュからキーが見つからなかった時は-1を返す
def hash2no( hash, str )
  if hash[str]==nil then
    return -1
  end
  return hash[str]
end


#----------------------------------------------------
# メイン処理
#----------------------------------------------------

#メッセージ置換データ作成
make_msg_hash( $msg_hash, msgtitle_path, msgsenario_path)
#p msg_hash

# シナリオフォーマット読み込み
datas = CSV.readlines(ARGV[0])
#p datas



# シナリオクラス初期化
pokewood = POKEWOOD_SENARIO.new( datas[1][1], datas[2][1], datas[3][1], 
                                 datas[4][1], datas[5][1], datas[7][1] )

#タイトル情報入力
pokewood.top_info( datas[9][1],  datas[10][1], datas[11][1], datas[12][1], datas[13][1] )

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
  pokewood.set_select_msg(            i, datas[i+28][1] )
  pokewood.set_select_msg( SELECT_MSG+i, datas[i+28][3] )
}

#シチュエーション別メッセージ格納
SITUATION_MSG.times{|i|
  pokewood.set_situation_msg(                i, datas[i+39][1] )
  pokewood.set_situation_msg( SITUATION_MSG+i, datas[i+39][3] )
}

#ターン毎メッセージ格納
TURN_MSG.times{|i|
  pokewood.set_turn_msg(            i, datas[i+47][1] )
  pokewood.set_turn_msg( TURN_MSG+i, datas[i+47][3] )
}


#イベント取り込み
EVENT_MAX.times{|i|
  if datas.length>EVENT1_START_LINE+EVENT_LINE_HEIGHT*i then
    pokewood.set_event_data(datas,i,EVENT1_START_LINE+EVENT_LINE_HEIGHT*i)
  end
}

#バイナリ出力
filename = ARGV[0].sub(/\.txt/,".bin")

File.open(filename,"w"){|file|
  file.binmode
  file.write( pokewood.pack )
}

