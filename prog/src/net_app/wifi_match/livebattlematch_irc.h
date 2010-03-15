//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		livebattlematch_flow.h
 *	@brief  ライブ大会赤外線通信モジュール
 *	@author	Toru=Nagihashi
 *	@data		2010.03.07
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "gamesystem/game_data.h"
#include "wifibattlematch_data.h"
#include "poke_tool/pokeparty.h"
#include "savedata/regulation.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define LIVEBATTLEMATCH_IRC_CONNECT_NUM (2)

//-------------------------------------
///	エラー結果
//=====================================
typedef enum
{
  LIVEBATTLEMATCH_IRC_SYSERROR_NONE,  //エラーは発生していない
} LIVEBATTLEMATCH_IRC_SYSERROR;

//-------------------------------------
///	エラー解決タイプ
//=====================================
typedef enum
{
  LIVEBATTLEMATCH_IRC_ERROR_REPAIR_NONE,  //正常
  LIVEBATTLEMATCH_IRC_ERROR_REPAIR_DISCONNECT,  //切断
} LIVEBATTLEMATCH_IRC_ERROR_REPAIR_TYPE;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	赤外線モジュール
//=====================================
typedef struct _LIVEBATTLEMATCH_IRC_WORK LIVEBATTLEMATCH_IRC_WORK;

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//-------------------------------------
///	ワーク作成・破棄・処理
//=====================================
extern LIVEBATTLEMATCH_IRC_WORK *LIVEBATTLEMATCH_IRC_Init( GAMEDATA *p_gamedata, HEAPID heapID );
extern void LIVEBATTLEMATCH_IRC_Exit( LIVEBATTLEMATCH_IRC_WORK *p_wk );
extern void LIVEBATTLEMATCH_IRC_Main( LIVEBATTLEMATCH_IRC_WORK *p_wk );

//-------------------------------------
///	エラー
//=====================================
extern LIVEBATTLEMATCH_IRC_ERROR_REPAIR_TYPE LIVEBATTLEMATCH_IRC_CheckErrorRepairType( LIVEBATTLEMATCH_IRC_WORK *p_wk );

//-------------------------------------
///	接続・切断
//=====================================
//繋がる
extern BOOL LIVEBATTLEMATCH_IRC_StartConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk );
extern BOOL LIVEBATTLEMATCH_IRC_WaitConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk );
//接続キャンセル
extern void LIVEBATTLEMATCH_IRC_StartCancelConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk );
extern BOOL LIVEBATTLEMATCH_IRC_WaitCancelConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk );
//切断する
extern BOOL LIVEBATTLEMATCH_IRC_StartDisConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk );
extern BOOL LIVEBATTLEMATCH_IRC_WaitDisConnect( LIVEBATTLEMATCH_IRC_WORK *p_wk );


//-------------------------------------
///	相手のデータ受信  お互いのデータを送る  SENDDATA系
//=====================================
//お互いの情報を送りあう
extern BOOL LIVEBATTLEMATCH_IRC_StartEnemyData( LIVEBATTLEMATCH_IRC_WORK *p_wk, const void *cp_buff );
//なぜ＊＊を渡すかというと、WIFIBATTLEMATCH_ENEMYDATAはフレキシブルな構造体なのでポインタを受け取り、外でMemCopyしてもらうためです。
extern BOOL LIVEBATTLEMATCH_IRC_WaitEnemyData( LIVEBATTLEMATCH_IRC_WORK *p_wk, WIFIBATTLEMATCH_ENEMYDATA **pp_data );

//ポケパーテイを送りあう
extern BOOL LIVEBATTLEMATCH_IRC_SendPokeParty( LIVEBATTLEMATCH_IRC_WORK *p_wk, const POKEPARTY *cp_party );
extern BOOL LIVEBATTLEMATCH_IRC_RecvPokeParty( LIVEBATTLEMATCH_IRC_WORK *p_wk, POKEPARTY *p_party );


//-------------------------------------
///	レギュレーション配信
//=====================================
//レギュレーションを受け取る
extern void LIVEBATTLEMATCH_IRC_StartRecvRegulation( LIVEBATTLEMATCH_IRC_WORK *p_wk, REGULATION_CARDDATA *p_recv );

typedef enum
{ 
  LIVEBATTLEMATCH_IRC_RET_UPDATE,  //処理中
  LIVEBATTLEMATCH_IRC_RET_SUCCESS,  //正常に受信
  LIVEBATTLEMATCH_IRC_RET_FAILED,   //不正なデータを受け取った
  LIVEBATTLEMATCH_IRC_RET_NOT_LAG,  //自分の言語のレギュレーションがなかった
}LIVEBATTLEMATCH_IRC_RET;
extern LIVEBATTLEMATCH_IRC_RET LIVEBATTLEMATCH_IRC_WaitRecvRegulation( LIVEBATTLEMATCH_IRC_WORK *p_wk );

extern void LIVEBATTLEMATCH_IRC_StartCancelRecvRegulation( LIVEBATTLEMATCH_IRC_WORK *p_wk );
extern BOOL LIVEBATTLEMATCH_IRC_WaitCancelRecvRegulation( LIVEBATTLEMATCH_IRC_WORK *p_wk );
