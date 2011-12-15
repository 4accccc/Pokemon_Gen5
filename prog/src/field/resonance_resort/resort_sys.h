//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		resort_sys.h
 *	@brief  リゾートシステム
 *	@author	Toru=Nagihashi
 *	@date		2011.07.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "field/resonance_resort_data.h"
#include "resort_binary.h"
#include "resort_data_manager.h"
#include "savedata/resonance_resort_sv.h"
#include "field/resort_work.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	リゾートシステム
//=====================================
typedef struct _RESORT_SYSTEM RESORT_SYSTEM;

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//システム作成・破棄
extern RESORT_SYSTEM * RESORT_SYSTEM_Init( const RESORT_ALL_DATA_PARAM *cp_param, RESORT_WORK *p_work, HEAPID heapID );
extern void RESORT_SYSTEM_Exit( RESORT_SYSTEM *p_wk );

//個別システム取得
extern RESORT_BINARY_SHOP * RESORT_SYSTEM_GetBinaryShop( RESORT_SYSTEM *p_wk );
extern RESORT_BINARY_SPOT * RESORT_SYSTEM_GetBinarySpot( RESORT_SYSTEM *p_wk );
extern RESORT_BINARY_CITY * RESORT_SYSTEM_GetBinaryCity( RESORT_SYSTEM *p_wk );
extern RESORT_BINARY_INSTITUTION * RESORT_SYSTEM_GetBinaryInstitution( RESORT_SYSTEM *p_wk );
extern RESORT_BINARY_PRESENT * RESORT_SYSTEM_GetBinaryPresent( RESORT_SYSTEM *p_wk );
extern RESORT_BINARY_PRESET * RESORT_SYSTEM_GetBinaryPreset( RESORT_SYSTEM *p_wk );
extern RESORT_BINARY_NPC * RESORT_SYSTEM_GetBinaryNpc( RESORT_SYSTEM *p_wk );

extern RESORT_ABSTRUCT_MANAGER * RESORT_SYSTEM_GetAbstructManager( RESORT_SYSTEM *p_wk );
extern RESORT_DATA_MANAGER * RESORT_SYSTEM_GetDataManager( RESORT_SYSTEM *p_wk );

//ワーク取得
extern RESORT_PEOPLE_ALL_DATA * RESORT_SYSTEM_GetPeopleAllData( RESORT_SYSTEM *p_wk );
extern RESORT_RECV_BUFF * RESORT_SYSTEM_GetRecvBuff( RESORT_SYSTEM *p_wk );
extern RESORT_RECV_BUFF * RESORT_SYSTEN_GetRecvTmpBuff( RESORT_SYSTEM *p_wk );
extern RESORT_PROXY_BUFF * RESORT_SYSTEM_GetProxyBuff( RESORT_SYSTEM *p_wk );
extern RESORT_CITY_DATA * RESORT_SYSTEM_GetCityData( RESORT_SYSTEM *p_wk );
extern RESORT_WORK * RESORT_SYSTEM_GetWork( RESORT_SYSTEM *p_wk );
