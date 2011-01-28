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

typedef enum
{
  LOCAL_TVT_SECRET_FACE_INVISIBLE,//映らない
  LOCAL_TVT_SECRET_FACE_HIDE_M,     //男：顔を隠している
  LOCAL_TVT_SECRET_FACE_VISIBLE_M,  //男：素顔
  LOCAL_TVT_SECRET_FACE_HIDE_F,     //女：顔を隠している
  LOCAL_TVT_SECRET_FACE_VISIBLE_F,  //女：素顔
}LOCAL_TVT_SECRET_FACE;
extern LOCAL_TVT_SECRET_FACE LOCAL_TVT_SECRET_GetFace( const LOCAL_TVT_CALLDATA *cp_data );
