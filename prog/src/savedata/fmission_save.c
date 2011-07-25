//============================================================================================
/**
 * @file    fmission_save.c
 * @brief   フェスミッションセーブデータ
 * @author　Miyuki Iwasawa
 * @date    11.07.22
 */
//============================================================================================
#include <gflib.h>
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
	MI_CpuClear8( sv, sizeof(FMISSION_SAVE));
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
 * @brief ミッションステータス ハイスコアを返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
u16 FMISSION_SAVE_GetMissionStatusHighScore( FMISSION_SAVE* sv, FEST_MISSION_ID id )
{
  if( id >= FMISSION_ID_MAX ){
    return 0;
  }
  return sv->status[id].high_score;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコアをアップデートする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_UpdateMissionStatusHighScore( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 new_score )
{
  if( id >= FMISSION_ID_MAX || new_score > FMISSION_SAVE_HIGH_SCORE_MAX ){
    return;
  }
  if( sv->status[id].high_score < new_score ){
    sv->status[id].high_score;
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
    return;
  }
  sv->status[id].high_score = score;
}

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス レベルを返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
FMISSION_LEVEL FMISSION_SAVE_GetMissionStatusLevel( FMISSION_SAVE* sv, FEST_MISSION_ID id )
{
  if( id >= FMISSION_ID_MAX ){
    return 0;
  }
  return sv->status[id].level;
}
//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス レベルをセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
void FMISSION_SAVE_SetMissionStatusLevel( FMISSION_SAVE* sv, FEST_MISSION_ID id, FMISSION_LEVEL level )
{
  if( id >= FMISSION_ID_MAX ){
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
BOOL FMISSION_SAVE_SetMissionStatusNewFlag( FMISSION_SAVE* sv, FEST_MISSION_ID id )
{
  if( id >= FMISSION_ID_MAX ){
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
BOOL FMISSION_SAVE_ResetMissionStatusNewFlag( FMISSION_SAVE* sv, FEST_MISSION_ID id )
{
  if( id >= FMISSION_ID_MAX ){
    return;
  }
  sv->status[id].new_f = FALSE;
}


