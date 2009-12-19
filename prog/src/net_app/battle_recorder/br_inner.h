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
	PLT_OBJ_M_COMMON	= 0,  //3本
  PLT_OBJ_M_POKEICON  = 4,  //3本

  PLT_OBJ_M_BTLSUBWAY_BP  = PLT_OBJ_M_POKEICON, //かぶらないので同じ箇所を使用  1本

  PLT_OBJ_M_PMS  = 7,
	PLT_OBJ_S_COMMON	= 0,  //3本
  PLT_OBJ_S_SPECIAL   = 4,

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


#include "../gds/gds_battle_rec.h"

//-------------------------------------
///	ランキングで必要な情報
//=====================================
#define BR_OUTLINE_RECV_MAX (30)
typedef struct 
{
	GT_BATTLE_REC_OUTLINE_RECV	data[ BR_OUTLINE_RECV_MAX ];			  ///< DL した概要一覧
	u8							            secure_flag[ BR_OUTLINE_RECV_MAX ];	///< DL した概要のフラグ
	int							            data_num;			      ///< DL した概要一覧の件数
} BR_OUTLINE_DATA;

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					  デバッグ用
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_
#ifdef PM_DEBUG
static inline void DEBUG_BR_OUTLINE_SetData( BR_OUTLINE_DATA *p_data )
{ 
  static const u16 * sc_debug_name[] =
  { 
    L"ホワイ",
    L"ブラク",
    L"ダイヤ",
    L"パール",
    L"ハート",
    L"ソウル",
  };

  int i;
  int j;


  for( i = 0; i < BR_OUTLINE_RECV_MAX; i++ )
  { 
    //名前
    for( j = 0 ; j < 3; j++ )
    { 
      p_data->data[i].profile.name[j]  = sc_debug_name[ i % NELEMS(sc_debug_name) ][j];
    }
    p_data->data[i].profile.name[j] = GFL_STR_GetEOMCode();

    //モンスター番号
    for( j = 0 ; j < GT_HEADER_MONSNO_MAX; j++ )
    {
      p_data->data[i].head.monsno[j]  = i+1;
      p_data->data[i].head.form_no[j] = 0;
    }
  }
  p_data->data_num  = i;
}
#endif //PM_DEBUG
