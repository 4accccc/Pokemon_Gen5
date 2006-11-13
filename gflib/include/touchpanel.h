//*****************************************************************************
/**
 *
 *@file		touchpanel.h
 *@brief	タッチパネルデータ処理
 *@author	tomoya takahashi   >  katsumi ohno
 *@data		2005.03.16
 */
//*****************************************************************************

#ifndef __TOUCHPANEL_H__
#define	__TOUCHPANEL_H__

#undef	GLOBAL
#ifdef	__TOUCHPANEL_H_GLOBAL__
#define	GLOBAL	/* */
#else
#define	GLOBAL	extern
#endif

//-----------------------------------------------------------------------------
/**
 *					定数宣言
 */
//-----------------------------------------------------------------------------

#define		TP_HIT_END		(0xff)			// テーブル終了コード
#define		TP_USE_CIRCLE	(0xfe)			// 円形として使う
#define		TP_HIT_NONE		(-1)	// あたったテーブルなし

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
 */
//-----------------------------------------------------------------------------
//-------------------------------------
//	矩形データ構造体
//=====================================
typedef union{
	struct {
		u8	top;	// 上（もしくは特殊コード）
		u8	bottom;	// 下
		u8	left;	// 左
		u8	right;	// 右
	}rect;

	struct {
		u8	code;	// TP_USE_CIRCLE を指定
		u8	x;
		u8	y;
		u8	r;
	}circle;

}RECT_HIT_TBL, TP_HIT_TBL;

/*
 * 使用例
 *
 * RECT_HIT_TBL rect_data[] ={
 *		{0,64,0,128},				// 配列要素優先順位:0>1>2>3>4>.....です
 *		{128,191,0,64},
 *		{32,94,129,200},
 *		{0,191,0,255},
 *      {TP_USE_CIRCLE, 100, 80, 16 },	// 円形として使う。
 *		{TP_HIT_END,0,0,0},		// 終了データ
 * };
 * 
 */

//------------------------------------------------------------------
/**
 * @brief	TPSYS 型宣言
 *
 * TCBシステムワーク構造体の宣言。
 * メイン処理用・VBlank用など、任意の箇所で実行される
 * 複数のTCBシステムを作成することが出来る。
 *
 * 内容は隠蔽されており、直接のアクセスはできない。
  */
//------------------------------------------------------------------
typedef struct _TPSYS		TPSYS;


//----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
 */
//----------------------------------------------------------------------------

GLOBAL int GF_TP_HitCont( const TPSYS* tpsys, const TP_HIT_TBL *tbl );
GLOBAL int GF_TP_HitTrg( const TPSYS* tpsys, const TP_HIT_TBL *tbl );

GLOBAL BOOL GF_TP_GetCont( const TPSYS* tpsys );
GLOBAL BOOL GF_TP_GetTrg( const TPSYS* tpsys );

GLOBAL int GF_TP_HitSelf( const TP_HIT_TBL *tbl, u32 x, u32 y );

GLOBAL BOOL GF_TP_GetPointCont( const TPSYS* tpsys, u32* x, u32* y );
GLOBAL BOOL GF_TP_GetPointTrg( const TPSYS* tpsys, u32* x, u32* y );


#undef	GLOBAL
#endif

