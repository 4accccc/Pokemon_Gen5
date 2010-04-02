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
