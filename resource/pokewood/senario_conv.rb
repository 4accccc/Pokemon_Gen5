#----------------------------------------------------------------------------
# ポケウッドシナリオコンバーター
# 2011/03 by mori
# 
# 入力：ruby senario_conv.rb <入力テキスト>
# 出力：<入力テキスト>の拡張子を「.bin」にしたものを出力します
#----------------------------------------------------------------------------
require 'csv'
require 'kconv'

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
EVENT_NO_PROVISION      = "なし"

EVENT_LINE_HEIGHT = 14   # イベントデータの行数
EVENT1_START_LINE = 69	 # イベントデータスタート位置

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


$bg_hash       = {"チェンジ０"=>0, "チェンジ１"=>1, "チェンジ２"=>2, "チェンジ３"=>3,}
$waza_ret_hash = { "技１"=>0, "技２"=>1, "技３"=>2, "技４"=>3 }
$batt_hash     = {"シングル"=>0, "ダブル"=>1}
$regu_hash     = {"制限なし"=>0, "フラットルール"=>1}
$itemok_hash   = {"使用可"=>0, "使用不可"=>1}
$sex_hash      = {"♂"=>0, "♀"=>1, "なし"=>2 }
$stage_hash    = {"両方役者"=>PWS_A_TRAINER_B_TRAINER, 
                  "両方ポケ"=>PWS_BOTH_POKE,
                  "敵役者"=>PWS_A_POKEMON_B_TRAINER,
                  "自分役者"=>PWS_A_TRAINER_B_POKEMON,
                  "敵退出"=>PWS_A_TRAINER_B_NONE,
                  "自分退出"=>PWS_A_NONE_B_TRAINER,
                  "無人"=>PWS_A_NONE_B_NONE}
$gmmtag_hash   = {"%MYPOKE"=>"[1:02:ポケモンニックネーム:@]",
                  "%MYNAME"=>"[1:00:トレーナー名:@]"}
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
  def set_event_data( data, index, start, gmm )

    if data[start][3]==EVENT_CLEARTURN_KEYWORD then
      @event[index].start_turn = 100
    elsif data[start][3]==EVENT_LOSETURN_KEYWORD then
      @event[index].start_turn = 101
    else
      @event[index].start_turn = data[start][3].to_i
    end
    
    if data[start][5]!=EVENT_NO_PROVISION then
      @event[index].flag_provision = data[start][5].to_i
    end
    EVENT_TALK_MAX.times{|i|
      msg1 = gmm.write_each( "pw_event#{index}_hero_#{i}", data[start+i+2][1], data[start+i+2][2], data[start+i+2][3], data[start+i+2][4] )
      msg2 = gmm.write_both( "pw_event#{index}_enemy_#{i}", data[start+i+2][5], data[start+i+2][6] )
      @event[index].set_event_msg(               i, msg1 )
      @event[index].set_event_msg(i+EVENT_TALK_MAX, msg2 )
      @event[index].set_stage_cast_data(i, hash2no($stage_hash, data[start+i+2][9]))

    }
    EVENT_REACTION_MAX.times{|i|
      msg1 = gmm.write_each( "pw_event#{index}_react_hero_#{i}", data[start+9+i][1], data[start+9+i][2], data[start+9+i][3], data[start+9+i][4] )
      msg2 = gmm.write_both( "pw_event#{index}_react_enemy_#{i}", data[start+9+i][5], data[start+9+i][6] )
      @event[index].set_reaction_data(i, msg1,
                                      msg2,data[start+9+i][7],data[start+9+i][8])
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

#----------------------------------------------------
# GMM作成クラス
#   GMMファイルはUTF-8でエンコード
#----------------------------------------------------
class MAKE_GMM
  def initialize
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
      $gmmtag_hash.each {|k,v|
        reg = Regexp.new(/#{k}\d/)
        if reg =~ str then
          tag = str.scan( reg )
          tag = str.scan( /\d/ )
          v.gsub!( /@/, tag[0] )
          str.gsub!( reg, v )
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

  def write_each( tag, str_kana_m, str_kanji_m, str_kana_f, str_kanji_f )
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
                                 datas[4][1], datas[5][1], datas[7][1] )

#タイトル文字列入力
title   = gmm.write_both( "pw_scene_title", datas[9][1], datas[9][2] )
mission = gmm.write_both( "pw_scene_mission", datas[11][1], datas[11][2] )
hint    = gmm.write_both( "pw_scene_hint", datas[12][1], datas[12][2] )

#タイトル情報入力
pokewood.top_info( title, datas[10][1], mission, hint, datas[13][1] )


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
  msg1 = gmm.write_each( "pw_wazasel_hero_#{i}", datas[i+28][1], datas[i+28][2], datas[i+28][3], datas[i+28][4] )
  msg2 = gmm.write_both( "pw_wazasel_enemy_#{i}", datas[i+28][5], datas[i+28][6] )
  pokewood.set_select_msg(            i, msg1 )
  pokewood.set_select_msg( SELECT_MSG+i, msg2 )
}

#シチュエーション別メッセージ格納
SITUATION_MSG.times{|i|
  msg1 = gmm.write_each( "pw_situ_hero_#{i}", datas[i+39][1], datas[i+39][2], datas[i+39][3], datas[i+39][4] )
  msg2 = gmm.write_both( "pw_situ_enemy_#{i}", datas[i+39][5], datas[i+39][6] )
  pokewood.set_situation_msg(                i, msg1 )
  pokewood.set_situation_msg( SITUATION_MSG+i, msg2 )
}

#ターン毎メッセージ格納
TURN_MSG.times{|i|
  msg1 = gmm.write_each( "pw_turn_hero_#{i}", datas[i+47][1], datas[i+47][2], datas[i+47][3], datas[i+47][4] )
  msg2 = gmm.write_both( "pw_turn_enemy_#{i}", datas[i+47][5], datas[i+47][6] )
  pokewood.set_turn_msg(            i, msg1 )
  pokewood.set_turn_msg( TURN_MSG+i, msg2 )
}


#イベント取り込み
EVENT_MAX.times{|i|
  if datas.length>EVENT1_START_LINE+EVENT_LINE_HEIGHT*i then
    pokewood.set_event_data(datas,i,EVENT1_START_LINE+EVENT_LINE_HEIGHT*i,gmm )
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

