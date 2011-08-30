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
void FMISSION_SAVE_IncMissionOpenNum( FMISSION_SAVE* sv )
{
  if( sv->open_num < FMISSION_SAVE_OPEN_NUM_MAX ){
    ++sv->open_num;
  }
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
  return sv->high_score[id];
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコア(数値)をアップデートする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_UpdateMissionStatusHighScoreNum( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 new_score )
{
  if( id >= FMISSION_ID_MAX || new_score > FMISSION_SAVE_HIGH_SCORE_MAX ){
    GF_ASSERT(0);
    return;
  }
  if( sv->high_score[id] < new_score ){
    sv->high_score[id] = new_score;
  }
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
  *sec = sv->high_score[id]%60;
  *min = sv->high_score[id]/60;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコア(数値)をアップデートする	
 * @param	work		ワークへのポインタ
 * @param id      ミッションID
 * @param rest_time_frame 残り時間(フレーム単位)
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_UpdateMissionStatusHighScoreTime( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 rest_time_frame )
{
  u32 sec;
  if( id >= FMISSION_ID_MAX ){
    GF_ASSERT(0);
    return;
  }
  if( rest_time_frame > FMISSION_LIFE_MAX ){
    rest_time_frame = FMISSION_LIFE_MAX;
  }
  //フレームを秒変換
  sec = rest_time_frame/60+((rest_time_frame%60)!=0); //余りは+1sec

  if( sec > sv->high_score[id] ){
    sv->high_score[id] = sec;
  }
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
  sv->high_score[id] = score;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス クリアしたレベルを返す	
 * @param	work		ワークへのポインタ
 *
 * @retval  0 未クリア
 * @retval  1～3  各レベルまでをクリアした
 */
//---------------------------------------------------------------------------
u8 FMISSION_SAVE_GetMissionStatusClearLevel( FMISSION_SAVE* sv, FEST_MISSION_ID id )
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
void FMISSION_SAVE_SetMissionStatusClearLevel( FMISSION_SAVE* sv, FEST_MISSION_ID id, FMISSION_LEVEL level )
{
  if( id >= FMISSION_ID_MAX ){
    GF_ASSERT(0);
    return;
  }
  sv->status[id].level = (level+1);
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

