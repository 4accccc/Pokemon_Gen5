//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_inner.h
 *	@brief	バトルレコーダー	プライベートヘッダ
 *	@author	Toru=Nagihashi
 *	@date		2009.11.11
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	BG面設定
//=====================================
enum 
{
  BG_FRAME_M_TEXT = GFL_BG_FRAME0_M,
	BG_FRAME_M_FONT	= GFL_BG_FRAME1_M,
	BG_FRAME_M_WIN	= GFL_BG_FRAME2_M,
	BG_FRAME_M_BACK	= GFL_BG_FRAME3_M,
	BG_FRAME_S_FONT	= GFL_BG_FRAME0_S,
	BG_FRAME_S_WIN	= GFL_BG_FRAME1_S,
	BG_FRAME_S_BACK	= GFL_BG_FRAME2_S,
};

//-------------------------------------
///	PLT設定
//=====================================
enum 
{
	//BG
	PLT_BG_M_COMMON	= 0,
	PLT_BG_S_COMMON	= 0,
  PLT_BG_M_FONT	= 14,
  PLT_BG_S_FONT	= 14,

	//PLT
	PLT_OBJ_M_COMMON	= 0,
	PLT_OBJ_S_COMMON	= 0,
};

//-------------------------------------
///	文字色（BMPWINフォント）
//=====================================
#define BR_PRINT_COL_BOOT	  	( PRINTSYS_LSB_Make(  1, 14,  0 ) )	// 起動時の文字色
#define BR_PRINT_COL_NORMAL 	( PRINTSYS_LSB_Make( 15, 13,  0 ) )	// よく使う文字色
#define	BR_PRINT_COL_BLACK		( PRINTSYS_LSB_Make( 14, 13,  0 ) )	// フォントカラー
#define	BR_PRINT_COL_INFO		  ( PRINTSYS_LSB_Make( 15, 13, 12 ) )	// フォントカラー
#define	BR_PRINT_COL_PHOTO		( BR_PRINT_COL_NORMAL )
#define	BR_PRINT_COL_BOX_SHOT	( BR_PRINT_COL_NORMAL )
#define	BR_PRINT_COL_BOX_NAME	( PRINTSYS_LSB_Make( 15,  1,  0 ) )
#define	BR_PRINT_COL_FREC		  ( BR_PRINT_COL_NORMAL )
#define	BR_PRINT_COL_PROF		  ( BR_PRINT_COL_NORMAL )


//-------------------------------------
///戦闘録画データ
//=====================================
typedef struct 
{
	BOOL						is_valid;
	void						*p_btlrec;
} BR_BTLREC_DATA;
typedef struct 
{
	BR_BTLREC_DATA	my;
	BR_BTLREC_DATA	other[3];
} BR_BTLREC_SET;

