//-----------------------------------------------------------------------------
/**
 *      スクリプト向け
 *        WFBC情報取得　データインデックス
 */
//-----------------------------------------------------------------------------



#pragma once

#define WFBC_GET_PARAM_BATTLE_TRAINER_ID (0)    // バトルトレーナーID
#define WFBC_GET_PARAM_IS_TAKES_ID (1)          // 街にいきたがるか
#define WFBC_GET_PARAM_IS_RIREKI (2)          // 前にいたBCの街の履歴があるか
#define WFBC_GET_PARAM_PEOPLE_NUM (3)         // 人口の取得 **zone_chnageのタイミングで呼んでOK**
#define WFBC_GET_PARAM_BC_NPC_WIN_NUM (4)     // BC　NPC勝利数
#define WFBC_GET_PARAM_BC_NPC_WIN_TARGET (5)  // BC　NPC勝利目標数
#define WFBC_GET_PARAM_WF_IS_POKECATCH (6)  // WF　村長イベント　ポケモンはいるか？
#define WFBC_GET_PARAM_WF_ITEM  (7)         // WF 村長イベント　ご褒美アイテムの取得
#define WFBC_GET_PARAM_WF_POKE  (8)         // WF 村長イベント　探すポケモンナンバーの取得
#define WFBC_GET_PARAM_OBJ_ID  (9)         // 話し相手のOBJID　（0～49）
#define WFBC_GET_PARAM_BC_CHECK_BATTLE (10)  // BC　相手とバトル可能かチェック

#define WFBC_GET_PARAM_MAX  (11)         // 最大値
