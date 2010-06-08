//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_util.h
 *	@brief  各PROCで使う共通モジュール系
 *	@author	Toru=Nagihashi
 *	@date		2009.11.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "print/gf_font.h"
#include "print/printsys.h"
#include "system/bmp_menuwork.h"

#include "br_res.h"

#include "savedata/gds_profile.h"

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					BMPWINメッセージ描画構造体
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	メッセージウィンドウ
//=====================================
typedef struct _BR_MSGWIN_WORK BR_MSGWIN_WORK;
//-------------------------------------
///	パブリック
//=====================================
extern BR_MSGWIN_WORK * BR_MSGWIN_Init( u16 frm, u8 x, u8 y, u8 w, u8 h, u8 plt, PRINT_QUE *p_que, HEAPID heapID );
extern void BR_MSGWIN_Exit( BR_MSGWIN_WORK* p_wk );
extern void BR_MSGWIN_Print( BR_MSGWIN_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, GFL_FONT *p_font );
extern void BR_MSGWIN_PrintBuf( BR_MSGWIN_WORK* p_wk, const STRBUF *cp_strbuf, GFL_FONT *p_font );
extern void BR_MSGWIN_PrintColor( BR_MSGWIN_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, GFL_FONT *p_font, PRINTSYS_LSB lsb );
void BR_MSGWIN_PrintBufColor( BR_MSGWIN_WORK* p_wk, const STRBUF *cp_strbuf, GFL_FONT *p_font, PRINTSYS_LSB lsb );
extern BOOL BR_MSGWIN_PrintMain( BR_MSGWIN_WORK* p_wk );

typedef enum
{
  BR_MSGWIN_POS_ABSOLUTE,
  BR_MSGWIN_POS_WH_CENTER,
}BR_MSGWIN_POS;
extern void BR_MSGWIN_SetPos( BR_MSGWIN_WORK* p_wk, s16 x, s16 y, BR_MSGWIN_POS type );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					  小さいボール演出
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	動きの種類
//=====================================
typedef enum
{ 
  BR_BALLEFF_MOVE_NOP,           //動かない(表示されない)
  BR_BALLEFF_MOVE_EMIT,          //放射に動く          STOP
  BR_BALLEFF_MOVE_LINE,          //線の動き            STOP
  BR_BALLEFF_MOVE_OPENING,       //開始の動き          STOP
  BR_BALLEFF_MOVE_BIG_CIRCLE,    //大きい円を描く      LOOP
  BR_BALLEFF_MOVE_CIRCLE,        //円を描く            LOOP
  BR_BALLEFF_MOVE_CIRCLE_CONT,   //場所を変えて円を描くLOOP
  BR_BALLEFF_MOVE_EMIT_FOLLOW,   //放射に動き、ついて行く STOP
  BR_BALLEFF_MOVE_OPENING_TOUCH, //開始時のタッチ     　STOP
}BR_BALLEFF_MOVE;
//-------------------------------------
///	カーソルワーク
//=====================================
typedef struct _BR_BALLEFF_WORK BR_BALLEFF_WORK;

//-------------------------------------
///	外部公開
//=====================================
extern BR_BALLEFF_WORK *BR_BALLEFF_Init( GFL_CLUNIT *p_unit, BR_RES_WORK *p_res, CLSYS_DRAW_TYPE draw, HEAPID heapID );
extern void BR_BALLEFF_Exit( BR_BALLEFF_WORK *p_wk );
extern void BR_BALLEFF_Main( BR_BALLEFF_WORK *p_wk );

extern void BR_BALLEFF_StartMove( BR_BALLEFF_WORK *p_wk, BR_BALLEFF_MOVE move, const GFL_POINT *cp_pos );
extern BOOL BR_BALLEFF_IsMoveEnd( const BR_BALLEFF_WORK *cp_wk );

extern void BR_BALLEFF_SetAnmSeq( BR_BALLEFF_WORK *p_wk, int seq );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					  リスト表示構造体
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	SELECTの返り値
//=====================================
#define BR_LIST_SELECT_NONE (0xFFFFFFFF)

//-------------------------------------
///	リストタイプ
//=====================================
typedef enum
{
  BR_LIST_TYPE_TOUCH,  //タッチリスト
  BR_LIST_TYPE_CURSOR, //カーソルリスト
} BR_LIST_TYPE;

//-------------------------------------
///	リスト設定情報
//=====================================
typedef struct 
{
  const BMP_MENULIST_DATA *cp_list;
  u32                     list_max;
  u8  x;
  u8  y;
  u8  w;
  u8  h;
  u8  plt;
  u8  frm;
  u8  str_line; //何行の文字か(何キャラ使うか)
  BR_LIST_TYPE    type;
  BR_RES_WORK     *p_res;
  GFL_CLUNIT      *p_unit;
  BR_BALLEFF_WORK *p_balleff_main;  //[in]リストカーソル用
  BR_BALLEFF_WORK *p_balleff_sub;   //[in]タッチ演出用
  BR_LIST_POS     *p_pos; //[in/out]リスト位置（br_inner.hに定義してある）
} BR_LIST_PARAM;

//-------------------------------------
///	リスト
//=====================================
typedef struct _BR_LIST_WORK BR_LIST_WORK;

//-------------------------------------
///	パブリック
//=====================================
extern BR_LIST_WORK * BR_LIST_Init( const BR_LIST_PARAM *cp_param, HEAPID heapID );
extern void BR_LIST_Exit( BR_LIST_WORK* p_wk );
extern void BR_LIST_Main( BR_LIST_WORK* p_wk );

extern BOOL BR_LIST_IsMoveEnable( const BR_LIST_WORK* cp_wk );
extern u32 BR_LIST_GetSelect( const BR_LIST_WORK* cp_wk );

//-------------------------------------
///	リスト拡張
//=====================================
//  通常は上記関数のみで動作するが、外部から特殊な文字を書き込みたいときは、
//  BMPに文字列を書き込んで、下記関数に渡すことで、スクロールに対応できる
//  その際、BMP_MENULIST_DATAのstrはNULLにすること。
extern void BR_LIST_SetBmp( BR_LIST_WORK* p_wk, u16 idx, GFL_BMP_DATA *p_src );
extern GFL_BMP_DATA *BR_LIST_GetBmp( const BR_LIST_WORK* cp_wk, u16 idx );

typedef enum
{
  BR_LIST_PARAM_IDX_CURSOR_POS,
  BR_LIST_PARAM_IDX_LIST_POS,
  BR_LIST_PARAM_IDX_MOVE_TIMING,
  BR_LIST_PARAM_IDX_LIST_MAX,

} BR_LIST_PARAM_IDX;
extern u32 BR_LIST_GetParam( const BR_LIST_WORK* cp_wk, BR_LIST_PARAM_IDX param );
extern void BR_LIST_Write( BR_LIST_WORK *p_wk );
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					  上画面テキスト
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	テキストワーク
//    内部的にはMSGWINと同じだが
//    フレームの書き込みを行なう
//=====================================
typedef BR_MSGWIN_WORK BR_TEXT_WORK;

//-------------------------------------
///	パブリック
//=====================================
extern BR_TEXT_WORK * BR_TEXT_Init( BR_RES_WORK *p_res, PRINT_QUE *p_que, HEAPID heapID );
extern void BR_TEXT_Exit( BR_TEXT_WORK *p_wk, BR_RES_WORK *p_res );
extern void BR_TEXT_Print( BR_TEXT_WORK* p_wk, const BR_RES_WORK *cp_res, u32 strID );
extern void BR_TEXT_PrintBuff( BR_TEXT_WORK* p_wk, const BR_RES_WORK *cp_res, const STRBUF *cp_strbuf );
extern BOOL BR_TEXT_PrintMain( BR_TEXT_WORK* p_wk );


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					  プロフィール画面作成
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	プロフィールの種類
//=====================================
typedef enum
{
  BR_PROFILE_TYPE_MY,   //自分
  BR_PROFILE_TYPE_OTHER,   //他人
} BR_PROFILE_TYPE;

//-------------------------------------
///	プロフィールワーク
//=====================================
typedef struct _BR_PROFILE_WORK BR_PROFILE_WORK;

//-------------------------------------
///	パブリック
//=====================================
extern BR_PROFILE_WORK * BR_PROFILE_CreateMainDisplay( const GDS_PROFILE_PTR cp_profile, BR_RES_WORK *p_res, GFL_CLUNIT *p_unit, PRINT_QUE *p_que, BR_PROFILE_TYPE type, HEAPID heapID );
extern void BR_PROFILE_DeleteMainDisplay( BR_PROFILE_WORK *p_wk );
extern BOOL BR_PROFILE_PrintMain( BR_PROFILE_WORK *p_wk );


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					  シーケンス管理
 */
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_
//-------------------------------------
///	シーケンスワーク
//=====================================
typedef struct _BR_SEQ_WORK BR_SEQ_WORK;

//-------------------------------------
///	シーケンス関数
//=====================================
typedef void (*BR_SEQ_FUNCTION)( BR_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	外部公開
//=====================================
extern BR_SEQ_WORK *BR_SEQ_Init( void *p_wk_adrs, BR_SEQ_FUNCTION seq_function, HEAPID heapID );
extern void BR_SEQ_Exit( BR_SEQ_WORK *p_wk );
extern void BR_SEQ_Main( BR_SEQ_WORK *p_wk );
extern BOOL BR_SEQ_IsEnd( const BR_SEQ_WORK *cp_wk );
extern void BR_SEQ_SetNext( BR_SEQ_WORK *p_wk, BR_SEQ_FUNCTION seq_function );
extern void BR_SEQ_End( BR_SEQ_WORK *p_wk );
extern void BR_SEQ_SetReservSeq( BR_SEQ_WORK *p_wk, int seq );
extern void BR_SEQ_NextReservSeq( BR_SEQ_WORK *p_wk );
extern BOOL BR_SEQ_IsComp( const BR_SEQ_WORK *cp_wk, BR_SEQ_FUNCTION seq_function );

