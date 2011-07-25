//============================================================================================
/**
 * @file    fmission_save.h
 * @brief   フェスミッションセーブデータ
 * @author　Miyuki Iwasawa
 * @date    11.07.22
 */
//============================================================================================
#pragma once

#include "savedata/save_control.h"
#include "gamesystem/fmission_data_def.h"
#include "gamesystem/fmission_data_id.h"

///<外部参照型定義
typedef struct _FMISSION_SAVE FMISSION_SAVE;

#define FMISSION_SAVE_HIGH_SCORE_MAX  (9999)
#define FMISSION_SAVE_OPEN_NUM_MAX (0xFFFFFFFF)
#define FMISSION_SAVE_ENTRY_NUM_MAX (0xFFFFFFFF)
#define FMISSION_SAVE_CLEAR_NUM_MAX (0xFFFFFFFF)

//---------------------------------------------------------------------------
/**
 * @brief	ワークのサイズ取得
 * @return	int		サイズ
 */
//---------------------------------------------------------------------------
extern int FMISSION_SAVE_GetWorkSize(void);

//---------------------------------------------------------------------------
/**
 * @brief	ワークの初期化
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_Init( FMISSION_SAVE * wk );

//---------------------------------------------------------------------------
/**
 * @brief	セーブポインタを返す
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern FMISSION_SAVE* SAVEDATA_GetFMissionSave( SAVE_CONTROL_WORK* sv_ctrl );

//---------------------------------------------------------------------------
/**
 * @brief ミッション開催回数を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern u32 FMISSION_SAVE_GetMissionOpenNum( FMISSION_SAVE* sv );

//---------------------------------------------------------------------------
/**
 * @brief ミッション開催回数をインクリメントする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_IncMissionOpenNum( FMISSION_SAVE* sv );

//---------------------------------------------------------------------------
/**
 * @brief ミッション開催回数をセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_SetMissionOpenNum( FMISSION_SAVE* sv, u32 value );

//---------------------------------------------------------------------------
/**
 * @brief ミッション参加回数を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern u32 FMISSION_SAVE_GetMissionEntryNum( FMISSION_SAVE* sv );

//---------------------------------------------------------------------------
/**
 * @brief ミッション参加回数をインクリメントする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_IncMissionEntryNum( FMISSION_SAVE* sv );

//---------------------------------------------------------------------------
/**
 * @brief ミッション参加回数をセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_SetMissionEntryNum( FMISSION_SAVE* sv, u32 value );

//---------------------------------------------------------------------------
/**
 * @brief ミッション達成回数を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern u32 FMISSION_SAVE_GetMissionClearNum( FMISSION_SAVE* sv );

//---------------------------------------------------------------------------
/**
 * @brief ミッション達成回数をインクリメントする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_IncMissionClearNum( FMISSION_SAVE* sv );

//---------------------------------------------------------------------------
/**
 * @brief ミッション達成回数をセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_SetMissionClearNum( FMISSION_SAVE* sv, u32 value );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコアを返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern u16 FMISSION_SAVE_GetMissionStatusHighScore( FMISSION_SAVE* sv, FEST_MISSION_ID id );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコアをアップデートする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_UpdateMissionStatusHighScore( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 new_score );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコアをセット
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_SetMissionStatusHighScore( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 score );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス レベルを返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern FMISSION_LEVEL FMISSION_SAVE_GetMissionStatusLevel( FMISSION_SAVE* sv, FEST_MISSION_ID id );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス レベルをセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_SetMissionStatusLevel( FMISSION_SAVE* sv, FEST_MISSION_ID id, FMISSION_LEVEL level );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 新着フラグを返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern BOOL FMISSION_SAVE_CheckMissionStatusNewFlag( FMISSION_SAVE* sv, FEST_MISSION_ID id );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 新着フラグをセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern BOOL FMISSION_SAVE_SetMissionStatusNewFlag( FMISSION_SAVE* sv, FEST_MISSION_ID id );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 新着フラグをセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern BOOL FMISSION_SAVE_ResetMissionStatusNewFlag( FMISSION_SAVE* sv, FEST_MISSION_ID id );


