/////////////////////////////////////////////////////////////////////
/**
 * @brief  すれ違い調査隊
 * @file   research_team.h
 * @author obata
 * @date   2010.03.31
 */
/////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "savedata/save_control.h"
#include "savedata/questionnaire_save.h" 

// 調査タイプを取得する
extern int RESEARCH_TEAM_GetResearchType( u8 requestID );
// 質問数を取得する
extern int RESEARCH_TEAM_GetQuestionNum( u8 requestID );
// 質問IDを取得する
extern void RESEARCH_TEAM_GetQuestionID( u8 requestID, u8* destQID );
// ノルマ人数を取得する
extern int RESEARCH_TEAM_GetNormCount( u8 requestID );
// ノルマ時間を取得する
extern int RESEARCH_TEAM_GetNormTime( u8 requestID );

//調査中の依頼の達成をチェックする(受けた依頼の達成チェックに使用)
extern u16 RESEARCH_TEAM_CheckCurrentResearchFinish( SAVE_CONTROL_WORK* save );

//指定した依頼をすでに達成しているかどうかをチェックする(依頼を受けようとした際の事前チェックに使用)
extern u16 RESEARCH_TEAM_CheckRequestFinishInAdvance( SAVE_CONTROL_WORK* save, u8 RID );

//調査中の依頼の経過時間を取得する
extern u16 RESEARCH_TEAM_GetPassedTime( SAVE_CONTROL_WORK* save );

//現在の調査人数を取得する
extern u32 RESEARCH_TEAM_GetCurrentCount( QUESTIONNAIRE_SAVE_WORK* QSave, u8 QID );

