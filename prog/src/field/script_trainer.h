//======================================================================
/**
 * @file	script_trainer.h
 * @brief	スクリプト・トレーナー関連
 * @author	tamada GAMEFREAK inc.
 * @date	2010.03.15
 *
 */
//======================================================================
#pragma once

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"
#include "eventwork.h"

#include "fldmmdl.h"    //MMDL

//--------------------------------------------------------------
/**
 * スクリプトIDから、トレーナーIDを取得
 *
 * @param   scr_id		スクリプトID
 *
 * @retval  "トレーナーID = フラグインデックス"
 */
//--------------------------------------------------------------
extern u16 SCRIPT_GetTrainerID_ByScriptID( u16 scr_id );

//--------------------------------------------------------------
/**
 * スクリプトIDから、左右どちらのトレーナーか取得
 *
 * @param   scr_id		スクリプトID
 *
 * @retval  "0=左、1=右"
 */
//--------------------------------------------------------------
extern BOOL SCRIPT_GetTrainerLR_ByScriptID( u16 scr_id );

//--------------------------------------------------------------
/**
 * トレーナーIDから、ダブルバトルタイプか取得
 *
 * @param   tr_id		トレーナーID
 *
 * @retval  "0=シングルバトル、1=ダブルバトル"
 */
//--------------------------------------------------------------
extern BOOL SCRIPT_CheckTrainer2vs2Type( u16 tr_id );

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをチェックする
 *
 * @param	tr_id		トレーナーID
 *
 * @retval	"1 = フラグON"
 * @retval	"0 = フラグOFF"
 */
//------------------------------------------------------------------
extern BOOL SCRIPT_CheckEventFlagTrainer( EVENTWORK *ev, u16 tr_id );

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをセットする
 *
 * @param	tr_id		トレーナーID
 *
 * @return	none
 */
//------------------------------------------------------------------
extern void SCRIPT_SetEventFlagTrainer( EVENTWORK *ev, u16 tr_id );

//------------------------------------------------------------------
/**
 * @brief	トレーナーイベントフラグをリセットする
 *
 * @param	tr_id		トレーナーID
 *
 * @return	none
 */
//------------------------------------------------------------------
extern void SCRIPT_ResetEventFlagTrainer( EVENTWORK *ev, u16 tr_id );


extern void SCRIPT_SetTrainerEyeData( GMEVENT *event, MMDL *mmdl,
    s16 range, u16 dir, u16 scr_id, u16 tr_id, int tr_type, int tr_no );

extern void * SCRIPT_GetTrainerEyeData( SCRIPT_WORK * sc, u32 tr_no );


