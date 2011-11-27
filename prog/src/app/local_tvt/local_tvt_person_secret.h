//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_person_secret.h
 *	@brief  各人物処理：隠しキャラ
 *	@author	Toru=Nagihashi
 *	@date		2011.01.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "local_tvt_person.h"
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

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
typedef enum
{
  LOCAL_TVT_SECRET_MODE_RECIVE, //このキャラが電話を受けた
  LOCAL_TVT_SECRET_MODE_SEND,   //主人公から電話をかけた
}LOCAL_TVT_SECRET_MODE;
extern void LOCAL_TVT_SECRET_SetPerson( LOCAL_TVT_PERSON *p_person, const GFL_MSGDATA *cp_msg, GAMEDATA *p_gamedata, const LOCAL_TVT_CALLDATA *cp_data, LOCAL_TVT_SECRET_MODE mode, HEAPID heapID );

extern LOCAL_TVT_CHARA_TYPE LOCAL_TVT_SECRET_GetFace( const LOCAL_TVT_CALLDATA *cp_data );
extern u16 LOCAL_TVT_SECRET_GetDispName( const LOCAL_TVT_CALLDATA *cp_data );

extern u32 LOCAL_TVT_SECRET_GetCallSECnt( const LOCAL_TVT_CALLDATA *cp_data );
