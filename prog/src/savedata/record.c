//============================================================================================
/**
 * @file  record.c
 * @brief ゲーム内カウント処理関連
 * @author  tamada
 * @date  2006.03.28
 */
//============================================================================================

#include <gflib.h>
#include "savedata/save_tbl.h"
#include "savedata/record.h"
#include "savedata/th_rank_def.h"


//============================================================================================
//============================================================================================
typedef struct{
  u16 crc16ccitt_hash;  ///<CRC
  u16 coded_number;   ///<暗号化キー
}RECORD_CRC;

struct RECORD{
  u32 large_rec[LARGE_REC_NUM]; //(70+1)x4 byte  = 284 byte
  u16 small_rec[SMALL_REC_NUM]; //    96x2 byte  = 192 byte
  //これは必ず最後
  RECORD_CRC crc;   //CRC & 暗号化キー   4 byte
};
#ifdef _NITRO
// 構造体が想定のサイズとなっているかチェック
SDK_COMPILER_ASSERT(sizeof(RECORD) == 284+192+4); // LARGE_REC_NUMx4 + SMALL_REC_NUMx2 + 4(CRC)
#endif

//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void Record_Coded(RECORD *rec, int id);
static void Record_Decoded(RECORD *rec, int id);


//============================================================================================
//
//  セーブデータシステムが依存する関数
//
//============================================================================================
//----------------------------------------------------------
/**
 * @brief ワークサイズ取得
 * @return  int   サイズ（バイト単位）
 */
//----------------------------------------------------------
int RECORD_GetWorkSize(void)
{
  return sizeof(RECORD);
}

//----------------------------------------------------------
/**
 * @brief レコード初期化
 * @param rec   レコードデータへのポインタ
 */
//----------------------------------------------------------
void RECORD_Init(RECORD * rec)
{
  MI_CpuClear32(rec, sizeof(RECORD));
  
  rec->crc.coded_number = OS_GetVBlankCount() | (OS_GetVBlankCount() << 8);
  Record_Coded(rec, RECID_REPORT_COUNT);
  
}

//----------------------------------------------------------
/**
 * @brief
 * @param sv
 * @return  RECORDへのポインタ
 */
//----------------------------------------------------------
RECORD * SaveData_GetRecord(SAVE_CONTROL_WORK * sv)
{
  return SaveControl_DataPtrGet(sv, GMDATA_ID_RECORD);
}

//============================================================================================
//
//
//      内部関数
//
//
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief   レコード暗号化
 * @param   rec   
 */
//--------------------------------------------------------------
static void Record_Coded(RECORD *rec, int id)
{
  if(id == RECID_WALK_COUNT){
    return;   //歩数カウンタは暗号化しない
  }
  
  //チェックサム作成(CRCにしたかったけどセーブデータのポインタがないので)
  rec->crc.crc16ccitt_hash //-sizeof(u32)は歩数カウンタ分
    = GFL_STD_CODED_CheckSum(&rec->large_rec[1], 
    sizeof(RECORD) - sizeof(RECORD_CRC) - sizeof(u32)) & 0xffff;

  GFL_STD_CODED_Coded(&rec->large_rec[1], sizeof(RECORD) - sizeof(RECORD_CRC) - sizeof(u32), 
    rec->crc.crc16ccitt_hash + (rec->crc.coded_number << 16));
}

//--------------------------------------------------------------
/**
 * @brief   レコード復号化
 * @param   rec   
 */
//--------------------------------------------------------------
static void Record_Decoded(RECORD *rec, int id)
{
  if(id == RECID_WALK_COUNT){
    return;   //歩数カウンタは暗号化しない
  }
  
  //-sizeof(u32)は歩数カウンタ分
  GFL_STD_CODED_Decoded(&rec->large_rec[1], sizeof(RECORD) - sizeof(RECORD_CRC) - sizeof(u32), 
    rec->crc.crc16ccitt_hash + (rec->crc.coded_number << 16));
}

//----------------------------------------------------------
/**
 * @brief 値の取得
 * @param rec   RECORDへのポインタ
 * @param id    レコード項目指定ID
 * @return  u32   現在の値
 */
//----------------------------------------------------------
static u32 get_value(const RECORD * rec, int id)
{
  if (id < LARGE_REC_MAX) {
    return rec->large_rec[id];
  } else if (id < SMALL_REC_MAX) {
    return rec->small_rec[id - LARGE_REC_MAX];
  }
  GF_ASSERT_MSG(0, "record: id error\n");
  return 0;
}

//----------------------------------------------------------
/**
 * @brief 値をセットする
 * @param rec   RECORDへのポインタ
 * @param id    レコード項目指定ID
 * @param value セットする値
 * @return  u32   現在の値
 */
//----------------------------------------------------------
static u32 set_value(RECORD * rec, int id, u32 value)
{
  if (id < LARGE_REC_MAX) {
    rec->large_rec[id] = value;
  } else if (id < SMALL_REC_MAX) {
    rec->small_rec[id - LARGE_REC_MAX] = value;
  } else {
    GF_ASSERT_MSG(0, "record: id error\n");
  }
  return get_value(rec, id);
}


#define RECODE_MAXTBL_MAX ( 6 )
//----------------------------------------------------------
/**
 * @brief レコード項目ごとの上限を取得する
 * @param id    レコード項目指定ID
 * @return  u32   上限の値
 */
//----------------------------------------------------------
static u32 get_limit(int id)
{
  static u8 limit[SMALL_REC_MAX] = {
    RECID_WALK_COUNT_FLAG,      
    RECID_REPORT_COUNT_FLAG,    
    RECID_CLEAR_TIME_FLAG,      
    RECID_RIDE_CYCLE_FLAG,      
    RECID_BATTLE_COUNT_FLAG,    
    RECID_BTL_ENCOUNT_FLAG,     
    RECID_BTL_TRAINER_FLAG,     
    RECID_CAPTURE_POKE_FLAG,    
    RECID_FISHING_SUCCESS_FLAG, 
    RECID_TAMAGO_HATCHING_FLAG, 
    RECID_POKE_EVOLUTION_FLAG,  
    RECID_RECOVERY_PC_FLAG,     
    RECID_COMM_TRADE_FLAG,      
    RECID_COMM_BATTLE_FLAG,     
    RECID_COMM_BTL_WIN_FLAG,    
    RECID_COMM_BTL_LOSE_FLAG,   
    RECID_WIFI_TRADE_FLAG,      
    RECID_WIFI_BATTLE_FLAG,     
    RECID_WIFI_BTL_WIN_FLAG,    
    RECID_WIFI_BTL_LOSE_FLAG,   
    RECID_WIFI_BTL_DRAW_FLAG,   
    RECID_SHOPPING_CNT_FLAG,    
    RECID_SHOPPING_MONEY_FLAG,  
    RECID_WATCH_TV_FLAG,        
    RECID_SODATEYA_CNT_FLAG,    
    RECID_KILL_POKE_FLAG,       
    RECID_DAYMAX_EXP_FLAG,      
    RECID_DAYCNT_EXP_FLAG,      
    RECID_GTS_PUT_FLAG,         
    RECID_MAIL_WRITE_FLAG,      
    RECID_NICKNAME_FLAG,        
    RECID_PREMIUM_BALL_FLAG,    
    RECID_LEADERHOUSE_BATTLE_FLAG,
    RECID_WIN_BP_FLAG,          
    RECID_USE_BP_FLAG,          
    RECID_SURECHIGAI_NUM_FLAG,  
    RECID_IRC_TRADE_FLAG,       
    RECID_IRC_BATTLE_FLAG,      
    RECID_IRC_BTL_WIN_FLAG,     
    RECID_IRC_BTL_LOSE_FLAG,    
    RECID_PALACE_TIME_FLAG,     
    RECID_PALACE_MISSION_FLAG,  

    RECID_LARGE_42_FLAG,  
    RECID_LARGE_43_FLAG,  
    RECID_LARGE_44_FLAG,  
    RECID_LARGE_45_FLAG,  
    RECID_LARGE_46_FLAG,  
    RECID_LARGE_47_FLAG,  
    RECID_LARGE_48_FLAG,  
    RECID_LARGE_49_FLAG,  
    RECID_LARGE_50_FLAG,  
    RECID_LARGE_51_FLAG,  
    RECID_LARGE_52_FLAG,  
    RECID_LARGE_53_FLAG,  
    RECID_LARGE_54_FLAG,  
    RECID_LARGE_55_FLAG,  
    RECID_LARGE_56_FLAG,  
    RECID_LARGE_57_FLAG,  
    RECID_LARGE_58_FLAG,  
    RECID_LARGE_59_FLAG,  
    RECID_LARGE_60_FLAG,  
    RECID_LARGE_61_FLAG,  
    RECID_LARGE_62_FLAG,  
    RECID_LARGE_63_FLAG,  
    RECID_LARGE_64_FLAG,  
    RECID_LARGE_65_FLAG,  
    RECID_LARGE_66_FLAG,  
    RECID_LARGE_67_FLAG,  
    RECID_LARGE_68_FLAG,  
    RECID_LARGE_69_FLAG,  
    RECID_LARGE_70_FLAG,  

    RECID_DENDOU_CNT_FLAG,        
    RECID_RECOVERY_HOME_FLAG,     
    RECID_WAZA_HANERU_FLAG,       
    RECID_WAZA_WARUAGAKI_FLAG,    
    RECID_WAZA_MUKOU_FLAG,        
    RECID_TEMOTI_KIZETU_FLAG,     
    RECID_TEMOTI_MAKIZOE_FLAG,    
    RECID_NIGERU_SIPPAI_FLAG,     
    RECID_NIGERARETA_FLAG,        
    RECID_FISHING_FAILURE_FLAG,   
    RECID_DAYMAX_KILL_FLAG,       
    RECID_DAYCNT_KILL_FLAG,       
    RECID_DAYMAX_CAPTURE_FLAG,    
    RECID_DAYCNT_CAPTURE_FLAG,    
    RECID_DAYMAX_TRAINER_BATTLE_FLAG, 
    RECID_DAYCNT_TRAINER_BATTLE_FLAG, 
    RECID_DAYMAX_EVOLUTION_FLAG,  
    RECID_DAYCNT_EVOLUTION_FLAG,  
    RECID_FOSSIL_RESTORE_FLAG,    
    RECID_GURUGURU_COUNT_FLAG,    
    RECID_RANDOM_FREE_SINGLE_WIN_FLAG,  
    RECID_RANDOM_FREE_SINGLE_LOSE_FLAG, 
    RECID_RANDOM_FREE_DOUBLE_WIN_FLAG,  
    RECID_RANDOM_FREE_DOUBLE_LOSE_FLAG, 
    RECID_RANDOM_FREE_ROTATE_WIN_FLAG,  
    RECID_RANDOM_FREE_ROTATE_LOSE_FLAG, 
    RECID_RANDOM_FREE_TRIPLE_WIN_FLAG,  
    RECID_RANDOM_FREE_TRIPLE_LOSE_FLAG, 
    RECID_RANDOM_FREE_SHOOTER_WIN_FLAG, 
    RECID_RANDOM_FREE_SHOOTER_LOSE_FLAG,
    RECID_RANDOM_RATE_SINGLE_WIN_FLAG,  
    RECID_RANDOM_RATE_SINGLE_LOSE_FLAG, 
    RECID_RANDOM_RATE_DOUBLE_WIN_FLAG,  
    RECID_RANDOM_RATE_DOUBLE_LOSE_FLAG, 
    RECID_RANDOM_RATE_ROTATE_WIN_FLAG,  
    RECID_RANDOM_RATE_ROTATE_LOSE_FLAG, 
    RECID_RANDOM_RATE_TRIPLE_WIN_FLAG,  
    RECID_RANDOM_RATE_TRIPLE_LOSE_FLAG, 
    RECID_RANDOM_RATE_SHOOTER_WIN_FLAG, 
    RECID_RANDOM_RATE_SHOOTER_LOSE_FLAG,
    RECID_USE_SHOOTER_COUNT_FLAG,       
    RECID_EFFECT_ENCOUNT_NUM_FLAG,      
    RECID_AFFINITY_CHECK_NUM_FLAG,      
    RECID_MUSICAL_PLAY_NUM_FLAG,        
    RECID_MUSICAL_WINNER_NUM_FLAG,      
    RECID_MUSICAL_COMM_NUM_FLAG,        
    RECID_MUSICAL_COMM_WINNER_NUM_FLAG, 
    RECID_MUSICAL_TOTAL_POINT_FLAG,     
    RECID_PDW_SLEEP_POKEMON_FLAG,       
    RECID_POKESHIFTER_COUNT_FLAG,       
    RECID_TRIALHOUSE_COUNT_FLAG,        
    RECID_SURECHIGAI_THANKS_FLAG,       
    RECID_TRIALHOUSE_RANK_FLAG,         
    RECID_TRIALHOUSE_SCORE_FLAG,        

    RECID_SMALL_54_FLAG, 
    RECID_SMALL_55_FLAG, 
    RECID_SMALL_56_FLAG, 
    RECID_SMALL_57_FLAG, 
    RECID_SMALL_58_FLAG, 
    RECID_SMALL_59_FLAG, 
    RECID_SMALL_60_FLAG, 
    RECID_SMALL_61_FLAG, 
    RECID_SMALL_62_FLAG, 
    RECID_SMALL_63_FLAG, 
    RECID_SMALL_64_FLAG, 
    RECID_SMALL_65_FLAG, 
    RECID_SMALL_66_FLAG, 
    RECID_SMALL_67_FLAG, 
    RECID_SMALL_68_FLAG, 
    RECID_SMALL_69_FLAG, 
    RECID_SMALL_70_FLAG, 
    RECID_SMALL_71_FLAG, 
    RECID_SMALL_72_FLAG, 
    RECID_SMALL_73_FLAG, 
    RECID_SMALL_74_FLAG, 
    RECID_SMALL_75_FLAG, 
    RECID_SMALL_76_FLAG, 
    RECID_SMALL_77_FLAG, 
    RECID_SMALL_78_FLAG, 
    RECID_SMALL_79_FLAG, 
    RECID_SMALL_80_FLAG, 
    RECID_SMALL_81_FLAG, 
    RECID_SMALL_82_FLAG, 
    RECID_SMALL_83_FLAG, 
    RECID_SMALL_84_FLAG, 
    RECID_SMALL_85_FLAG, 
    RECID_SMALL_86_FLAG, 
    RECID_SMALL_87_FLAG, 
    RECID_SMALL_88_FLAG, 
    RECID_SMALL_89_FLAG, 
    RECID_SMALL_90_FLAG, 
    RECID_SMALL_91_FLAG, 
    RECID_SMALL_92_FLAG, 
    RECID_SMALL_93_FLAG, 
    RECID_SMALL_94_FLAG, 
    RECID_SMALL_95_FLAG, 
  };
  static const u32 max_tbl[]={
    999999999,
    999999,
    65535,
    9999,
    999,
    6,
  };
  int index = limit[id];

  // idが定義範囲を超えている
  if(index>=SMALL_REC_MAX){
    return 0;
  }
  // max_tblで引っ張れる数値の範囲を超えている
  if( index>= RECODE_MAXTBL_MAX){
    return 0;
  }

  // そのRECORD定義で扱える最大値
  return max_tbl[index];
  
}


//============================================================================================
//
//
//        操作関数
//
//
//============================================================================================
//----------------------------------------------------------
/**
 * @brief レコードをセットする
 * @param rec   RECORDへのポインタ
 * @param id    項目を指定するID
 * @param value セットする値
 * @return  u32   その項目の値
 */
//----------------------------------------------------------
u32 RECORD_Set(RECORD * rec, int id, u32 value)
{
  u32 limit = get_limit(id);
    u32 ret;
  
  Record_Decoded(rec, id);
  
  if (value < limit) {
    ret = set_value(rec, id, value);
  } else {
    ret = set_value(rec, id, limit);
  }
  
  Record_Coded(rec, id);
    return ret;
}

//----------------------------------------------------------
/**
 * @brief 大きければレコードを更新する
 * @param rec   RECORDへのポインタ
 * @param id    項目を指定するID
 * @param value セットする値
 * @return  u32   その項目の値
 */
//----------------------------------------------------------
u32 RECORD_SetIfLarge(RECORD * rec, int id, u32 value)
{
  u32 limit = get_limit(id);
  u32 now;
    u32 ret;

  Record_Decoded(rec, id);

  now = get_value(rec, id);
  ret = now;
  
  if(value > limit){
    value = limit;  //上限チェック
  }
  if (now < value) {
    ret = set_value(rec, id, value);
  } else {
    if(now > limit){
      ret = set_value(rec, id, limit);
    }
  }

  Record_Coded(rec, id);
    return ret;
}

//----------------------------------------------------------
/**
 * @brief レコードに値を加える
 * @param rec   RECORDへのポインタ
 * @param id    項目を指定するID
 * @param value 加える値
 * @return  u32   その項目の値
 */
//----------------------------------------------------------
u32 RECORD_Add(RECORD * rec, int id, u32 add_value)
{
  u32 limit = get_limit(id);
  u32 now;
  u32 ret;
    
  Record_Decoded(rec, id);

  now = get_value(rec, id);
  if (now + add_value < limit) {
    ret = set_value(rec, id, now + add_value);
  } else {
    ret = set_value(rec, id, limit);
  }

  Record_Coded(rec, id);
  return ret;
}

//----------------------------------------------------------
/**
 * @brief レコードをインクリメントする
 * @param rec   RECORDへのポインタ
 * @param id    項目を指定するID
 * @param value セットする値
 * @return  u32   その項目の値
 */
//----------------------------------------------------------
u32 RECORD_Inc(RECORD * rec, int id)
{
  return RECORD_Add( rec, id, 1 );
}


//----------------------------------------------------------
/**
 * @brief レコードを取得する
 * @param rec   RECORDへのポインタ
 * @param id    項目を指定するID
 * @return  u32   その項目の値
 */
//----------------------------------------------------------
u32 RECORD_Get(RECORD * rec, int id)
{
  u32 limit = get_limit(id);
  u32 value;

  Record_Decoded(rec, id);
  
  value = get_value(rec, id);

  Record_Coded(rec, id);

  if(value > limit){
    return limit;
  }else{
    return value;
  }
}


// レコードデータ1日更新処理用テーブル
static const u16 oneday_update_tbl[][2]={
  { RECID_DAYMAX_EXP,            RECID_DAYCNT_EXP },    // 1日に稼いだ経験値の最大値
  { RECID_DAYMAX_KILL,           RECID_DAYCNT_KILL},    // 1日に倒したポケモンの最大数
  { RECID_DAYMAX_CAPTURE,        RECID_DAYCNT_CAPTURE,},// 1日に捕獲したポケモンの最大数
  { RECID_DAYMAX_TRAINER_BATTLE, RECID_DAYCNT_TRAINER_BATTLE }, // 1日にトレーナー戦をした最大回数
  { RECID_DAYMAX_EVOLUTION,      RECID_DAYCNT_EVOLUTION }, //1日にポケモン進化させた最大回数
};

//=============================================================================================
/**
 * @brief 1日に一回行うデータ更新処理
 *
 * @param   rec   
 */
//=============================================================================================
void RECORD_1day_Update( RECORD *rec )
{
  int i;
  u32 ret;

  // レコードデータ1日更新処理
  for(i=0;i<NELEMS(oneday_update_tbl);i++){
    // 最大値を超えている場合は登録
    ret=RECORD_SetIfLarge( rec, oneday_update_tbl[i][0], 
                           RECORD_Get(rec, oneday_update_tbl[i][1]));
    // 一日の値はリセット
    RECORD_Set( rec, oneday_update_tbl[i][1], 0);
  }


}



//----------------------------------------------------------
/**
 * @brief ランクのセット(上書きチェック内臓)
 * @param rec     RECORDへのポインタ
 * @param inRank  ランク th_rank_def.h 参照
 * @return  none
 */
//----------------------------------------------------------
void RECORD_SetThRank(RECORD * rec, u8 inRank)
{
  
  if (inRank < TH_RANK_NUM)
  {
    RECORD_SetIfLarge( rec, RECID_TRIALHOUSE_RANK, inRank );
  }
  else GF_ASSERT_MSG( 0,"rank error %d", inRank );
}

//----------------------------------------------------------
/**
 * @brief ランクの取得
 * @param rec   RECORDへのポインタ
 * @return  u8    現在のランク    th_rank_def.h 参照
 */
//----------------------------------------------------------
u8 RECORD_GetThRank(RECORD * rec)
{
  return RECORD_Get( rec, RECID_TRIALHOUSE_RANK );
//  return rec->Rank;
}

//----------------------------------------------------------
/**
 * @brief トライアルハウスのスコアのセット(上書きチェック内臓)
 * @param rec     RECORDへのポインタ
 * @param inScore
 * @return  none
 */
//----------------------------------------------------------
void RECORD_SetThScore(RECORD * rec, u32 inScore)
{
  if (inScore < TH_SCORE_MAX)
  {
    RECORD_SetIfLarge( rec, RECID_TRIALHOUSE_SCORE, inScore );
  }
  else GF_ASSERT_MSG( 0,"th score over %d", inScore );
}

//----------------------------------------------------------
/**
 * @brief トライアルハウスのスコアの取得
 * @param rec     RECORDへのポインタ
 * @return  u32 スコア
 */
//----------------------------------------------------------
u32 RECORD_GetThScore(RECORD * rec)
{
  return RECORD_Get( rec, RECID_TRIALHOUSE_SCORE);
}

#ifdef PM_DEBUG
//----------------------------------------------------------
/**
 * @brief スコアのセット
 * @param rec   RECORDへのポインタ
 * @param inScore セットするスコア
 */
//----------------------------------------------------------
void RECORD_Score_DebugSet(RECORD * rec, const u32 inScore)
{
  u32 score;

  score = inScore;
  
  //カンストチェックする
  if ( score > SCORE_MAX ){
    score = SCORE_MAX;
  }

  RECORD_Set(rec, RECID_REPORT_COUNT, score);
}


static const u16 debug_tbl[][2]={
  {    0,    1},
  { 1000, 1001},
  { 9999, 1000},
  { 5000, 4999},
  { 5000, 5000},
};

void RECORD_DebugSet( RECORD *rec )
{
  int i;
  for(i=0;i<NELEMS(oneday_update_tbl);i++){
    RECORD_Set(rec, oneday_update_tbl[i][0], debug_tbl[i][0]);
    RECORD_Set(rec, oneday_update_tbl[i][1], debug_tbl[i][1]);
  }
}


#endif  //PM_DEBUG


