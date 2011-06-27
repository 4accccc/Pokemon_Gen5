//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		key_system_util.h
 *	@brief  共通モジュール
 *	@author	Toru=Nagihashi
 *	@date		2011.05.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "system/main.h"    //HEAPID
#include "print/gf_font.h"  //GFL_FONT
#include "system/bmp_menulist.h"  //BMPMENULIST_NULL
#include "gamesystem/game_data.h"

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					テキスト描画構造体
 *            ・文字描画はストリーム
 *            ・テキスト範囲は画面下部文字2行ほど
 *            ・BMPWINの周りにフレームがつく
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	描画方式
//=====================================
typedef enum
{
  KEYSYS_TEXT_TYPE_QUE,     //プリントキューを使う
  KEYSYS_TEXT_TYPE_STREAM,  //ストリームを使う(読み飛ばしあり、速さ設定依存)
  KEYSYS_TEXT_TYPE_WAIT,   //待機アイコンをつかう  終了はEndWait
  KEYSYS_TEXT_TYPE_STREAM_FIX,  //ストリームを使う（固定速さ）

  KEYSYS_TEXT_TYPE_MAX,    //c内部にて使用
} KEYSYS_TEXT_TYPE;

//-------------------------------------
///	座標位置
//=====================================
typedef enum 
{
  KEYSYS_TEXT_POS_ABSOLUTE,     //絶対位置
  KEYSYS_TEXT_POS_WH_CENTER,  //相対座標  幅、高さともに中央
  KEYSYS_TEXT_POS_H_CENTER,    //相対座標  高さのみ中央
  KEYSYS_TEXT_POS_W_BACK,     //後ろから
} KEYSYS_TEXT_POS;

//-------------------------------------
///	メッセージウィンドウ
//=====================================
typedef struct _KEYSYS_TEXT_WORK KEYSYS_TEXT_WORK;
//-------------------------------------
///	パブリック
//=====================================
extern KEYSYS_TEXT_WORK * KEYSYS_TEXT_Init( u16 frm, u16 x, u16 y, u16 w, u16 h, u16 font_plt, GFL_FONT *p_font, HEAPID heapID );
extern void KEYSYS_TEXT_Exit( KEYSYS_TEXT_WORK* p_wk );
extern void KEYSYS_TEXT_Draw( KEYSYS_TEXT_WORK* p_wk );
extern void KEYSYS_TEXT_Print( KEYSYS_TEXT_WORK* p_wk, GFL_MSGDATA *p_msg, u32 strID, KEYSYS_TEXT_TYPE type );
extern void KEYSYS_TEXT_PrintBuf( KEYSYS_TEXT_WORK* p_wk, const STRBUF *cp_strbuf, KEYSYS_TEXT_TYPE type );
extern void KEYSYS_TEXT_SetColor( KEYSYS_TEXT_WORK* p_wk, PRINTSYS_LSB color );
extern void KEYSYS_TEXT_SetPos( KEYSYS_TEXT_WORK* p_wk, s16 x, s16 y, KEYSYS_TEXT_POS type );
extern BOOL KEYSYS_TEXT_IsMsgEnd( const KEYSYS_TEXT_WORK* cp_wk );
extern void KEYSYS_TEXT_EndWait( KEYSYS_TEXT_WORK* p_wk );
extern void KEYSYS_TEXT_WriteWindowFrame( KEYSYS_TEXT_WORK *p_wk, u16 frm_chr, u8 frm_plt );
extern void KEYSYS_TEXT_ClearCharacter( KEYSYS_TEXT_WORK* p_wk );
extern void KEYSYS_TEXT_ChangePalette( KEYSYS_TEXT_WORK* p_wk, u8 plt );

#ifdef PM_DEBUG
extern void KEYSYS_TEXT_PrintDebug( KEYSYS_TEXT_WORK* p_wk, const u16 *cp_str, u16 len, GFL_FONT *p_font );
#endif

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					メッセージセット構造体
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	メッセージセットウィンドウ
//=====================================
typedef struct _KEYSYS_TEXTSET_WORK KEYSYS_TEXTSET_WORK;

//-------------------------------------
///	メッセージセット初期化
//=====================================
typedef struct
{
  u16 x;
  u16 y;
  u16 w;
  u16 h;
  u32 strID;
} KEYSYS_TEXTSET_WND_DATA;

//-------------------------------------
///	パブリック
//=====================================
extern KEYSYS_TEXTSET_WORK * KEYSYS_TEXTSET_Init( const KEYSYS_TEXTSET_WND_DATA *cp_data_tbl, u16 data_num, u16 frm, u16 font_plt, GFL_FONT *p_font, GFL_MSGDATA *p_msg, HEAPID heapID );
extern void KEYSYS_TEXTSET_Exit( KEYSYS_TEXTSET_WORK* p_wk );
extern void KEYSYS_TEXTSET_Draw( KEYSYS_TEXTSET_WORK* p_wk );
extern BOOL KEYSYS_TEXTSET_IsDrawEnd( const KEYSYS_TEXTSET_WORK* cp_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  リスト
 *				    ・簡単にリストを出すために最大値決めうち
 *            ・表示数＝リスト最大数
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
#define KEYSYS_LIST_SELECT_NULL    (BMPMENULIST_NULL)
#define KEYSYS_LIST_WINDOW_MAX     (4)

//-------------------------------------
///	選択リスト
//=====================================
typedef struct 
{
  GFL_MSGDATA *p_msg;
  GFL_FONT    *p_font;
  PRINT_QUE   *p_que;
  u32 strID[KEYSYS_LIST_WINDOW_MAX];
  u32 list_max;

  u16 frm;
  u16 font_plt;
  u16 frm_plt;
  u16 frm_chr;

  BOOL  is_cancel;
  u16   cancel_idx;
  u16   default_idx;
} KEYSYS_LIST_SETUP;

//-------------------------------------
///	選択リスト
//=====================================
typedef struct _KEYSYS_LIST_WORK KEYSYS_LIST_WORK;
//-------------------------------------
///	パブリック
//=====================================
extern KEYSYS_LIST_WORK * KEYSYS_LIST_Init( const KEYSYS_LIST_SETUP *cp_setup, HEAPID heapID );
extern KEYSYS_LIST_WORK * KEYSYS_LIST_InitEx( const KEYSYS_LIST_SETUP *cp_setup, u8 x, u8 y, u8 w, u8 h, HEAPID heapID );
extern void KEYSYS_LIST_Exit( KEYSYS_LIST_WORK *p_wk );
extern u32 KEYSYS_LIST_Main( KEYSYS_LIST_WORK *p_wk );
extern BOOL KEYSYS_LIST_Draw( KEYSYS_LIST_WORK *p_wk );

//=============================================================================
/**
 *					メニュー
*/
//=============================================================================
#define KEYSYS_MENU_WINDOW_MAX (4)
//-------------------------------------
///	メニュー
//=====================================
typedef struct _KEYSYS_MENU_WORK KEYSYS_MENU_WORK;
//-------------------------------------
///	初期化構造体
//=====================================
typedef struct
{
  u32 frm;
  u32 font_plt;
  u32 frm_chr;
  u32 frm_plt;
  GFL_MSGDATA       *p_msg;
  GFL_FONT          *p_font;
  u32 max;
  s32 cursor;
  struct
  {
    u32 strID;
    u8  x;
    u8  y;
    u8  w;
    u8  h;
  } wnd_data[KEYSYS_MENU_WINDOW_MAX];
  BOOL (*DecideCallBack)( int index, void *p_wk_adrs ); //押せるならばTRUE
  void *p_wk_adrs;
}KEYSYS_MENU_PARAM;
//-------------------------------------
///	パブリック
//=====================================
extern KEYSYS_MENU_WORK * KEYSYS_MENU_Init( const KEYSYS_MENU_PARAM *cp_param, HEAPID heapID );
extern void KEYSYS_MENU_Exit( KEYSYS_MENU_WORK *p_wk );
extern void KEYSYS_MENU_Main( KEYSYS_MENU_WORK *p_wk );
extern void KEYSYS_MENU_Draw( KEYSYS_MENU_WORK *p_wk );
extern BOOL KEYSYS_MENU_IsDrawEnd( const KEYSYS_MENU_WORK *cp_wk );
extern s32 KEYSYS_MENU_GetSelect( const KEYSYS_MENU_WORK *cp_wk );
extern BOOL KEYSYS_MENU_IsDecide( const KEYSYS_MENU_WORK *cp_wk );
extern void KEYSYS_MENU_ReStart( KEYSYS_MENU_WORK *p_wk );
extern const KEYSYS_MENU_PARAM * KEYSYS_MENU_GetParam( const KEYSYS_MENU_WORK *cp_wk );
extern BOOL KEYSYS_MENU_IsChangeSelect( const KEYSYS_MENU_WORK *cp_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  シーケンス管理
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	シーケンスワーク
//=====================================
typedef struct _KEYSYS_SEQ_WORK KEYSYS_SEQ_WORK;

//-------------------------------------
///	シーケンス関数
//=====================================
typedef void (*KEYSYS_SEQ_FUNCTION)( KEYSYS_SEQ_WORK *p_wk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	パブリック
//=====================================
extern KEYSYS_SEQ_WORK *KEYSYS_SEQ_Init( u32 stack_max, void *p_wk_adrs, KEYSYS_SEQ_FUNCTION seq_function, HEAPID heapID );
extern void KEYSYS_SEQ_Exit( KEYSYS_SEQ_WORK *p_wk );
extern void KEYSYS_SEQ_Main( KEYSYS_SEQ_WORK *p_wk );
extern BOOL KEYSYS_SEQ_IsEnd( const KEYSYS_SEQ_WORK *cp_wk );
extern void KEYSYS_SEQ_Next( KEYSYS_SEQ_WORK *p_wk, KEYSYS_SEQ_FUNCTION seq_function );
extern void KEYSYS_SEQ_Call( KEYSYS_SEQ_WORK *p_wk, KEYSYS_SEQ_FUNCTION seq_function );
extern void KEYSYS_SEQ_End( KEYSYS_SEQ_WORK *p_wk );
extern void KEYSYS_SEQ_Return( KEYSYS_SEQ_WORK *p_wk );
extern void KEYSYS_SEQ_SetReservSeq( KEYSYS_SEQ_WORK *p_wk, int seq );
extern void KEYSYS_SEQ_NextReservSeq( KEYSYS_SEQ_WORK *p_wk );
extern BOOL KEYSYS_SEQ_IsCompireFunc( const KEYSYS_SEQ_WORK *cp_wk, KEYSYS_SEQ_FUNCTION seq_function );
extern void KEYSYS_SEQ_AddLocalSeq( KEYSYS_SEQ_WORK *p_wk, int add );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  ディスプレイ作成
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	ディスプレイ作成テーブル
//=====================================
typedef struct
{
  void (*Create)( void *p_wk_adrs, HEAPID heapID );
  BOOL (*IsEndCreate)( void *p_wk_adrs );
  void (*Delete)( void *p_wk_adrs );
  BOOL (*IsEndDelete)( void *p_wk_adrs );
} KEYSYS_DISPLAY_FUNCTION;

//-------------------------------------
///	シーケンスワーク
//=====================================
typedef struct _KEYSYS_DISPLAY_GENELATER KEYSYS_DISPLAY_GENELATER;

//-------------------------------------
///	パブリック
//=====================================
extern KEYSYS_DISPLAY_GENELATER * KEYSYS_DISPLAY_Init( void *p_wk_adrs, HEAPID heapID );
extern void KEYSYS_DISPLAY_Exit( KEYSYS_DISPLAY_GENELATER *p_wk );
extern void KEYSYS_DISPLAY_Main( KEYSYS_DISPLAY_GENELATER *p_wk );
extern void KEYSYS_DISPLAY_Create( KEYSYS_DISPLAY_GENELATER *p_wk, const KEYSYS_DISPLAY_FUNCTION *cp_func, HEAPID heapID );
extern void KEYSYS_DISPLAY_Delete( KEYSYS_DISPLAY_GENELATER *p_wk );
extern BOOL KEYSYS_DISPLAY_IsComplete( const KEYSYS_DISPLAY_GENELATER *cp_wk );


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  ETC
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
extern void KEYSYS_UTIL_MainPltAnm( NNS_GFD_DST_TYPE type, u16 *p_buff, u16 cnt, u8 plt_num, u8 plt_col, GXRgb start, GXRgb end );
