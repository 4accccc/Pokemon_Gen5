//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		scrcmd_overlay.h
 *	@brief  スクリプトコマンドのオーバーレイ管理
 *	        ゾーンIDに対応したオーバーレイを読み込みます
 *	@author	Toru=Nagihashi
 *	@date		2010.10.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "include\gamesystem\game_data.h"

//=============================================================================
/**
 *  外部公開
 */
//=============================================================================
//ゾーン切り替わり時をチェック
extern void SCRCMD_OVERLAY_CheckMapChange( GAMEDATA *p_gamedata, ZONEID zoneID );

//スクリプト用オーバーレイを読み込む
extern void SCRCMD_OVERLAY_SetUp( GAMEDATA  *p_gamedata );

//スクリプト用オーバーレイを解放
extern void SCRCMD_OVERLAY_CleanUp( GAMEDATA  *p_gamedata );

//読み込まれたオーバーレイに対応するスクリプトコマンドテーブルを取得
extern const VMCMD_FUNC *VMCMD_OVERLAY_GetTable( const GAMEDATA  *cp_gamedata );
extern u32 VMCMD_OVERLAY_GetTableMax( const GAMEDATA  *cp_gamedata );
