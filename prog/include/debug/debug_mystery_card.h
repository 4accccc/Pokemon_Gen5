//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_mystery_card.h
 *	@brief  デバッグ用ふしぎなおくりもの作成  
 *	@author	Toru=Nagihashi
 *	@date		2010.02.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//セーブデータ
#include "savedata/mystery_data.h"

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

//ポケモンデータ作成
extern void DEBUG_MYSTERY_SetGiftPokeData( GIFT_PACK_DATA *p_data );

//どうぐデータ作成
extern void DEBUG_MYSTERY_SetGiftItemData( GIFT_PACK_DATA *p_data );

//GPOWERデータ作成
extern void DEBUG_MYSTERY_SetGiftGPowerData( GIFT_PACK_DATA *p_data );

//実データ以外のものをセット
extern void DEBUG_MYSTERY_SetGiftCommonData( GIFT_PACK_DATA *p_data, u32 event_id, BOOL only_one_flag );

//ダウンロードデータを設定
extern void DEBUG_MYSTERY_SetDownLoadData( DOWNLOAD_GIFT_DATA *p_data, u32 version, u8 langCode );
