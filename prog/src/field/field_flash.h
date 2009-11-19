//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_flash.h
 *	@brief  フラッシュワーク
 *	@author	tomoya takahashi
 *	@date		2009.11.18
 *
 *	モジュール名：FIELD_FLASH
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	フラッシュ　状態
//=====================================
typedef enum 
{
  FIELD_FLASH_STATUS_NONE,      // 何もなし
  FIELD_FLASH_STATUS_NEAR,      // 視覚狭い
  FIELD_FLASH_STATUS_FADEOUT,   // フェードアウト中
  FIELD_FLASH_STATUS_FAR,       // 視覚広い
} FIELD_FLASH_STATUS;

//-------------------------------------
///	フラッシュ　リクエスト
//=====================================
typedef enum 
{
  FIELD_FLASH_REQ_ON_NEAR,    // 視覚狭いを設定
  FIELD_FLASH_REQ_FADEOUT,    // フェードアウト
  FIELD_FLASH_REQ_ON_FAR,     // 広い状態で設定
  FIELD_FLASH_REQ_OFF,        // 完全に消す


  FIELD_FLASH_REQ_MAX,  
} FIELD_FLASH_REQ;

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	フラッシュ管理ワーク
//=====================================
typedef struct _FIELD_FLASH FIELD_FLASH;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

// 生成　破棄
extern FIELD_FLASH* FIELD_FLASH_Create( HEAPID heapID );
extern void FIELD_FLASH_Delete( FIELD_FLASH* p_wk );

// 更新
extern void FIELD_FLASH_Update( FIELD_FLASH* p_wk );
extern void FIELD_FLASH_Draw( FIELD_FLASH* p_wk );

// ON OFF
extern void FIELD_FLASH_Control( FIELD_FLASH* p_wk, FIELD_FLASH_REQ req );

// 状態取得


#ifdef _cplusplus
}	// extern "C"{
#endif



