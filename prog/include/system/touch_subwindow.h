//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		touch_subwindow.h
 *	@brief		サブ画面　触れるウィンドウボタンシステム
 *	@author		tomoya takahashi
 *	@data		2006.03.23
 *				2009.01.15 GSより移植 Ariizumi
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifndef __TOUCH_SUBWINDOW_H__
#define __TOUCH_SUBWINDOW_H__

#include "system/palanm.h"

#undef GLOBAL
#ifdef	__TOUCH_SUBWINDOW_H_GLOBAL
#define	GLOBAL	/* */
#else
#define	GLOBAL	extern
#endif



//-----------------------------------------------------------------------------
/**
 *	【実装中の注意事項】
 *	
 *	VBlank期間中に
 *		gflib/bg_system.h
 *		GLOBAL void GF_BGL_VBlankFunc( GF_BGL_INI * ini );
 *	を呼んでください。	スクリーン転送を行います。
 */
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
//	システム動作　戻り値
//=====================================
enum{
	TOUCH_SW_RET_NORMAL,	// 何もなし
	TOUCH_SW_RET_YES,		// はい
	TOUCH_SW_RET_NO,		// いいえ
	TOUCH_SW_RET_YES_TOUCH,	// はい押された瞬間
	TOUCH_SW_RET_NO_TOUCH,	// いいえ押された瞬間
	TOUCH_SW_RET_NUM
};

enum{
	TOUCH_SW_TYPE_S,	///<スモール
	TOUCH_SW_TYPE_L,	///<ラージ
};

// パレット転送サイズ
#define TOUCH_SW_USE_PLTT_NUM	(2)			// パレット本数単位
// キャラクタデータサイズ
#define TOUCH_SW_USE_CHAR_NUM	(15*4)		// キャラクタ単位
// スクリーンサイズ
#define TOUCH_SW_USE_SCRN_SX		(6)			// キャラクタ単位
#define TOUCH_SW_USE_SCRN_SY		(4)			// キャラクタ単位
#define TOUCH_SW_USE_SCRN_LX		(16)		// キャラクタ単位
#define TOUCH_SW_USE_SCRN_LY		(6)			// キャラクタ単位

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
//	システム初期化データ
//=====================================
typedef struct {
	u32 bg_frame;			// BGナンバー
	u32 char_offs;			// キャラクタ転送オフセット (ｷｬﾗｸﾀ単位 1/32byte)
	u32 pltt_offs;			// パレット転送オフセット（ﾊﾟﾚｯﾄ1本分単位 1/32byte）
							// ２本分使用するので注意！pltt_offsとpltt_offs+1が潰れます

	u8 x;					// x座標（ｷｬﾗｸﾀ単位）
	u8 y;					// y座標（ｷｬﾗｸﾀ単位）
	u8 kt_st:4;				// キーorタッチのステータス
	u8 key_pos:4;			// キーの初期カレント
	u8 type;				// 表示タイプ(TOUCH_SW_TYPE_S,TOUCH_SW_TYPE_L)
} TOUCH_SW_PARAM;


//-------------------------------------
//	システムワーク
//=====================================
typedef struct _TOUCH_SW_SYS TOUCH_SW_SYS;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	システムワーク作成
 *	
 *	@param	heapid	使用ヒープID 
 *	
 *	@return	ワークポインタ
 */
//-----------------------------------------------------------------------------
GLOBAL TOUCH_SW_SYS* TOUCH_SW_AllocWork( u32 heapid );

//----------------------------------------------------------------------------
/**
 *	@brief	システムワーク破棄
 *	
 *	@param	p_touch_sw	システムワーク
 */
//-----------------------------------------------------------------------------
GLOBAL void TOUCH_SW_FreeWork( TOUCH_SW_SYS* p_touch_sw );

//----------------------------------------------------------------------------
/**
 *	@brief	システム初期化
 *
 *	@param	p_touch_sw	システムワーク
 *	@param	cp_param	システム動作データ
 */
//-----------------------------------------------------------------------------
GLOBAL void TOUCH_SW_Init( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param );

//----------------------------------------------------------------------------
/**
 *	@brief	システム初期化EX パレットアニメワーク初期化処理込み
 *
 *	@param	p_touch_sw	システムワーク
 *	@param	cp_param	システム動作データ
 *	@param	palASys		パレットアニメシステムワーク
 */
//-----------------------------------------------------------------------------
GLOBAL void TOUCH_SW_InitEx( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param,PALETTE_FADE_PTR palASys);

//----------------------------------------------------------------------------
/**
 *	@brief	システムメイン動作
 *
 *	@param	p_touch_sw	システムワーク
 *
 *	@retval	TOUCH_SW_RET_NORMAL	// 何もなし
 *	@retval	TOUCH_SW_RET_YES	// はい
 *	@retval	TOUCH_SW_RET_NO		// いいえ
 *
 *	TOUCH_SW_MainMCのとき以下の値も帰ってきます。(MC=Moment Check)
 *	@retval	TOUCH_SW_RET_YES_TOUCH	// はい押した瞬間
 *	@retval	TOUCH_SW_RET_NO_TOUCH	// いいえ押した瞬間
 *	
 */
//-----------------------------------------------------------------------------
GLOBAL u32 TOUCH_SW_Main( TOUCH_SW_SYS* p_touch_sw );
GLOBAL u32 TOUCH_SW_MainMC( TOUCH_SW_SYS* p_touch_sw );

//----------------------------------------------------------------------------
/**
 *	@brief	システムデータリセット	(Initの前の状態にする　いらないかもしれない)
 *
 *	@param	p_touch_sw	システムワーク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
GLOBAL void TOUCH_SW_Reset( TOUCH_SW_SYS* p_touch_sw );

//----------------------------------------------------------------------------
/**
 *	@brief	現在の操作モードを取得(システムデータリセット前に取得すること)
 *
 *	@param	p_touch_sw	システムワーク
 *
 *	@retval APP_END_KEY
 *	@retval APP_END_TOUCH	
 */
//-----------------------------------------------------------------------------
GLOBAL TOUCH_SW_GetKTStatus( TOUCH_SW_SYS* p_touch_sw );

#undef	GLOBAL
#endif		// __TOUCH_SUBWINDOW_H__

