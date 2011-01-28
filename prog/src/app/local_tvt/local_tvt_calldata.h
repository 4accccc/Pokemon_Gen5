//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_calldata.h
 *	@brief  呼び出しデータ
 *	@author	Toru=Nagihashi
 *	@date		2011.01.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//コンバータから吐き出されるデータ定義
#include "local_tvt_call.h"
#include "savedata/person_state.h"
//関連情報
#include "local_tvt_type_def.h"
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
///	呼び出しデータワーク
//=====================================
typedef struct _LOCAL_TVT_CALLDATA LOCAL_TVT_CALLDATA;

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
extern LOCAL_TVT_CALLDATA * LOCAL_TVT_CALLDATA_Init( GAMEDATA *p_gamedata, HEAPID heapID );
extern void LOCAL_TVT_CALLDATA_Exit( LOCAL_TVT_CALLDATA *p_wk );

extern LOCAL_TVT_BACK LOCAL_TVT_CALLDATA_GetBack( const LOCAL_TVT_CALLDATA *cp_wk, u16 person );
extern LOCAL_TVT_WEATHER LOCAL_TVT_CALLDATA_GetWeather( const LOCAL_TVT_CALLDATA *cp_wk, u16 person );
extern u16 LOCAL_TVT_CALLDATA_GetGreetingMsgID( const LOCAL_TVT_CALLDATA *cp_wk, u16 person );
extern u16 LOCAL_TVT_CALLDATA_GetTalkMsgID( const LOCAL_TVT_CALLDATA *cp_wk, u16 person );
extern u16 LOCAL_TVT_CALLDATA_GetFarewellMsgID( const LOCAL_TVT_CALLDATA *cp_wk, u16 person );
extern u16 LOCAL_TVT_CALLDATA_GetSpecialMsgID( const LOCAL_TVT_CALLDATA *cp_wk, u16 person );
extern BOOL LOCAL_TVT_CALLDATA_IsCall( const LOCAL_TVT_CALLDATA *cp_wk, u16 person );
extern BOOL LOCAL_TVT_CALLDATA_IsSelect( const LOCAL_TVT_CALLDATA *cp_wk, u16 person );

extern LOCAL_TVT_BACK LOCAL_TVT_CALLDATA_GetBackByZoneID( const LOCAL_TVT_CALLDATA *cp_wk, ZONEID zoneID );
extern LOCAL_TVT_WEATHER LOCAL_TVT_CALLDATA_GetWeatherByZoneID( const LOCAL_TVT_CALLDATA *cp_wk, ZONEID zoneID );

//
extern u16 LOCAL_TVT_CALLDATA_GetState( const LOCAL_TVT_CALLDATA *cp_wk, u16 person );
extern void LOCAL_TVT_CALLDATA_AddState( LOCAL_TVT_CALLDATA *p_wk, u16 person, u16 max );

//隠しキャラの状態取得
extern u32 LOCAL_TVT_CALLDATA_GetSecretSex( const LOCAL_TVT_CALLDATA *cp_wk );
extern BOOL LOCAL_TVT_CALLDATA_GetSecretDispleased( const LOCAL_TVT_CALLDATA *cp_wk );
extern void LOCAL_TVT_CALLDATA_SetSecretDispleased( LOCAL_TVT_CALLDATA *p_wk, BOOL enable );
extern void LOCAL_TVT_CALLDATA_SetSecretDateEvent( LOCAL_TVT_CALLDATA *p_wk, PERSON_STATE_SECRET_DATE type );
extern BOOL LOCAL_TVT_CALLDATA_GetSecretSeasonDate( const LOCAL_TVT_CALLDATA *cp_wk );
extern void LOCAL_TVT_CALLDATA_SetSecretOnceCall( LOCAL_TVT_CALLDATA *p_wk );
