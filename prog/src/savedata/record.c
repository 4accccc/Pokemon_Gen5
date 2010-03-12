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
  u32 large_rec[LARGE_REC_NUM]; //(70+1)x4 byte = 284 byte
  u16 small_rec[SMALL_REC_NUM]; //(51+26)x2 byte = 154 byte
  u16 padding;
  //これは必ず最後
  RECORD_CRC crc;   //CRC & 暗号化キー   4 byte
};
#ifdef _NITRO
// 構造体が想定のサイズとなっているかチェック
SDK_COMPILER_ASSERT(sizeof(RECORD) == 284+154+4+2); //+=プラチナ追加分
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
  Record_Coded(rec, RECID_REPORT_COUNT_EX);
  
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
    1,  //RECID_WALK_COUNT
    //4byte
    1,0,1,1,1,1,1,0,0,0,1,1,1,1,1,  //1-16
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  //17-32
    1,1,1,1,1,1,1,1,0,0,0,  //33-43
    0,0,0,0,0,0,0,0,0,0,0,0,0,  //44-56 RECID_GTS_PUT～
    1,1,1,1,1,1,1,1,1,1,1,1,1,0,  //57-70 RECID_LEADERHOUSE_BATTLE～
    
    //2byte
    1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,  //44-59
    0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,  //60-75
    0,0,0,0,0,0,0,0,0,1,  //76-85
    0,0,0,0,0,0,0,      //RECID_GTS_SUCCESS～RECID_GURUGURU_COUNT
  };

  if (id < LARGE_REC_MAX) {
    if(limit[id]){
      GF_ASSERT_MSG( 0, "record: max over id %d 99999999\n", id );
      return 999999999;
    }else{
      GF_ASSERT_MSG( 0, "record: max over id %d 999999\n", id );
      return 999999;
    }
  } else if (id < SMALL_REC_MAX) {
    if(limit[id]){
      return 0xffff;
    }else{
      GF_ASSERT_MSG( 0, "record: max over id %d 9999\n", id );
      return 9999;
    }
  }
  return 0;
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
 * @brief レコードをインクリメントする
 * @param rec   RECORDへのポインタ
 * @param id    項目を指定するID
 * @param value セットする値
 * @return  u32   その項目の値
 */
//----------------------------------------------------------
u32 RECORD_Inc(RECORD * rec, int id)
{
  u32 limit = get_limit(id);
  u32 now;
    u32 ret;
    
  Record_Decoded(rec, id);

    now = get_value(rec, id);
  if (now + 1 < limit) {
    ret = set_value(rec, id, now + 1);
  } else {
    ret =  set_value(rec, id, limit);
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

//----------------------------------------------------------
/**
 * @brief スコアの加算( WBでは存在しない）
 * @param rec   RECORDへのポインタ
 * @param id    スコア指定のID
 */
//----------------------------------------------------------
void RECORD_Score_Add(RECORD * rec, int id)
{

  return ;

#if 0
  u32 score;
  
  GF_ASSERT(id < SCOREID_MAX);

  //カンストチェックする
  score = RECORD_Get(rec, RECID_SCORE);
  if ( score+get_score_number(id) > SCORE_MAX ){
    RECORD_Set(rec, RECID_SCORE, SCORE_MAX);
  }else{
    RECORD_Add(rec, RECID_SCORE, get_score_number(id));
  }
#endif
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
//  return rec->TrialScore;
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

  RECORD_Set(rec, RECID_REPORT_COUNT_EX, score);
}

#endif  //PM_DEBUG


