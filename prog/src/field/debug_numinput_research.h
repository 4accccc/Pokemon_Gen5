////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  デバッグメニュー『数値入力』 ( すれ違い調査関連 )
 * @file   debug_numinput_research.c
 * @author obata
 * @date   2010.03.13
 */
//////////////////////////////////////////////////////////////////////////////// 
#pragma once
#include <gflib.h>
#include "gamesystem/gamesystem.h" // for GAMESYS_WORK
#include "gamesystem/game_data.h" // for GAMEDATA
#include "event_debug_numinput.h" // for DEBUG_NUMINPUT_XXXX

// @todo: 定数の定義を整理
///質問のバリエーション数
#define QUESTIONNAIRE_ITEM_NUM       (30)
///回答のバリエーション数(無回答を除く)
#define QUESTIONNAIRE_ANSWER_NUM     (150)

///本日の調査人数最大数
#define QUESTIONNAIRE_TODAY_COUNT_MAX   (999)
///今までの調査人数最大数
#define QUESTIONNAIRE_TOTAL_COUNT_MAX   (99999)

///質問毎の本日の調査人数最大数
#define QUESTIONNAIRE_TODAY_ANSWER_COUNT_MAX   (99)
///質問毎の今までの調査人数最大数
#define QUESTIONNAIRE_TOTAL_ANSWER_COUNT_MAX   (9999)

//------------------------------------------------------------------------------
/**
 * @brief 数値取得関数 ( 数値入力 → 今日のすれ違い調査人数 → Qx )
 *
 * @param gameSystem
 * @param gameData
 * @param questionID 取得する質問のID ( QUESTION_ID_xxxx )
 *
 * @return 質問に対する, 今日の回答人数
 */
//------------------------------------------------------------------------------
extern u32 DEBUG_GetTodayCountOfQuestion( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 questionID ); 
//------------------------------------------------------------------------------
/**
 * @brief 数値設定関数 ( 数値入力 → 今日のすれ違い調査人数 → Qx )
 *
 * @param gameSystem
 * @param gameData
 * @param questionID 設定する質問のID ( QUESTION_ID_xxxx )
 * @param count      設定する数値
 */
//------------------------------------------------------------------------------
extern void DEBUG_SetTodayCountOfQuestion( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 questionID, u32 count );
//------------------------------------------------------------------------------
/**
 * @brief 数値取得関数 ( 数値入力 → いままでのすれ違い調査人数 → Qx )
 *
 * @param gameSystem
 * @param gameData
 * @param questionID 取得する質問のID ( QUESTION_ID_xxxx )
 *
 * @return 質問に対する, いままでの回答人数
 */
//------------------------------------------------------------------------------
extern u32 DEBUG_GetTotalCountOfQuestion( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 questionID ); 
//------------------------------------------------------------------------------
/**
 * @brief 数値設定関数 ( 数値入力 → いままでのすれ違い調査人数 → Qx )
 *
 * @param gameSystem
 * @param gameData
 * @param questionID 設定する質問のID ( QUESTION_ID_xxxx )
 * @param count      設定する数値
 */
//------------------------------------------------------------------------------
extern void DEBUG_SetTotalCountOfQuestion( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 questionID, u32 count );
//------------------------------------------------------------------------------
/**
 * @brief 数値取得関数 ( 数値入力 → 今日のすれ違い回答人数 → Qx )
 *
 * @param gameSystem
 * @param gameData
 * @param answerID 取得する質問のID ( ANSWER_ID_xxxx )
 *
 * @return 質問に対する, 今日の回答人数
 */
//------------------------------------------------------------------------------
extern u32 DEBUG_GetTodayCountOfAnswer( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 answerID ); 
//------------------------------------------------------------------------------
/**
 * @brief 数値設定関数 ( 数値入力 → 今日のすれ違い回答人数 → Ax )
 *
 * @param gameSystem
 * @param gameData
 * @param answerID 設定する質問のID ( ANSWER_ID_xxxx )
 * @param count      設定する数値
 */
//------------------------------------------------------------------------------
extern void DEBUG_SetTodayCountOfAnswer( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 answerID, u32 count );
//------------------------------------------------------------------------------
/**
 * @brief 数値取得関数 ( 数値入力 → いままでのすれ違い回答人数 → Ax )
 *
 * @param gameSystem
 * @param gameData
 * @param answerID 取得する回答のID ( ANSWER_ID_xxxx )
 *
 * @return 質問に対する, いままでの回答人数
 */
//------------------------------------------------------------------------------
extern u32 DEBUG_GetTotalCountOfAnswer( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 answerID ); 
//------------------------------------------------------------------------------
/**
 * @brief 数値設定関数 ( 数値入力 → いままでのすれ違い回答人数 → Ax )
 *
 * @param gameSystem
 * @param gameData
 * @param answerID   設定する回答のID ( ANSWER_ID_xxxx )
 * @param count      設定する数値
 */
//------------------------------------------------------------------------------
extern void DEBUG_SetTotalCountOfAnswer( 
    GAMESYS_WORK* gameSystem, GAMEDATA* gameData, u32 answerID, u32 count );
