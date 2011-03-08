#! ruby -Ks
#============================================================================
#
# WBTトレーナーリスト　コンバータ
# 2011.03.08  tamada@GAMEFREAK inc.
#
#
#============================================================================

PROJECT_ROOT = ENV["PROJECT_ROOT"]
  require PROJECT_ROOT + 'tools/label_make/label_make'
  require PROJECT_ROOT + 'tools/gmm_make/gmm_make'
  require PROJECT_ROOT + 'tools/constant'
  require PROJECT_ROOT + 'tools/hash/monsno_hash.rb'
  #require PROJECT_ROOT +
  #require File.dirname(__FILE__) + '/../hash/wazano_hash.rb'
  #require File.dirname(__FILE__) + '/../hash/item_hash.rb'
  #require File.dirname(__FILE__) + '/../hash/tokusei_hash.rb'

#----------------------------------------------------------------------------
#データテーブル
  $type_table = {
    "ノーマル"=>0,
    "ノ－マル"=>0,
    "かくとう"=>1,
    "ひこう"=>2,
    "どく"=>3,
    "じめん"=>4,
    "いわ"=>5,
    "むし"=>6,
    "ゴースト"=>7,
    "ゴ－スト"=>7,
    "はがね"=>8,
    "ほのお"=>9,
    "みず"=>10,
    "くさ"=>11,
    "でんき"=>12,
    "エスパー"=>13,
    "エスパ－"=>13,
    "こおり"=>14,
    "ドラゴン"=>15,
    "あく"=>16,
  }


  $entry_timing_table = {
    "初期参加" => 0,
    "バッジ６個目" => 1,
    "バッジ８個目" => 2,
    "保留" => 3,
    "クリア後" => 4,
    "チャンピオントーナメント時" => 5,
  }

  $maru_batu_table = {
    "○" => "TRUE",
    "×" => "FALSE",
  }

  $bgm_table = {
    "SWジムリーダー戦" => "SEQ_BGM_VS_GYMLEADER",
    "戦闘！チェレン・ベル" => "SEQ_BGM_VS_RIVAL",
    "BWジムリーダー戦" => "SEQ_BGM_VS_GYMLEADER",
    "RGチャンピオン戦" => "SEQ_BGM_VS_CHAMP",
    "GSチャンピオン戦" => "SEQ_BGM_VS_CHAMP",
    "RSチャンピオン戦" => "SEQ_BGM_VS_CHAMP",
    "BWチャンピオン戦" => "SEQ_BGM_VS_CHAMP",
    "Eチャンピオン戦" => "SEQ_BGM_VS_CHAMP",
    "RGジムリーダー戦" => "SEQ_BGM_VS_GYMLEADER",
    "GSジムリーダー戦" => "SEQ_BGM_VS_GYMLEADER",
    "RSジムリーダー戦" => "SEQ_BGM_VS_GYMLEADER",
    "DPジムリーダー戦" => "SEQ_BGM_VS_GYMLEADER",
    "戦闘！シロナ" => "SEQ_BGM_VS_SHIRONA",
    "汎用トーナメント" => "SEQ_BGM_VS_GYMLEADER",
  }
#----------------------------------------------------------------------------
# エクセルデータの各欄の定義
#----------------------------------------------------------------------------
class COL
  enum_const_set %w[
    NO
    NAME
    COMMENT_POSITION
    TYPE
    MMDL_ID
    TRTYPE
    ENTRY_TIMING
    TOURNAMENT_COMMENT
    ALL_ENTRY
    CHAMP_ENTRY
    TYPE_ENTRY
    PRIORITY
    AI
    BGM
    JOBNAME
    ACE_POKE
    MSG_1ST_START
    MSG_2ND_START
    MSG_FINAL_START
    MSG_FIGHT_AFTER
    MSG_FIGHT_WIN
    MSG_FIGHT_LOSE
    MSG_ANNOUNCE
  ]
end


#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
class WBT_TRDATA

  def initialize( column )
    @no = column[COL::NO].to_i
    @name = column[COL::NAME]
    @comment_pos = column[COL::COMMENT_POSITION]
    @type = column[COL::TYPE]
    @mmdl_id = column[COL::MMDL_ID]
    @trtype = column[COL::TRTYPE]
    @entry_timing = column[COL::ENTRY_TIMING]
    @comment_tour = column[COL::TOURNAMENT_COMMENT]
    @all_entry = column[COL::ALL_ENTRY]
    @champ_entry = column[COL::CHAMP_ENTRY]
    @type_entry = column[COL::TYPE_ENTRY]
    @priority = column[COL::PRIORITY].to_i
    @ai = column[COL::AI]
    @bgm = column[COL::BGM]
    @jobname = column[COL::JOBNAME]
    @ace_poke = column[COL::ACE_POKE]
    @msg_1st_start = 0 #column[COL::MSG_1ST_START]
    @msg_2nd_start = 0 #column[COL::MSG_2ND_START]
    @msg_final_start = 0 #column[COL::MSG_FINAL_START]
    @msg_fight_after = 0 #column[COL::MSG_FIGHT_AFTER]
    @msg_fight_win = 0 #column[COL::MSG_FIGHT_WIN]
    @msg_fight_lose = 0 #column[COL::MSG_FIGHT_LOSE]
    @msg_fight_announce = 0 #column[COL::MSG_ANNOUNCE]

    if @mmdl_id == "可変" then @mmdl_id = "WKOBJCODE00" end
    raise Exception, "BGM\"#{@bgm}\"に対応する定義がありません" unless $bgm_table.has_key? @bgm
    raise Exception, "異常なデータです！" if @name == nil
    @ace_poke_no = if $monsno_hash.has_key?(@ace_poke) then
                     $monsno_hash[@ace_poke]
                   else
                     0
                   end
  end

  def type_def( typename )
    if $type_table.has_key?( typename )
      return $type_table[typename ]
    elsif $maru_batu_table.has_key?( typename )
      if $maru_batu_table[typename] == "TRUE"
        return 17
      else
        return -1
      end
    else
      raise Exception, "未知のタイプ指定です'#{typename}'"
    end
  end

  def printout(idx)
    if @no != idx then
      p self
      raise Exception, "インデックスがずれています！def(#{@no}) != count(#{idx})"
    end
    str = ""
    str += sprintf("\t{ //%03d %s\n", @no, @name)
    str += sprintf("\t\t%2d, //%s\n", $type_table[@type], @type)
    str += sprintf("\t\t%-16s,// 動作モデル指定\n", @mmdl_id )
    str += sprintf("\t\tTRTYPE_%-12s, //トレーナータイプ指定\n", @trtype )
    str += sprintf("\t\t%2d, //エントリタイミング\n", $entry_timing_table[@entry_timing] )
    str += sprintf("\t\t%-5s, //オールトーナメント参加\n", $maru_batu_table[@all_entry] )
    str += sprintf("\t\t%-5s, //チャンピオントーナメント参加\n", $maru_batu_table[@champ_entry] )
    str += sprintf("\t\t%2d, //タイプトーナメント種別\n", type_def(@type_entry) )
    str += sprintf("\t\t%2d, //優先度\n", @priority )
    str += sprintf("\t\t%2d, //AI\n", @ai.to_i )
    str += sprintf("\t\t%-40s, // BGM\n", $bgm_table[@bgm] )
    str += sprintf("\t\t%s, //切り札ポケモン\n", @ace_poke_no )
    str += sprintf("\t\t%-40s, //一回戦用対戦前メッセージ\n", @msg_1st_start )
    str += sprintf("\t\t%-40s, //二回戦用対戦前メッセージ\n", @msg_2nd_start )
    str += sprintf("\t\t%-40s, //決勝戦用対戦前メッセージ\n", @msg_final_start )
    str += sprintf("\t\t%-40s, //対戦後メッセージ\n", @msg_fight_after )
    str += sprintf("\t\t%-40s, //対戦勝メッセージ\n", @msg_fight_win )
    str += sprintf("\t\t%-40s, //対戦負メッセージ\n", @msg_fight_lose )
    str += sprintf("\t\t%-40s, //前口上メッセージ\n", @msg_fight_announce )

    str += sprintf("\t},\n")

    return str
  end
end

#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
def read_trdata_list( filename )
  trdata_list = []

  File.open(filename){|file|
    while line = file.gets
      column = line.chomp.split(",")
      if column[0] == "No" then
        break
      end
    end

    while line = file.gets
      column = line.chomp.split(",")
      if column.size < COL::PRIORITY then
        break
      else
        trdata_list << WBT_TRDATA::new( column )
      end
    end
  }
  return trdata_list
end
#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
def put_trdata_source( trdata_list )
  output = ""
  output += sprintf("static const WBT_TRDATA WBTTrainerData[] = {\n");

  trdata_list.each_with_index{|trdata, idx|
    output += trdata.printout(idx)
  }
  output += "};\n"
  return output
end


#----------------------------------------------------------------------------
#----------------------------------------------------------------------------
puts put_trdata_source( read_trdata_list( ARGV[0] ) )






