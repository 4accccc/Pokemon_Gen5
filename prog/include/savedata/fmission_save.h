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

typedef enum{
  FMISSION_SV_EXP_WHITE, ///<ホワイトLv経験値
  FMISSION_SV_EXP_BLACK, ///<ブラックLv経験値
  FMISSION_SV_EXP_IDX_MAX,
}FMISSION_SV_EXP_IDX;

#define FMISSION_SV_HILINK_LEVEL_EXP_MAX  (1000)  //経験値max値
#define FMISSION_SV_HILINK_LEVEL_EXP_STEP (100)   //経験値いくつごとにレベルアップさせるかの閾値
#define FMISSION_SV_HILINK_LEVEL_UP_ONCE_MAX  (10)  //一度のミッションで上がるレベル上限値

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
extern u32 FMISSION_SAVE_IncMissionOpenNum( FMISSION_SAVE* sv );

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
extern u32 FMISSION_SAVE_IncMissionEntryNum( FMISSION_SAVE* sv );

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
 * @brief ミッション最高参加者数を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern u8 FMISSION_SAVE_GetMissionMaxJoinNum( FMISSION_SAVE* sv );

//---------------------------------------------------------------------------
/**
 * @brief ミッション最高参加者数をアップデートする	
 * @param	work		ワークへのポインタ
 *
 * @param entry_num 参加人数、記録より少なければ更新しない
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_UpdateMissionMaxJoinNum( FMISSION_SAVE* sv, u8 join_num );

//---------------------------------------------------------------------------
/**
 * @brief ミッション最高参加者数をセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_SetMissionMaxJoinNum( FMISSION_SAVE* sv, u16 join_num );

//---------------------------------------------------------------------------
/**
 * @brief ミッショントップスコアを取った回数を返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern u32 FMISSION_SAVE_GetMissionTopScoreNum( FMISSION_SAVE* sv );

//---------------------------------------------------------------------------
/**
 * @brief ミッショントップスコアを取った回数をインクリメントする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_IncMissionTopScoreNum( FMISSION_SAVE* sv );

//---------------------------------------------------------------------------
/**
 * @brief ミッショントップスコアを取った回数をセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_SetMissionTopScoreNum( FMISSION_SAVE* sv, u16 top_num );

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
extern BOOL FMISSION_SAVE_UpdateMissionStatusHighScore( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 new_score );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス ハイスコアをセット
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_SetMissionStatusHighScore( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 score );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 自分のハイスコアを返す	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern u16 FMISSION_SAVE_GetMissionStatusHighScoreMine( FMISSION_SAVE* sv, FEST_MISSION_ID id );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 自分のハイスコアをアップデートする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern BOOL FMISSION_SAVE_UpdateMissionStatusHighScoreMine( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 new_score );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 自分のハイスコアをセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_SetMissionStatusHighScoreMine( FMISSION_SAVE* sv, FEST_MISSION_ID id, u32 score );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス クリアしたレベルを返す	
 * @param	work		ワークへのポインタ
 *
 * @retval  0 未クリア
 * @retval  1～4  各レベルまでをクリアした
 */
//---------------------------------------------------------------------------
extern FMISSION_CLEAR_LEVEL FMISSION_SAVE_GetMissionStatusClearLevel( FMISSION_SAVE* sv, FEST_MISSION_ID id );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス レベルをセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_SetMissionStatusClearLevel( FMISSION_SAVE* sv, FEST_MISSION_ID id, FMISSION_CLEAR_LEVEL level );

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
extern void FMISSION_SAVE_SetMissionStatusNewFlag( FMISSION_SAVE* sv, FEST_MISSION_ID id );

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 新着フラグをリセットする	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_ResetMissionStatusNewFlag( FMISSION_SAVE* sv, FEST_MISSION_ID id );

//---------------------------------------------------------------------------
/**
 * @brief ミッション　ハイリンクレベル経験値取得	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern u16 FMISSION_SAVE_GetHilinkLevelExp( FMISSION_SAVE* sv, FMISSION_SV_EXP_IDX version_idx );

//---------------------------------------------------------------------------
/**
 * @brief ミッション　ハイリンクレベル経験値セット	
 * @param	work		ワークへのポインタ
 */
//---------------------------------------------------------------------------
extern void FMISSION_SAVE_SetHilinkLevelExp( FMISSION_SAVE* sv, FMISSION_SV_EXP_IDX version_idx, u16 value );


#ifdef PM_DEBUG

//---------------------------------------------------------------------------
/**
 * @brief ミッションステータス 全ミッションのクリアレベルをセット
 * @param	work		ワークへのポインタ
 *
 * @param   FMISSION_CLEAR_LEVEL_0-FMISSION_LEVEL_4
 */
//---------------------------------------------------------------------------
extern void DEBUG_FMISSION_SV_SetMissionStatusClearLevelAll( FMISSION_SAVE* sv,  FMISSION_CLEAR_LEVEL level );

#endif  //PM_DEBUG

