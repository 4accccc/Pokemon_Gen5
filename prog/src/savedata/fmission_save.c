//============================================================================================
/**
 * @file    fmission_save.c
 * @brief   フェスミッションセーブデータ
 * @author　Miyuki Iwasawa
 * @date    11.07.22
 */
//============================================================================================
#include <gflib.h>
#include "gamesystem/link_festival.h"
#include "gamesystem/fest_mission_local.h"
#include "savedata/save_tbl.h"
#include "savedata/fmission_save.h"
#include "fmission_save_local.h"

SDK_COMPILER_ASSERT( FMISSION_ID_MAX <= FMISSION_STATUS_MAX );
SDK_COMPILER_ASSERT( sizeof(FMISSION_SAVE) == 256 );

//---------------------------------------------------------------------------
/**
 * @brief	ワークのサイズ取得
 * @return	int		サイズ
 */
//---------------------------------------------------------------------------
int FMISSION_SAVE_GetWorkSize(void)
{
	return sizeof(FMISSION_SAVE);
}

//---------------------------------------------------------------------------
/**
 * @brief	ワークの初期化
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_Init( FMISSION_SAVE * sv )
{
  int i;

	MI_CpuClear8( sv, sizeof(FMISSION_SAVE));

  for( i = 0;i < FMISSION_ID_MAX; i++ ){
    sv->status[i].new_f = TRUE;  //初期値がTRUE
  } 
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブポインタを返す
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
FMISSION_SAVE* SAVEDATA_GetFMissionSave( SAVE_CONTROL_WORK* sv_ctrl )
{
  return (FMISSION_SAVE*)SaveControl_DataPtrGet( sv_ctrl, GMDATA_ID_FMISSION_SAVE );
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション開催回数を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
u32 FMISSION_SAVE_GetMissionOpenNum( FMISSION_SAVE* sv )
{
  return sv->open_num;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション開催回数をインクリメントする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
u32 FMISSION_SAVE_IncMissionOpenNum( FMISSION_SAVE* sv )
{
  if( sv->open_num < FMISSION_SAVE_OPEN_NUM_MAX ){
    ++sv->open_num;
  }
  return sv->open_num;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション開催回数をセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_SetMissionOpenNum( FMISSION_SAVE* sv, u32 value )
{
  if( value <= FMISSION_SAVE_OPEN_NUM_MAX ){
    sv->open_num = value;
  }
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション参加回数を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
u32 FMISSION_SAVE_GetMissionEntryNum( FMISSION_SAVE* sv )
{
  return sv->entry_num;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション参加回数をインクリメントする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_IncMissionEntryNum( FMISSION_SAVE* sv )
{
  if( sv->entry_num < FMISSION_SAVE_ENTRY_NUM_MAX ){
    ++sv->entry_num;
  }
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション参加回数をセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_SetMissionEntryNum( FMISSION_SAVE* sv, u32 value )
{
  if( value <= FMISSION_SAVE_ENTRY_NUM_MAX ){
    sv->entry_num = value;
  }
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション達成回数を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
u32 FMISSION_SAVE_GetMissionClearNum( FMISSION_SAVE* sv )
{
  return sv->clear_num;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション達成回数をインクリメントする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_IncMissionClearNum( FMISSION_SAVE* sv )
{
  if( sv->clear_num < FMISSION_SAVE_CLEAR_NUM_MAX ){
    ++sv->clear_num;
  }
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション達成回数をセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_SetMissionClearNum( FMISSION_SAVE* sv, u32 value )
{
  if( value <= FMISSION_SAVE_CLEAR_NUM_MAX ){
    sv->clear_num = value;
  }
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション最高参加者数を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
u8 FMISSION_SAVE_GetMissionMaxJoinNum( FMISSION_SAVE* sv )
{
  return sv->max_join_num;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション最高参加者数をアップデートする	
 * @param	work		ワークへのポインタ
 *
 * @param entry_num 参加人数、記録より少なければ更新しない
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_UpdateMissionMaxJoinNum( FMISSION_SAVE* sv, u8 join_num )
{
  if( join_num > LINKFEST_JOIN_MAX ){
    GF_ASSERT(0);
    return;
  }
  if( sv->max_join_num < join_num ){
    sv->max_join_num = join_num;
  }
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション最高参加者数をセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_SetMissionMaxJoinNum( FMISSION_SAVE* sv, u16 join_num )
{
  if( join_num <= LINKFEST_JOIN_MAX ){
    sv->max_join_num = join_num;
  }
}

//---------------------------------------------------------------------------
/**
 * @brief ミッショントップスコアを取った回数を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
u32 FMISSION_SAVE_GetMissionTopScoreNum( FMISSION_SAVE* sv )
{
  return sv->top_score_num;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッショントップスコアを取った回数をアップデートする	
 * @param	work		ワークへのポインタ
 *
 * @param entry_num 参加人数、記録より少なければ更新しない
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_UpdateMissionTopScoreNum( FMISSION_SAVE* sv, u16 top_num )
{
  if( top_num > FMISSION_SAVE_TOP_SCORE_NUM_MAX ){
    GF_ASSERT(0);
    return;
  }
  if( sv->top_score_num < top_num ){
    sv->top_score_num = top_num;
  }
}

//---------------------------------------------------------------------------
/**
 * @brief ミッショントップスコアを取った回数をセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_SetMissionTopScoreNum( FMISSION_SAVE* sv, u16 top_num )
{
  if( top_num <= FMISSION_SAVE_TOP_SCORE_NUM_MAX ){
    sv->top_score_num = top_num;
  }
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコア(数値)を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
u16 FMISSION_SAVE_GetMissionStatusHighScoreNum( FMISSION_SAVE* sv, FEST_MISSION_ID id )
{
  if( id >= FMISSION_ID_MAX ){
    GF_ASSERT(0);
    return 0;
  }
  return sv->status[id].high_score;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコア(数値)をアップデートする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
BOOL FMISSION_SAVE_UpdateMissionStatusHighScoreNum( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 new_score )
{
  if( id >= FMISSION_ID_MAX || new_score > FMISSION_SAVE_HIGH_SCORE_MAX ){
    GF_ASSERT(0);
    return FALSE;
  }
  if( sv->status[id].high_score < new_score ){
    sv->status[id].high_score = new_score;
    return TRUE;
  }
  return FALSE;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコア(時間)を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_GetMissionStatusHighScoreTime( FMISSION_SAVE* sv, FEST_MISSION_ID id, u8* min, u8* sec )
{
  if( id >= FMISSION_ID_MAX ){
    *min = 0;
    *sec = 0;
    GF_ASSERT(0);
    return;
  }
  *sec = sv->status[id].high_score%60;
  *min = sv->status[id].high_score/60;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコア(数値)をアップデートする	
 * @param	work		ワークへのポインタ
 * @param id      ミッションID
 * @param rest_time_sec 残り時間(秒)
 */
//---------------------------------------------------------------------------
BOOL FMISSION_SAVE_UpdateMissionStatusHighScoreTime( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 rest_time_sec )
{
  if( id >= FMISSION_ID_MAX ){
    GF_ASSERT(0);
    return FALSE;
  }
  if( rest_time_sec > FMISSION_TIME_MAX ){
    rest_time_sec = FMISSION_TIME_MAX;
  }
  if( rest_time_sec > sv->status[id].high_score ){
    sv->status[id].high_score = rest_time_sec;
    return TRUE;
  }
  return FALSE;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコアをセット
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_SetMissionStatusHighScore( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 score )
{
  if( id >= FMISSION_ID_MAX || score > FMISSION_SAVE_HIGH_SCORE_MAX ){
    GF_ASSERT(0);
    return;
  }
  sv->status[id].high_score = score;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス マイレコード<自分のハイスコア(数値)>を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
u16 FMISSION_SAVE_GetMissionStatusMyRecord( FMISSION_SAVE* sv, FEST_MISSION_ID id )
{
  if( id >= FMISSION_ID_MAX ){
    GF_ASSERT(0);
    return 0;
  }
  return sv->status[id].my_record;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス マイレコード<自分のハイスコア(数値)>をアップデートする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
BOOL FMISSION_SAVE_UpdateMissionStatusMyRecord( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 new_score )
{
  if( id >= FMISSION_ID_MAX || new_score > FMISSION_SAVE_HIGH_SCORE_MAX ){
    GF_ASSERT(0);
    return FALSE;
  }
  if( sv->status[id].my_record < new_score ){
    sv->status[id].my_record = new_score;
    return TRUE;
  }
  return FALSE;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス マイレコード<自分のハイスコア(数値)>をセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_SetMissionStatusMyRecord( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 score )
{
  if( id >= FMISSION_ID_MAX || score > FMISSION_SAVE_HIGH_SCORE_MAX ){
    GF_ASSERT(0);
    return;
  }
  sv->status[id].my_record = score;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス クリアしたレベルを返す	
 * @param	work		ワークへのポインタ
 *
 * @retval  FMISSION_CLEAR_LEVEL_0 未クリア
 * @retval  FMISSION_CLEAR_LEVEL_1～4  各レベルまでをクリアした
 */
//---------------------------------------------------------------------------
FMISSION_CLEAR_LEVEL FMISSION_SAVE_GetMissionStatusClearLevel( FMISSION_SAVE* sv, FEST_MISSION_ID id )
{
  if( id >= FMISSION_ID_MAX ){
    GF_ASSERT(0);
    return 0;
  }
  return sv->status[id].level;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス クリアしたレベルをセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_SetMissionStatusClearLevel( FMISSION_SAVE* sv, FEST_MISSION_ID id, FMISSION_CLEAR_LEVEL level )
{
  if( id >= FMISSION_ID_MAX || level >= FMISSION_CLEAR_LEVEL_MAX ){
    GF_ASSERT(0);
    return;
  }
  sv->status[id].level = level;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 新着フラグを返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
BOOL FMISSION_SAVE_CheckMissionStatusNewFlag( FMISSION_SAVE* sv, FEST_MISSION_ID id )
{
  if( id >= FMISSION_ID_MAX ){
    GF_ASSERT(0);
    return FALSE;
  }
  return sv->status[id].new_f;
}
//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 新着フラグをセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_SetMissionStatusNewFlag( FMISSION_SAVE* sv, FEST_MISSION_ID id )
{
  if( id >= FMISSION_ID_MAX ){
    GF_ASSERT(0);
    return;
  }
  sv->status[id].new_f = TRUE;
}
//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 新着フラグをセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_ResetMissionStatusNewFlag( FMISSION_SAVE* sv, FEST_MISSION_ID id )
{
  if( id >= FMISSION_ID_MAX ){
    GF_ASSERT(0);
    return;
  }
  sv->status[id].new_f = FALSE;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション　ハイリンクレベル経験値取得	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
u16 FMISSION_SAVE_GetHilinkLevelExp( FMISSION_SAVE* sv, FMISSION_SV_EXP_IDX version_idx )
{
  if( version_idx >= FMISSION_SV_EXP_IDX_MAX ){
    GF_ASSERT(0);
    return 0;
  }
  return sv->exp[version_idx];
}

//---------------------------------------------------------------------------
/**
 * @brief ミッション　ハイリンクレベル経験値セット	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_SetHilinkLevelExp( FMISSION_SAVE* sv, FMISSION_SV_EXP_IDX version_idx, u16 value )
{
  if( version_idx >= FMISSION_SV_EXP_IDX_MAX || value >= FMISSION_SV_HILINK_LEVEL_EXP_STEP ){
    GF_ASSERT_MSG(0,"%d->%d\n",version_idx,value);
    return;
  }
  sv->exp[version_idx] = value;
}

