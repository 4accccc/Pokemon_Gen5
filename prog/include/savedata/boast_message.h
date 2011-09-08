//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		\home\swan\prog\include\savedata\boast_message.h
 *	@brief  すれ違い 自慢メッセージ
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.09.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include <gflib.h>
#include "savedata/save_control.h"	//SAVE_CONTROL_WORK
#include "gamesystem/gamedata_def.h"	//SAVE_CONTROL_WORK

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

// 不完全型
typedef struct _BOAST_MSG_SAVEDATA BOAST_MSG_SAVEDATA;

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
//----------------------------------------------------------
//セーブデータシステムに依存する関数
//----------------------------------------------------------
extern int BOAST_MSG_SAVEDATA_GetWorkSize( void );
extern void BOAST_MSG_SAVEDATA_Init( BOAST_MSG_SAVEDATA *p_boast_msg_sv );

//----------------------------------------------------------
//セーブデータ取得のための関数
//----------------------------------------------------------
extern const BOAST_MSG_SAVEDATA * BOAST_MSG_SAVEDATA_GetSaveDataConst( const SAVE_CONTROL_WORK * cp_sv );
extern BOAST_MSG_SAVEDATA * BOAST_MSG_SAVEDATA_GetSaveData( SAVE_CONTROL_WORK * p_sv );

//----------------------------------------------------------
//メッセージ取得・設定
//----------------------------------------------------------
extern void BOAST_MSG_SAVEDATA_SetMsg( BOAST_MSG_SAVEDATA *p_sv, const STRBUF *srcbuf );
extern const STRCODE * BOAST_MSG_SAVEDATA_GetMsg( const BOAST_MSG_SAVEDATA *cp_sv );


