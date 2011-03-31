//============================================================================================
/**
 * @file  wbt_trlist_struct.h
 * @brief WBTトレーナーROMデータ構造
 * @author  tamada GAMEFREAK inc.
 * @date  2011.03.09
 *
 * resource/wbt/wbt_trainer/の下のコンバータからも参照されているので
 * 記述内容には注意すること
 */
//============================================================================================

typedef struct {
  u8  type;               // タイプ指定
  u8  sex;                // 性別指定
  u16 mmdl_id;            // 動作モデル指定
  u8 tr_type;             //トレーナータイプ指定
  u8 entry_timing;        //エントリタイミング
  BOOL entry_all;         //オールトーナメント参加
  BOOL entry_champ;       //チャンピオントーナメント参加
  u8  type_tournament_id; //タイプトーナメント種別
  u8  priority; //優先度
  u8  ai_type; //AI
  u16 bgm_id;             // BGM
  u16 ace_poke_id;        //切り札ポケモン
  u16 msg_1st_start;      //一回戦用対戦前メッセージ
  u16 msg_2nd_start;      //二回戦用対戦前メッセージ
  u16 msg_final_start;    //決勝戦用対戦前メッセージ
  u16 msg_fight_after;    //対戦後メッセージ
  u16 msg_fight_win;      //対戦勝メッセージ
  u16 msg_fight_lose;     //対戦負メッセージ
  u16 msg_fight_announce; //前口上メッセージ
} WBT_TRDATA; 

