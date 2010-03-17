//=============================================================================================
/**
 * @file  hand_side.h
 * @brief ポケモンWB バトルシステム イベントファクター [サイドエフェクト]
 * @author  taya
 *
 * @date  2009.06.18  作成
 */
//=============================================================================================
#pragma once

#include "..\btl_pokeparam.h"
#include "..\btl_server_flow.h"


extern void BTL_HANDLER_SIDE_InitSystem( void );
extern BTL_EVENT_FACTOR*  BTL_HANDLER_SIDE_Add( BtlSide side, BtlSideEffect sideEffect, BPP_SICK_CONT contParam );
extern BOOL BTL_HANDLER_SIDE_Remove( BtlSide side, BtlSideEffect sideEffect );
extern BOOL BTL_HANDER_SIDE_IsExist( BtlSide side, BtlSideEffect effect );
extern u32 BTL_HANDLER_SIDE_GetAddCount( BtlSide side, BtlSideEffect effect );

/**
 *  ターンチェック時に効果が切れたエフェクトの種類＆サイドを通知するためのコールバック関数型
 */
typedef void (*pSideEffEndCallBack)( BtlSide side, BtlSideEffect sideEffect, void* arg );

extern void BTL_HANDLER_SIDE_TurnCheck( pSideEffEndCallBack callBack, void* callbackArg );

