//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		local_tvt_callselect.c
 *	@brief  呼び出し選択画面
 *	@author	Toru=Nagihashi
 *	@data		2011.01.13
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//システム
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "sound/pm_sndsys.h"
#include "app/app_taskmenu.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"
#include "comm_tvt.naix"

//自分のモジュール
#include "local_tvt_graphic.h"
#include "local_tvt_util.h"
#include "local_tvt_snd.h"

//外部参照
#include "local_tvt_callselect.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//リソースインデックス
enum
{
  RESID_OBJ_PLT,
  RESID_OBJ_NCG,
  RESID_OBJ_ANM,
  RESID_MAX,
};

//セルアニメ
enum
{
  ANMID_BAR,
  ANMID_CHECK_OFF,
  ANMID_CHECK_ON,
  ANMID_CHECK_NG,
  ANMID_SCROLL_BAR,
  ANMID_SCROLL_BAR_PUSH,
  ANMID_BAR_PUSH,
  ANMID_BAR_SELECT,
};

//バー位置
#define CTVT_CALL_BAR_X (128)
#define CTVT_CALL_BAR_Y (16)
#define CTVT_CALL_BAR_HEIGHT (32)

#define CTVT_CALL_SEARCH_NUM (15) //画面分

#define CTVT_CALL_SCROLL_LEN (CTVT_CALL_BAR_HEIGHT*CTVT_CALL_SEARCH_NUM-20*8)
#define CTVT_CALL_SCROLL_SPEED  (8)

#define CTVT_CALL_CHECK_X (18)
#define CTVT_CALL_CAMERA_X (200)

#define CTVT_CALL_SCROLL_X (256-8)
#define CTVT_CALL_SCROLL_Y (24)
#define CTVT_CALL_SCROLLBAR_LEN (120)

#define CTVT_CALL_SCROLL_HIT_WIDTH  (12)
#define CTVT_CALL_SCROLL_HIT_HEIGHT (12)


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	バー１つずつのワーク
//=====================================
typedef struct 
{
  BOOL  is_check;
  BOOL  is_select;
  BOOL  is_active;
  BOOL  is_no_push; //押せない
  
  GFL_CLWK *p_clwk_bar;
  GFL_CLWK *p_clwk_check;
  GFL_CLWK *p_clwk_camera;

  GFL_BMPWIN *p_str_win;
  
  u32 bar_idx;
} BAR;

//-------------------------------------
///	スクロールバー
//=====================================
typedef struct 
{
  BOOL        is_use;
  BOOL        is_hold_scroll;
  BOOL        is_update;
  BOOL        is_target_update;
  GFL_CLWK    *p_clwk_scrollbar;
  s32         scroll_bar_pos;
  s32         scroll_bar_next;
} SCROLLBAR;

//-------------------------------------
///	決定ボタン&メッセージ表示
//=====================================
typedef struct 
{
  GFL_BMPWIN              *p_msgwin;
  APP_TASKMENU_WIN_WORK   *p_menuwin;
  APP_TASKMENU_RES        *p_menures;
  LOCAL_TVT_MESSAGE       *p_message;
  STRBUF                  *p_btnstr;
  STRBUF                  *p_msgstr;
  HEAPID                  heapID;
  BOOL                    is_visible;
  BOOL                    is_select;
} DECIDE;

//-------------------------------------
///	バー集合
//=====================================
typedef struct 
{
  u16     scroll_ofs;
  u16     max;
  s32     cursor_pos;
  s32     is_key_trg_update;
  BAR     bar[LOCAL_TVT_CALLSELECT_SELECT_MAX];

  SCROLLBAR   scroll_bar;
  DECIDE      decide;
} BARSET;

//-------------------------------------
///	呼び出し選択画面メインワーク
//=====================================
struct _LOCAL_TVT_CALLSELECT
{
  s32         res[RESID_MAX];
  u32         seq;
  LOCAL_TVT_CALLSELECT_RESULT result;

  LOCAL_TVT_CALLSELECT_PARAM  param;
  BARSET      barset;

  LOCAL_TVT_TOUCHBAR  *p_touchbar;
  LIST_UI             *p_ui;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//resource
static void LOCAL_TVT_CALLSELECT_LoadResource( LOCAL_TVT_CALLSELECT *p_wk, HEAPID heapID );
static void LOCAL_TVT_CALLSELECT_UnLoadResource( LOCAL_TVT_CALLSELECT *p_wk );

//UI
static LIST_UI_RESULT UI_UpdateTouch( void *p_wk_adrs );
static LIST_UI_RESULT UI_UpdateKey( void *p_wk_adrs );
static void UI_Change( void *p_wk_adrs, int touch_or_key );

//barset
static void BARSET_Init( BARSET *p_wk, const LOCAL_TVT_CALLSELECT *cp_sys, HEAPID heapID );
static void BARSET_Exit( BARSET *p_wk );
static void BARSET_Main( BARSET *p_wk );
static void BARSET_AnimeMain( BARSET *p_wk );
static BOOL BARSET_Touch( BARSET *p_wk );
static BOOL BARSET_Key( BARSET *p_wk );
static s32  BARSET_GetSelectIdx( const BARSET *cp_wk ); 
static BOOL BARSET_IsSelect( const BARSET *cp_wk ); 
static void BARSET_SetUIMode( BARSET *p_wk, int touch_or_key );
static BOOL BARSET_IsTrg( const BARSET *cp_wk );
static BOOL BARSET_IsEnd( const BARSET *cp_wk );
static BOOL BARSET_IsMoveScrollbar( const BARSET *cp_wk );

//bar
static void BAR_Init( BAR *p_wk, const LOCAL_TVT_CALLSELECT *cp_sys, GFL_CLUNIT *p_clunit, u32 bar_idx, const STRBUF *cp_name, LOCAL_TVT_MESSAGE *p_message, BOOL is_no_push, HEAPID heapID);
static void BAR_Exit( BAR *p_wk );
static void BAR_Move( BAR *p_wk, s32 scroll_ofs );
static BOOL BAR_SetCheck( BAR *p_wk, BOOL is_check );
static void BAR_StartBlink( BAR *p_wk );
static BOOL BAR_IsCheck( const BAR *cp_wk );
static void BAR_SetActibe( BAR *p_wk, BOOL is_active );
static BOOL BAR_IsActibe( const BAR *cp_wk );
static void BAR_SetSelect( BAR *p_wk, BOOL is_select );
static BOOL BAR_IsSelect( const BAR *cp_wk );

//scroll_bar
static void SCROLLBAR_Init( SCROLLBAR *p_wk, const LOCAL_TVT_CALLSELECT *cp_sys, HEAPID heapID );
static void SCROLLBAR_Exit( SCROLLBAR *p_wk );
static void SCROLLBAR_Main( SCROLLBAR *p_wk );
static BOOL SCROLLBAR_Touch( SCROLLBAR *p_wk );
static BOOL SCROLLBAR_IsMove( const SCROLLBAR *cp_wk );
static BOOL SCROLLBAR_IsDrag( const SCROLLBAR *cp_wk );
static void SCROLLBAR_SetScrollRate( SCROLLBAR *p_wk, fx32 rate );
static fx32 SCROLLBAR_GetScrollRate( const SCROLLBAR *cp_wk );

//decide
static void DECIDE_Init( DECIDE *p_wk, const LOCAL_TVT_CALLSELECT *cp_sys, HEAPID heapID );
static void DECIDE_Exit( DECIDE *p_wk );
static void DECIDE_Main( DECIDE *p_wk );
static void DECIDE_FlipMain( DECIDE *p_wk, const BARSET *cp_barset );
static void DECIDE_SetTrg( DECIDE *p_wk );
static void DECIDE_SetSelect( DECIDE *p_wk, BOOL is_select );
static BOOL DECIDE_IsSelect( const DECIDE *cp_wk );
static BOOL DECIDE_IsButton( const DECIDE *cp_wk );
static BOOL DECIDE_IsTrg( const DECIDE *cp_wk );
static BOOL DECIDE_IsEnd( const DECIDE *cp_wk );

//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し選択画面初期化
 *
 *	@param	const LOCAL_TVT_CALLSELECT_PARAM * cp_param 初期化構造体
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_CALLSELECT * LOCAL_TVT_CALLSELECT_Init( const LOCAL_TVT_CALLSELECT_PARAM * cp_param, HEAPID heapID )
{
  LOCAL_TVT_CALLSELECT *p_wk  = GFL_HEAP_AllocClearMemory( heapID, sizeof(LOCAL_TVT_CALLSELECT));
  p_wk->param = *cp_param;

  //リソース読込み
  LOCAL_TVT_CALLSELECT_LoadResource( p_wk, heapID );

  //タッチバーの作成
  p_wk->p_touchbar  = LOCAL_TVT_TOUCHBAR_Init( p_wk->param.p_clunit, heapID );

  //バーの初期化
  BARSET_Init( &p_wk->barset, p_wk, heapID );

  //UIシステム
  {
    LIST_UI_PARAM param;
    GFL_STD_MemClear( &param, sizeof(LIST_UI_PARAM) );
    param.UpdateTouch = UI_UpdateTouch;
    param.UpdateKey   = UI_UpdateKey;
    param.Change      = UI_Change;
    param.p_wk_adrs   = p_wk;
    p_wk->p_ui  = LIST_UI_Init( &param ,heapID );
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し選択画面破棄
 *
 *	@param	LOCAL_TVT_CALLSELECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CALLSELECT_Exit( LOCAL_TVT_CALLSELECT *p_wk )
{
  //UIの破棄
  LIST_UI_Exit( p_wk->p_ui );

  //バーの破棄
  BARSET_Exit( &p_wk->barset );

  //タッチバーの破棄
  LOCAL_TVT_TOUCHBAR_Exit( p_wk->p_touchbar );

  //リソース破棄
  LOCAL_TVT_CALLSELECT_UnLoadResource( p_wk );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し選択画面メイン処理
 *
 *	@param	LOCAL_TVT_CALLSELECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LOCAL_TVT_CALLSELECT_Main( LOCAL_TVT_CALLSELECT *p_wk )
{
  enum
  {
    SEQ_MAIN,
    SEQ_DECIDEANM,
    SEQ_CANCELANM,
    SEQ_END,
  };

  switch( p_wk->seq )
  {
  case SEQ_MAIN:
    //バーのメイン処理
    BARSET_Main( &p_wk->barset );

    //スクロールバー移動中は操作を受け付けない
    if( !BARSET_IsMoveScrollbar( &p_wk->barset ) )
    {
      //UIのメイン
      LIST_UI_Main( p_wk->p_ui );

      //タッチバー動作
      LOCAL_TVT_TOUCHBAR_Main( p_wk->p_touchbar );
    }

    //バーを決定
    if( LIST_UI_IsDecide( p_wk->p_ui ) )
    {
      p_wk->seq= SEQ_DECIDEANM;
    }

    //キャンセル
    if( LOCAL_TVT_TOUCHBAR_IsTrg( p_wk->p_touchbar ) )
    {
      p_wk->seq= SEQ_CANCELANM;
    }
    break;

  case SEQ_DECIDEANM:
    //決定アニメまち
    if( BARSET_IsEnd( &p_wk->barset ) )
    {
      p_wk->result  = LOCAL_TVT_CALLSELECT_RESULT_DECIDE;
      p_wk->seq= SEQ_END;
    }
    break;

  case SEQ_CANCELANM:
    //キャンセルアニメまち
    if( LOCAL_TVT_TOUCHBAR_IsEnd( p_wk->p_touchbar ) )
    {
      p_wk->result  = LOCAL_TVT_CALLSELECT_RESULT_CANCEL;
      p_wk->seq= SEQ_END;
    }
    break;

  case SEQ_END:
    break;
  }

  BARSET_AnimeMain( &p_wk->barset );
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し選択画面終了検知
 *
 *	@param	const LOCAL_TVT_CALLSELECT *cp_wk   ワーク
 *
 *	@return LOCAL_TVT_CALLSELECT_RESULT_NONE,     未選択
 *          LOCAL_TVT_CALLSELECT_RESULT_DECIDE,   決定
 *          LOCAL_TVT_CALLSELECT_RESULT_CANCEL,   キャンセル
 *
 */
//-----------------------------------------------------------------------------
LOCAL_TVT_CALLSELECT_RESULT LOCAL_TVT_CALLSELECT_GetResult( const LOCAL_TVT_CALLSELECT *cp_wk )
{
  return cp_wk->result;
}
//----------------------------------------------------------------------------
/**
 *	@brief  呼び出し選択画面  現在選択しているものを取得
 *
 *	@param	const LOCAL_TVT_CALLSELECT *cp_wk   ワーク
 *
 *	@return インデックス
 */
//-----------------------------------------------------------------------------
s32 LOCAL_TVT_CALLSELECT_GetSelect( const LOCAL_TVT_CALLSELECT *cp_wk )
{
  return BARSET_GetSelectIdx( &cp_wk->barset );
}

//----------------------------------------------------------------------------
/**
 *	@brief  リソース読込み
 *
 *	@param	LOCAL_TVT_CALLSELECT *p_wk  ワーク
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_CALLSELECT_LoadResource( LOCAL_TVT_CALLSELECT *p_wk, HEAPID heapID )
{
  ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_COMM_TVT_GRA , heapID ); 

  //セルリソース
  p_wk->res[RESID_OBJ_PLT]  = GFL_CLGRP_PLTT_RegisterComp( p_handle , 
      NARC_comm_tvt_tv_t_obj_NCLR , CLSYS_DRAW_SUB , 
      LTVT_PLT_S_OBJ*32 , heapID  ); 
  p_wk->res[RESID_OBJ_NCG]  = GFL_CLGRP_CGR_Register( p_handle , 
      NARC_comm_tvt_tv_t_call_obj_NCGR , FALSE , CLSYS_DRAW_SUB , heapID  );
  p_wk->res[RESID_OBJ_ANM]  = GFL_CLGRP_CELLANIM_Register( p_handle , 
      NARC_comm_tvt_tv_t_call_obj_NCER , NARC_comm_tvt_tv_t_call_obj_NANR, heapID  ); 

  //BG
  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle , NARC_comm_tvt_tv_t_tuuwa_bg_NCGR ,
      LTVT_FRAME_SUB_MSG , 0 , (32*10)*0x20, FALSE , heapID );
  GFL_ARCHDL_UTIL_TransVramScreen( p_handle , NARC_comm_tvt_tv_t_yobidashi_bg_NSCR , 
      LTVT_FRAME_SUB_MSG ,  0 , 0, FALSE , heapID );
  GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_MSG );

  GFL_ARC_CloseDataHandle( p_handle );


} 
//----------------------------------------------------------------------------
/**
 *	@brief  リソース破棄
 *
 *	@param	LOCAL_TVT_CALLSELECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void LOCAL_TVT_CALLSELECT_UnLoadResource( LOCAL_TVT_CALLSELECT *p_wk )
{
  GFL_BG_ClearScreen( LTVT_FRAME_SUB_MSG );
  GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_MSG );

  GFL_CLGRP_CELLANIM_Release( p_wk->res[RESID_OBJ_ANM] );
  GFL_CLGRP_CGR_Release( p_wk->res[RESID_OBJ_NCG] );
  GFL_CLGRP_PLTT_Release( p_wk->res[RESID_OBJ_PLT] );
}

//----------------------------------------------------------------------------
/**
 *	@brief  タッチの操作
 *
 *	@param	void *p_wk_adrs   わーく
 *
 *	@return 結果
 */
//-----------------------------------------------------------------------------
static LIST_UI_RESULT UI_UpdateTouch( void *p_wk_adrs )
{
  LOCAL_TVT_CALLSELECT *p_wk  = p_wk_adrs;

  LIST_UI_RESULT  result  = LIST_UI_RESULT_NONE;

  //バーのタッチ処理
  if( BARSET_Touch( &p_wk->barset ) )
  {
    result  = LIST_UI_RESULT_MOVE;
  }

  if( BARSET_IsTrg( &p_wk->barset ) )
  {
    result  = LIST_UI_RESULT_DECIDE;
  }

  return result;
}
//----------------------------------------------------------------------------
/**
 *	@brief  キーの操作
 *
 *	@param	void *p_wk_adrs   わーく
 *
 *	@return 結果
 */
//-----------------------------------------------------------------------------
static LIST_UI_RESULT UI_UpdateKey( void *p_wk_adrs )
{
  LOCAL_TVT_CALLSELECT *p_wk  = p_wk_adrs;
  LIST_UI_RESULT  result  = LIST_UI_RESULT_NONE;

  if( BARSET_Key( &p_wk->barset ) )
  {
    result  = LIST_UI_RESULT_MOVE;
  }
  
  if( BARSET_IsTrg( &p_wk->barset ) )
  {
    result = LIST_UI_RESULT_DECIDE;
  }

  return result;
}
//----------------------------------------------------------------------------
/**
 *	@brief
 *
 *	@param	void *p_wk_adrs わーｋ
 *	@param  touch_or_key  キーに切り替えたかタッチに切り替えたか
 */
//-----------------------------------------------------------------------------
static void UI_Change( void *p_wk_adrs, int touch_or_key )
{
  LOCAL_TVT_CALLSELECT *p_wk  = p_wk_adrs;

  BARSET_SetUIMode( &p_wk->barset, touch_or_key );
}

//=============================================================================
/**
 *  バー初期化
 */
//=============================================================================
static BOOL Barset_GetTouchBarIndex( const BARSET *cp_wk, s32 *p_bar_idx );
//----------------------------------------------------------------------------
/**
 *	@brief  バーの初期化
 *
 *	@param	LOCAL_TVT_CALLSELECT *p_wk  ワーク
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void BARSET_Init( BARSET *p_wk, const LOCAL_TVT_CALLSELECT *cp_sys, HEAPID heapID )
{
  int i;

  GF_ASSERT( cp_sys->param.max <= LOCAL_TVT_CALLSELECT_SELECT_MAX );

  GFL_STD_MemClear( p_wk, sizeof(BARSET));
  p_wk->max = cp_sys->param.max;

  //スクロールバーの初期化
  SCROLLBAR_Init( &p_wk->scroll_bar, cp_sys, heapID );

  //決定の初期化
  DECIDE_Init( &p_wk->decide, cp_sys, heapID );

  //バーの初期化処理
  for( i = 0; i < p_wk->max; i++ )
  {
    STRBUF  *p_str  = GFL_MSG_CreateString( cp_sys->param.p_msgdata, cp_sys->param.msgID[i] );
    BAR_Init( &p_wk->bar[i], cp_sys, cp_sys->param.p_clunit, i, p_str, cp_sys->param.p_message, !cp_sys->param.is_select[i], heapID );
    GFL_STR_DeleteBuffer( p_str );
  }
  //文字面の設定
  GFL_BG_SetScrollReq( LTVT_FRAME_SUB_MISC , GFL_BG_SCROLL_Y_SET , p_wk->scroll_ofs );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーの破棄
 *
 *	@param	LOCAL_TVT_CALLSELECT *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BARSET_Exit( BARSET *p_wk )
{
  int i;

  //文字面の破棄
  GFL_BG_ClearScreen( LTVT_FRAME_SUB_MISC );
  GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_MISC );
  GFL_BG_SetScrollReq( LTVT_FRAME_SUB_MISC, GFL_BG_SCROLL_Y_SET , 0 );

  //バーの破棄処理
  for( i = 0; i < p_wk->max; i++ )
  {
    BAR_Exit( &p_wk->bar[i] );
  }

  //決定の破棄
  DECIDE_Exit( &p_wk->decide );

  //スクロールバーの破棄
  SCROLLBAR_Exit( &p_wk->scroll_bar );

  GFL_STD_MemClear( p_wk, sizeof(BARSET));
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーのメイン処理
 *
 *	@param	LOCAL_TVT_CALLSELECT *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void BARSET_Main( BARSET *p_wk )
{
  BOOL is_update_bar_pos  = FALSE;

  //決定の切り替え処理
  DECIDE_FlipMain( &p_wk->decide, p_wk );
  
  //スクロール
  if( SCROLLBAR_IsDrag( &p_wk->scroll_bar ) )
  {
    const u32 max_len = (p_wk->max-5) * CTVT_CALL_BAR_HEIGHT;
    p_wk->scroll_ofs = FX_Whole( max_len * SCROLLBAR_GetScrollRate( &p_wk->scroll_bar ) );
    is_update_bar_pos = TRUE;
  }

  //スクロールバーのメイン処理
  SCROLLBAR_Main( &p_wk->scroll_bar );

  //押されたときの判定
  if( p_wk->is_key_trg_update )
  {
    p_wk->is_key_trg_update = FALSE;
    //規定の数押されたら、それ以外はアクティブじゃなくなる
    //もし何もおされていないならば、すべてノンアクティブ
    if( BARSET_IsSelect( p_wk ) == FALSE )
    {
      int i;
      for( i = 0; i < p_wk->max; i++ )
      {
        if( BAR_IsActibe(&p_wk->bar[ i ]) == FALSE )
        {
          BAR_SetActibe( &p_wk->bar[ i ], TRUE );
        }
      }
    }
    else
    {    
      int i;
      for( i = 0; i < p_wk->max; i++ )
      {
        if( i != BARSET_GetSelectIdx( p_wk ) )
        {
          BAR_SetActibe( &p_wk->bar[ i ], FALSE );
        }
      }
    }
  }

  //バーのメイン処理
  {
    int i;
    //バーの位置処理
    for( i = 0; i < p_wk->max; i++ )
    {
      BAR_Move( &p_wk->bar[i], p_wk->scroll_ofs );
    }
  }
  //動作処理
  if( is_update_bar_pos )
  {
    //文字面の設定
    GFL_BG_SetScrollReq( LTVT_FRAME_SUB_MISC , GFL_BG_SCROLL_Y_SET , p_wk->scroll_ofs );

  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  アニメ処理
 *
 *	@param	BARSET *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BARSET_AnimeMain( BARSET *p_wk )
{
  //決定のメイン
  DECIDE_Main( &p_wk->decide );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーのタッチ処理
 *
 *	@param	LOCAL_TVT_CALLSELECT *p_wk ワーク
 *	@retval   TRUEタッチした  FALSE操作していない
 */
//-----------------------------------------------------------------------------
static BOOL BARSET_Touch( BARSET *p_wk )
{
  //バーへのタッチ処理
  int i;
  u32 tpx,tpy;
  if( GFL_UI_TP_GetPointTrg( &tpx,&tpy ) )
  {
    //押したバーを取得
    if( Barset_GetTouchBarIndex( p_wk, &p_wk->cursor_pos ) )
    {
      //1個までしかONにできない
      //OFFにはできる
      
      if( BAR_IsCheck( &p_wk->bar[ p_wk->cursor_pos ] ) )
      {
        //すでに押されていたので解除
        if( BAR_SetCheck( &p_wk->bar[ p_wk->cursor_pos ], FALSE ) )
        {
          BAR_StartBlink( &p_wk->bar[ p_wk->cursor_pos ] );
          p_wk->is_key_trg_update = TRUE;
          return   TRUE;
        }
      }
      else
      {
        //押した
        if( BAR_SetCheck( &p_wk->bar[ p_wk->cursor_pos ], TRUE ) )
        {
          BAR_StartBlink( &p_wk->bar[ p_wk->cursor_pos ] );
          p_wk->is_key_trg_update = TRUE;
          return   TRUE;
        }
      }
    }
  }

  //スクロールバーのタッチメイン
  if( SCROLLBAR_Touch( &p_wk->scroll_bar ) )
  {
    return TRUE;
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーのキー操作処理
 *
 *	@param	BARSET *p_wk  ワーク
 *
 *	@return TRUEキー操作した  FALSEしていない
 */
//-----------------------------------------------------------------------------
static BOOL BARSET_Key( BARSET *p_wk )
{
  BOOL is_select  = BARSET_IsSelect( p_wk );
  s32  select = 0;
  BOOL is_update  = FALSE;
  BOOL is_decide  = FALSE;

  u32 max = p_wk->max;
  if( DECIDE_IsButton( &p_wk->decide ) )
  {
    max++;
  }

  //現在選択されているものを取得
  if( is_select )
  {
    select  = BARSET_GetSelectIdx( p_wk ); 
  }

  //キー操作
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP )
  {
    p_wk->cursor_pos--;
    if( p_wk->cursor_pos < 0 )
    {
      p_wk->cursor_pos  = max-1;
    }

    PMSND_PlaySE( LOCAL_TVT_SE_SELECT );
    is_update = TRUE;
  }
  else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN )
  {
    p_wk->cursor_pos++;
    p_wk->cursor_pos  %= max;

    PMSND_PlaySE( LOCAL_TVT_SE_SELECT );
    is_update = TRUE;
  }
  else if( GFL_UI_KEY_GetTrg( ) & PAD_BUTTON_A )
  {
    int i;

    if( p_wk->cursor_pos  == p_wk->max )
    {
      DECIDE_SetTrg( &p_wk->decide );
      is_update = TRUE;
      is_decide = TRUE;
      p_wk->is_key_trg_update = TRUE;
    }
    else
    {
      BOOL is_check = BAR_IsCheck(&p_wk->bar[ p_wk->cursor_pos ]);
      if( BAR_SetCheck( &p_wk->bar[ p_wk->cursor_pos ], !is_check ) )
      {
        int i;
        for( i = 0; i < p_wk->max; i++ )
        {
          if( p_wk->cursor_pos != i )
          {
            BAR_SetCheck( &p_wk->bar[ i ], FALSE );
          }
        }

        BAR_StartBlink( &p_wk->bar[ p_wk->cursor_pos ] );
        is_update = TRUE;
        is_decide = TRUE;
        p_wk->is_key_trg_update = TRUE;
      }
    }
  }

  //操作による処理
  if( is_update )
  {
    int i;
    //バーの選択処理
    for( i = 0; i < p_wk->max; i++ )
    {
      if( p_wk->cursor_pos == i )
      {
        BAR_SetSelect( &p_wk->bar[i], TRUE );
      }
      else
      {
        BAR_SetSelect( &p_wk->bar[i], FALSE );
      }
    }

    //スクロール
    if( p_wk->max >= LOCAL_TVT_CALLSELECT_SELECT_MAX )
    {
      if( p_wk->cursor_pos <= 0 )
      {
        SCROLLBAR_SetScrollRate( &p_wk->scroll_bar, 0 );
      }
      else if( p_wk->cursor_pos >= 5 )
      {
        SCROLLBAR_SetScrollRate( &p_wk->scroll_bar, FX32_ONE );
      }
    }

    //決定ボタンの選択処理
    if( p_wk->cursor_pos == p_wk->max )
    {
      DECIDE_SetSelect( &p_wk->decide, TRUE );
    }
    else
    {
      DECIDE_SetSelect( &p_wk->decide, FALSE );
    }
  }

  return is_decide;
}

//----------------------------------------------------------------------------
/**
 *	@brief  タッチしたかどうか
 *
 *	@param	const BAR *cp_wk   ワーク
 *	@param  s32 p_bar_idx タッチしたバーのインデックス
 *
 *	@return TRUEでタッチ  FALSEで触っていない
 */
//-----------------------------------------------------------------------------
static BOOL Barset_GetTouchBarIndex( const BARSET *cp_wk, s32 *p_bar_idx )
{
  u32 tpx, tpy;

  if( GFL_UI_TP_GetPointTrg( &tpx,&tpy ) )
  {
    if( tpx > 8 && tpx < 256-(8*4) && tpy < 192-(8*3) )
    {
      s32 idx = (tpy+cp_wk->scroll_ofs)/CTVT_CALL_BAR_HEIGHT;
      if( idx >= 0 && idx < cp_wk->max )
      {
        *p_bar_idx  = idx;
        return TRUE;
      }
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  選択中のバーインデックスを取得
 *
 *	@param	const LOCAL_TVT_CALLSELECT *cp_wk ワーク
 *
 *	@return 選択中のバーインデックス
 */
//-----------------------------------------------------------------------------
static s32  BARSET_GetSelectIdx( const BARSET *cp_wk )
{
  int i;
  int cnt = 0;
  for( i = 0; i < cp_wk->max; i++ )
  {
    if( BAR_IsCheck( &cp_wk->bar[i] ) )
    {
      return i;
    }
  }

  GF_ASSERT(0);
  return 0;
}
//----------------------------------------------------------------------------
/**
 *	@brief  選択中かどうかを取得
 *
 *	@param	const LOCAL_TVT_CALLSELECT *cp_wk ワーク
 *
 *	@return TRUE選択している  FALSEしていない 
 */
//-----------------------------------------------------------------------------
static BOOL BARSET_IsSelect( const BARSET *cp_wk )
{
  int i;
  int cnt = 0;
  for( i = 0; i < cp_wk->max; i++ )
  {
    if( BAR_IsCheck( &cp_wk->bar[i] ) )
    {
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バー集合  UIモードを設定
 *
 *	@param	BARSET *p_wk
 *	@param	touch_or_key 
 */
//-----------------------------------------------------------------------------
static void BARSET_SetUIMode( BARSET *p_wk, int touch_or_key )
{
  if( touch_or_key == GFL_APP_END_TOUCH )
  {
    int i;
    for( i = 0; i < p_wk->max; i++ )
    {
      BAR_SetSelect( &p_wk->bar[i], FALSE );
    }
  }
  else
  {
    int i;
    for( i = 0; i < p_wk->max; i++ )
    {
      if( p_wk->cursor_pos == i )
      {
        BAR_SetSelect( &p_wk->bar[i], TRUE );
      }
      else
      {
        BAR_SetSelect( &p_wk->bar[i], FALSE );
      }
    }

    if( p_wk->cursor_pos == p_wk->max )
    {
      DECIDE_SetSelect( &p_wk->decide, TRUE );
    }
    else
    {
      DECIDE_SetSelect( &p_wk->decide, FALSE );
    }

    //スクロール
    if( p_wk->max >= LOCAL_TVT_CALLSELECT_SELECT_MAX )
    {
      if( p_wk->cursor_pos <= 0 )
      {
        SCROLLBAR_SetScrollRate( &p_wk->scroll_bar, 0 );
      }
      else if( p_wk->cursor_pos >= 5 )
      {
        SCROLLBAR_SetScrollRate( &p_wk->scroll_bar, FX32_ONE );
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタンを押したかどうか
 *
 *	@param	const BARSET *cp_wk ワーク
 *
 *	@return TRUE  ボタンを押した  FALSEボタンを押していない
 */
//-----------------------------------------------------------------------------
static BOOL BARSET_IsTrg( const BARSET *cp_wk )
{
  return DECIDE_IsTrg( &cp_wk->decide );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタン演出が終了したか
 *
 *	@param	const BARSET *cp_wk ワーク
 *
 *	@return TRUE終了した  FALSE終了していない
 */
//-----------------------------------------------------------------------------
static BOOL BARSET_IsEnd( const BARSET *cp_wk )
{
  return DECIDE_IsEnd( &cp_wk->decide );
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクロールバー移動をしているか
 *
 *	@param	const BARSET *cp_wk ワーク
 *
 *	@return TRUEスクロールバーが移動中  FALSEしていない
 */
//-----------------------------------------------------------------------------
static BOOL BARSET_IsMoveScrollbar( const BARSET *cp_wk )
{
  return SCROLLBAR_IsMove( &cp_wk->scroll_bar );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーを初期化
 *
 *	@param	BAR *p_wk  バーワーク
 *	@param	LOCAL_TVT_CALLSELECT *cp_sys    システムワーク
 *	@param	GFL_CLUNIT  *CLWK生成用ユニット
 *	@param  u32  バーインデックス
 *	@param  cp_name 表示名前
 *	@param  p_message 文字表示システム
 *	@param  is_no_push  押せないフラグ
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void BAR_Init( BAR *p_wk, const LOCAL_TVT_CALLSELECT *cp_sys, GFL_CLUNIT *p_clunit, u32 bar_idx, const STRBUF *cp_name, LOCAL_TVT_MESSAGE *p_message, BOOL is_no_push, HEAPID heapID)
{
  GFL_STD_MemClear( p_wk, sizeof(BAR) );
  p_wk->bar_idx     = bar_idx;
  p_wk->is_no_push  = is_no_push;

  //CLWK作成
  {
    GFL_CLWK_DATA clwk_data;
    clwk_data.bgpri = 3;

    clwk_data.pos_x = CTVT_CALL_BAR_X;
    clwk_data.pos_y = CTVT_CALL_BAR_Y + CTVT_CALL_BAR_HEIGHT*bar_idx;
    clwk_data.anmseq = ANMID_BAR;
    clwk_data.softpri = 32;
    p_wk->p_clwk_bar = GFL_CLACT_WK_Create( p_clunit,
              cp_sys->res[RESID_OBJ_NCG],
              cp_sys->res[RESID_OBJ_PLT],
              cp_sys->res[RESID_OBJ_ANM],
              &clwk_data ,CLSYS_DRAW_SUB , heapID );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_bar, TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( p_wk->p_clwk_bar, TRUE );

    clwk_data.pos_x = CTVT_CALL_CHECK_X;
    clwk_data.anmseq = ANMID_CHECK_OFF;
    clwk_data.softpri = 16;
    p_wk->p_clwk_check = GFL_CLACT_WK_Create( p_clunit,
              cp_sys->res[RESID_OBJ_NCG],
              cp_sys->res[RESID_OBJ_PLT],
              cp_sys->res[RESID_OBJ_ANM],
              &clwk_data ,CLSYS_DRAW_SUB , heapID );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_check, TRUE );

    /*
    clwk_data.pos_x = CTVT_CALL_CAMERA_X;
    clwk_data.anmseq = CTOAS_CAMERA_ON;
    clwk_data.softpri = 16;
    p_wk->p_clwk_camera = GFL_CLACT_WK_Create( p_clunit,
              cp_sys->res[RESID_OBJ_NCG],
              cp_sys->res[RESID_OBJ_PLT],
              cp_sys->res[RESID_OBJ_ANM],
              &clwk_data ,CLSYS_DRAW_SUB , heapID );
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_camera, FALSE );
    */
  }

  //文字BMPWIN作成
  {
    p_wk->p_str_win = GFL_BMPWIN_Create( LTVT_FRAME_SUB_MISC,
                      3, bar_idx*(CTVT_CALL_BAR_HEIGHT/8)+1 , 25 , 2 ,
                      LTVT_PLT_BG_SUB_FONT, 
                      GFL_BMP_CHRAREA_GET_B );

    LOCAL_TVT_MESSAGE_SetPrintColor( p_message, LOCAL_TVT_MSG_COLOR_WHITE );
    LOCAL_TVT_MESSAGE_SetPrintPos( p_message, 6, 0,LOCAL_TVT_MESSAGE_POS_ABSOLUTE );
    LOCAL_TVT_MESSAGE_PrintStr( p_message, p_wk->p_str_win, cp_name );
    GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_str_win );
  }

  BAR_SetActibe( p_wk, TRUE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーを破棄
 *
 *	@param	BAR *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BAR_Exit( BAR *p_wk )
{
  //BMPWIN破棄
  GFL_BMPWIN_Delete( p_wk->p_str_win );

  //CLWK破棄
  //GFL_CLACT_WK_Remove( p_wk->p_clwk_camera );
  GFL_CLACT_WK_Remove( p_wk->p_clwk_check );
  GFL_CLACT_WK_Remove( p_wk->p_clwk_bar );

  GFL_STD_MemClear( p_wk, sizeof(BAR));
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーを更新
 *
 *	@param	BAR *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BAR_Move( BAR *p_wk, s32 scroll_ofs )
{
  //アニメ
  if( GFL_CLACT_WK_CheckAnmActive(p_wk->p_clwk_bar) == FALSE )
  {
    static const u8 sc_bar_anmID[] =
    {
      ANMID_BAR,
      ANMID_BAR_SELECT
    };
    GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk_bar, sc_bar_anmID[p_wk->is_select] );
  }

  //位置
  {
    s16 pos_y = p_wk->bar_idx*CTVT_CALL_BAR_HEIGHT - scroll_ofs;

    if( pos_y >= -CTVT_CALL_BAR_HEIGHT &&
        pos_y <= 192-CTVT_CALL_BAR_HEIGHT )  
    {
      GFL_CLACTPOS pos;
      pos.x = CTVT_CALL_BAR_X;
      pos.y = CTVT_CALL_BAR_Y + pos_y;
      GFL_CLACT_WK_SetPos( p_wk->p_clwk_bar , &pos , CLSYS_DRAW_SUB );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_bar , TRUE );
      pos.x = CTVT_CALL_CHECK_X;
      GFL_CLACT_WK_SetPos( p_wk->p_clwk_check , &pos , CLSYS_DRAW_SUB );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_check , TRUE );
      pos.x = CTVT_CALL_CAMERA_X;
      //GFL_CLACT_WK_SetPos( p_wk->p_clwk_camera , &pos , CLSYS_DRAW_SUB );
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  バー選択時の点滅開始
 *
 *	@param	BAR *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BAR_StartBlink( BAR *p_wk )
{
  //バーの点滅開始
  GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk_bar, ANMID_BAR_PUSH );
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーのチェックボックスを設定
 *
 *	@param	BAR *p_wk ワーク
 *	@param  is_check  TRUEチェックON, FALSEチェックOFF
 *	@retval TRUEでタッチできた  FALSEでタッチできなかった
 */
//-----------------------------------------------------------------------------
static BOOL BAR_SetCheck( BAR *p_wk, BOOL is_check )
{
  //ノンアクティブのときは押せない
  //設定するチェックと内部が同じなら押せない
  if( p_wk->is_active ==FALSE || p_wk->is_check == is_check )
  {
    return  FALSE;
  }

  p_wk->is_check  = is_check;

  //チェックボックス変更
  BAR_SetActibe( p_wk, TRUE );

  if( p_wk->is_check )
  {
    PMSND_PlaySE( LOCAL_TVT_SE_CALLBAR_TOUCH );
  }
  else
  {
    PMSND_PlaySE( LOCAL_TVT_SE_CALLBAR_CANCEL );
  }

  return TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーの選択状況を取得
 *
 *	@param	const BAR *cp_wk ワーク
 *
 *	@return TRUE選択中  FALSE選択していない
 */
//-----------------------------------------------------------------------------
static BOOL BAR_IsCheck( const BAR *cp_wk )
{
  return cp_wk->is_check;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーをアクティブ設定する
 *
 *	@param	BAR *p_wk  ワーク
 *	@param	is_active   TRUEでアクティブ  FALSEでアクティブではない
 */
//-----------------------------------------------------------------------------
static void BAR_SetActibe( BAR *p_wk, BOOL is_active )
{
  p_wk->is_active = is_active;

  if( p_wk->is_active && !p_wk->is_no_push )
  {
    //アクティブの時はチェックボックスになる
    if( p_wk->is_check )
    {
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk_check, ANMID_CHECK_ON );
    }
    else
    {
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk_check, ANMID_CHECK_OFF );
    }
  }
  else
  {
    //ノンアクティブのときは強制変更
    GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk_check, ANMID_CHECK_NG );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーがアクティブかどうか
 *
 *	@param	const BAR *cp_wk  ワーク
 *
 *	@return TRUEアクティブ  FALSEノンアクティブ
 */
//-----------------------------------------------------------------------------
static BOOL BAR_IsActibe( const BAR *cp_wk )
{
  return cp_wk->is_active;
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーの選択設定
 *
 *	@param	BAR *p_wk ワーク
 *	@param	is_select TRUE選択  FALSE選択していない
 */
//-----------------------------------------------------------------------------
static void BAR_SetSelect( BAR *p_wk, BOOL is_select )
{
  p_wk->is_select = is_select;

  //アニメ中でなければアニメ変更
  if( GFL_CLACT_WK_GetAnmSeq(p_wk->p_clwk_bar) != ANMID_BAR_PUSH )
  {
    static const u8 sc_bar_anmID[] =
    {
      ANMID_BAR,
      ANMID_BAR_SELECT
    };
    GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk_bar, sc_bar_anmID[p_wk->is_select] );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  バーが選択されているかどうか
 *
 *	@param	const BAR *cp_wk  ワーク
 *
 *	@return TRUE選択中  FALSE選択していない
 */
//-----------------------------------------------------------------------------
static BOOL BAR_IsSelect( const BAR *cp_wk )
{
  return cp_wk->is_select;
}
//=============================================================================
/**
 *    SCROLLBAR
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  スクロールバー初期化
 *
 *	@param	SCROLLBAR *p_wk ワーク
 *	@param  sys             システムワーク
 *	@param	heapID          ヒープID
 */
//-----------------------------------------------------------------------------
static void SCROLLBAR_Init( SCROLLBAR *p_wk, const LOCAL_TVT_CALLSELECT *cp_sys, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(SCROLLBAR) );
  if( cp_sys->param.max > 5) 
  {
    p_wk->is_use  = TRUE;
  }

  //スクロールアイコン
  {
    GFL_CLWK_DATA clwk_data;
    clwk_data.softpri = 0;
    clwk_data.bgpri = 0;
    clwk_data.pos_x = CTVT_CALL_SCROLL_X;
    clwk_data.pos_y = CTVT_CALL_SCROLL_Y;
    clwk_data.anmseq = ANMID_SCROLL_BAR;
    p_wk->p_clwk_scrollbar  = GFL_CLACT_WK_Create( cp_sys->param.p_clunit,
              cp_sys->res[RESID_OBJ_NCG],
              cp_sys->res[RESID_OBJ_PLT],
              cp_sys->res[RESID_OBJ_ANM],
              &clwk_data ,CLSYS_DRAW_SUB , heapID );

    if( p_wk->is_use )
    {
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_scrollbar , TRUE );
    }
    else
    {
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk_scrollbar , FALSE );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールバー破棄
 *
 *	@param	SCROLLBAR *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void SCROLLBAR_Exit( SCROLLBAR *p_wk )
{
  //スクロールアイコンの破棄
  GFL_CLACT_WK_Remove( p_wk->p_clwk_scrollbar );
  GFL_STD_MemClear( p_wk, sizeof(SCROLLBAR) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクロールバーメイン
 *
 *	@param	SCROLLBAR *p_wk   ワーク
 */
//-----------------------------------------------------------------------------
static void SCROLLBAR_Main( SCROLLBAR *p_wk )
{
  if( !p_wk->is_use )
  {
    return;
  }

  //更新
  if( p_wk->is_update )
  {
    p_wk->is_update = FALSE;

    //バー動作
    {
      GFL_CLACTPOS pos;
      pos.x = CTVT_CALL_SCROLL_X;
      pos.y = CTVT_CALL_SCROLL_Y + p_wk->scroll_bar_pos;
      GFL_CLACT_WK_SetPos( p_wk->p_clwk_scrollbar, &pos , CLSYS_DRAW_SUB );
    }
  }

  //移動更新
  if( p_wk->is_target_update )
  {
    if( p_wk->scroll_bar_pos > p_wk->scroll_bar_next )
    {
      if( p_wk->scroll_bar_pos - p_wk->scroll_bar_next < CTVT_CALL_SCROLL_SPEED )
      {
        p_wk->scroll_bar_pos  = p_wk->scroll_bar_next;
      }
      else
      {
        p_wk->scroll_bar_pos  -= CTVT_CALL_SCROLL_SPEED;
      }
    }
    else if( p_wk->scroll_bar_pos < p_wk->scroll_bar_next )
    {
      if( p_wk->scroll_bar_next - p_wk->scroll_bar_pos < CTVT_CALL_SCROLL_SPEED )
      {
        p_wk->scroll_bar_pos  = p_wk->scroll_bar_next;
      }
      else
      {
        p_wk->scroll_bar_pos  += CTVT_CALL_SCROLL_SPEED;
      }
    }

    if(p_wk->scroll_bar_next == p_wk->scroll_bar_pos )
    {
      p_wk->is_target_update  = FALSE;
    }

    p_wk->is_update = TRUE;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクロールバータッチ処理
 *
 *	@param	SCROLLBAR *p_wk ワーク
 *
 *	@retval TRUEタッチした  FALSEまだ
 */
//-----------------------------------------------------------------------------
static BOOL SCROLLBAR_Touch( SCROLLBAR *p_wk )
{
  //使用不可ならば何もしない
  if( !p_wk->is_use )
  {
    return FALSE;
  }

  {
    u32 tpx,tpy;
    u32 holdx,holdy;
    BOOL is_trg = GFL_UI_TP_GetPointTrg( &tpx,&tpy );
    BOOL is_hold = GFL_UI_TP_GetPointCont( &holdx,&holdy );

    if( is_trg && tpx >= CTVT_CALL_SCROLL_X - CTVT_CALL_SCROLL_HIT_WIDTH &&
        tpx <= CTVT_CALL_SCROLL_X + CTVT_CALL_SCROLL_HIT_WIDTH &&
        tpy >= CTVT_CALL_SCROLL_Y - CTVT_CALL_SCROLL_HIT_HEIGHT + p_wk->scroll_bar_pos &&
        tpy <= CTVT_CALL_SCROLL_Y + CTVT_CALL_SCROLL_HIT_HEIGHT + p_wk->scroll_bar_pos )
    {
      p_wk->is_hold_scroll  = TRUE;
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk_scrollbar, ANMID_SCROLL_BAR_PUSH );
      PMSND_PlaySE( LOCAL_TVT_SE_TOUCH );
      return TRUE;
    }
    else if( is_hold && p_wk->is_hold_scroll )
    {
      if( holdx >= CTVT_CALL_SCROLL_X - CTVT_CALL_SCROLL_HIT_WIDTH &&
          holdx <= CTVT_CALL_SCROLL_X + CTVT_CALL_SCROLL_HIT_WIDTH &&
          holdy >= CTVT_CALL_SCROLL_Y - CTVT_CALL_SCROLL_HIT_HEIGHT &&
          holdy <= CTVT_CALL_SCROLL_Y + CTVT_CALL_SCROLLBAR_LEN + CTVT_CALL_SCROLL_HIT_HEIGHT )
      {
        s32 scroll_ofs  = holdy - CTVT_CALL_SCROLL_Y;
        p_wk->scroll_bar_pos  = MATH_CLAMP( scroll_ofs, 0, CTVT_CALL_SCROLLBAR_LEN );

        p_wk->is_update = TRUE;

        return TRUE;
      }
    }
    else
    {
      p_wk->is_hold_scroll  = FALSE;
      GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk_scrollbar, ANMID_SCROLL_BAR );
    }
  }

  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  移動中かどうか取得
 *
 *	@param	const SCROLLBAR *cp_wk  ワーク
 *
 *	@return TRUEで自動移動中  FALSEでしていない
 */
//-----------------------------------------------------------------------------
static BOOL SCROLLBAR_IsMove( const SCROLLBAR *cp_wk )
{
  return cp_wk->is_target_update;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクロールバーのドラッグを取得
 *
 *	@param	const SCROLLBAR *cp_wk  ワーク
 *
 *	@return TRUEでドラッグ中  FALSEでしていない
 */
//-----------------------------------------------------------------------------
static BOOL SCROLLBAR_IsDrag( const SCROLLBAR *cp_wk )
{
  return cp_wk->is_update;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクロールバーの移動割合を取得
 *
 *	@param	const SCROLLBAR *cp_wk ワーク
 *
 *	@return バーの割合
 */
//-----------------------------------------------------------------------------
static fx32 SCROLLBAR_GetScrollRate( const SCROLLBAR *cp_wk )
{
  return cp_wk->scroll_bar_pos * FX32_ONE / CTVT_CALL_SCROLLBAR_LEN;
}
//----------------------------------------------------------------------------
/**
 *	@brief  スクロールバーの移動割合を設定
 *
 *	@param	SCROLLBAR *p_wk ワーク
 *	@param	rate  バーの割合
 */
//-----------------------------------------------------------------------------
static void SCROLLBAR_SetScrollRate( SCROLLBAR *p_wk, fx32 rate )
{
  p_wk->scroll_bar_next  = FX_Whole( CTVT_CALL_SCROLLBAR_LEN * rate );
  p_wk->is_target_update  = TRUE;
}
//=============================================================================
/**
 *  決定ボタン＆メッセージ
 */
//=============================================================================
static void Decide_SetVisibleBtn( DECIDE *p_wk, BOOL is_visible );
//----------------------------------------------------------------------------
/**
 *	@brief  決定ボタン＆メッセージ初期化
 *
 *	@param	DECIDE *p_wk  ワーク
 *	@param	LOCAL_TVT_CALLSELECT *cp_sys  システム
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
static void DECIDE_Init( DECIDE *p_wk, const LOCAL_TVT_CALLSELECT *cp_sys, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(DECIDE) );
  p_wk->p_message = cp_sys->param.p_message;
  p_wk->heapID  = heapID;

  //メニューリソース読込み
  {
    p_wk->p_menures = APP_TASKMENU_RES_Create( 
        LTVT_FRAME_SUB_MSG, 
        LTVT_PLT_BG_SUB_TASKMENU ,
        LOCAL_TVT_MESSAGE_GetFont(p_wk->p_message),
        LOCAL_TVT_MESSAGE_GetPrintQue(p_wk->p_message), 
        heapID ); 
  }

  p_wk->p_msgwin = GFL_BMPWIN_Create( LTVT_FRAME_SUB_MSG, 
                                        1 , 21 , 21 , 2 ,
                                        LTVT_PLT_BG_SUB_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );

  //メッセージを取得
  {
    const GFL_MSGDATA *cp_msg  = LOCAL_TVT_MESSAGE_GetCallMsgData( p_wk->p_message );
    p_wk->p_btnstr  = GFL_MSG_CreateString( cp_msg, CALL_STR_02 );
    p_wk->p_msgstr  = GFL_MSG_CreateString( cp_msg, CALL_STR_01 );
  }

  LOCAL_TVT_MESSAGE_SetPrintColor( p_wk->p_message, LOCAL_TVT_MSG_COLOR_BLACK );
  LOCAL_TVT_MESSAGE_PrintStr( p_wk->p_message, p_wk->p_msgwin, p_wk->p_msgstr );
  GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_msgwin );
  LOCAL_TVT_MESSAGE_WriteFrame( p_wk->p_message, p_wk->p_msgwin );
  GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_MSG );
}
//----------------------------------------------------------------------------
/**
 *	@brief  決定ボタン＆メッセージ破棄
 *
 *	@param	DECIDE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void DECIDE_Exit( DECIDE *p_wk )
{
  if( p_wk->p_menuwin )
  {
    APP_TASKMENU_WIN_Delete( p_wk->p_menuwin );
    p_wk->p_menuwin = NULL;
  }

  //メッセージ破棄
  {
    GFL_STR_DeleteBuffer( p_wk->p_msgstr );
    GFL_STR_DeleteBuffer( p_wk->p_btnstr );
  }

  //下ウィンドウ破棄
  GFL_BMPWIN_Delete( p_wk->p_msgwin );
  GFL_BG_ClearScreen( LTVT_FRAME_SUB_MSG );
  GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_MSG );
  GFL_BG_SetScrollReq( LTVT_FRAME_SUB_MSG, GFL_BG_SCROLL_Y_SET , 0 );

  APP_TASKMENU_RES_Delete( p_wk->p_menures );

  GFL_STD_MemClear( p_wk, sizeof(DECIDE) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  決定ボタン＆メッセージメイン処理
 *
 *	@param	DECIDE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void DECIDE_Main( DECIDE *p_wk )
{
  //決定処理
  if( p_wk->p_menuwin != NULL )
  {
    if( APP_TASKMENU_WIN_IsDecide( p_wk->p_menuwin ) == FALSE )
    {
      if( APP_TASKMENU_WIN_IsTrg( p_wk->p_menuwin ) == TRUE )
      {
        PMSND_PlaySE( LOCAL_TVT_SE_DECIDE );
        APP_TASKMENU_WIN_SetDecide( p_wk->p_menuwin, TRUE );
      }
    }

    APP_TASKMENU_WIN_Update( p_wk->p_menuwin );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  決定ボタン、メッセージの切り替えメイン
 *
 *	@param	DECIDE *p_wk    ワーク
 *	@param	BARSET *cp_barset   バー集合
 */
//-----------------------------------------------------------------------------
static void DECIDE_FlipMain( DECIDE *p_wk, const BARSET *cp_barset )
{
  BOOL is_select  = BARSET_IsSelect( cp_barset );
  if( p_wk->is_visible != is_select )
  {
    p_wk->is_visible  = is_select;

    Decide_SetVisibleBtn( p_wk, p_wk->is_visible );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  決定
 *
 *	@param	DECIDE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void DECIDE_SetTrg( DECIDE *p_wk )
{
  PMSND_PlaySE( LOCAL_TVT_SE_DECIDE );
  APP_TASKMENU_WIN_SetDecide( p_wk->p_menuwin, TRUE );
}
//----------------------------------------------------------------------------
/**
 *	@brief  決定ボタン＆メッセージボタン設定
 *
 *	@param	DECIDE *p_wk  ワーク
 *	@param	is_visible    ボタン表示ON  ボタン表示OFF
 */
//-----------------------------------------------------------------------------
static void Decide_SetVisibleBtn( DECIDE *p_wk, BOOL is_visible )
{
  if( is_visible )
  {
    //メッセージを消してボタンをON
    GFL_BMPWIN_ClearTransWindow_VBlank( p_wk->p_msgwin );
    LOCAL_TVT_MESSAGE_ClearFrame( p_wk->p_message, p_wk->p_msgwin );
    
    //もし押せる状態ならば作成
    if(p_wk->p_menuwin==NULL)
    {
      APP_TASKMENU_ITEMWORK menu_param;
      GFL_STD_MemClear( &menu_param, sizeof(APP_TASKMENU_ITEMWORK) );
      menu_param.str = p_wk->p_btnstr;
      menu_param.msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
      menu_param.type = APP_TASKMENU_WIN_TYPE_NORMAL; 

      p_wk->p_menuwin = APP_TASKMENU_WIN_Create( p_wk->p_menures, &menu_param, 0 , 21 , 21 , p_wk->heapID );
    }
  }
  else
  {
    //ボタンを消してメッセージをON
    if( p_wk->p_menuwin != NULL )
    {
      APP_TASKMENU_WIN_Delete( p_wk->p_menuwin );
      p_wk->p_menuwin = NULL;
    }

    //メッセージ表示
    LOCAL_TVT_MESSAGE_SetPrintColor( p_wk->p_message, LOCAL_TVT_MSG_COLOR_BLACK );
    LOCAL_TVT_MESSAGE_PrintStr( p_wk->p_message, p_wk->p_msgwin, p_wk->p_msgstr );
    GFL_BMPWIN_MakeTransWindow_VBlank( p_wk->p_msgwin );
    LOCAL_TVT_MESSAGE_WriteFrame( p_wk->p_message, p_wk->p_msgwin );
  }

  GFL_BG_LoadScreenReq( LTVT_FRAME_SUB_MSG );
}
//----------------------------------------------------------------------------
/**
 *	@brief  選択設定
 *
 *	@param	DECIDE *p_wk  ワーク
 *	@param	is_select   TRUE選択  FALSE選択していない
 */
//-----------------------------------------------------------------------------
static void DECIDE_SetSelect( DECIDE *p_wk, BOOL is_select )
{
  if( DECIDE_IsButton( p_wk ) )
  {
    p_wk->is_select = is_select;
    APP_TASKMENU_WIN_SetActive( p_wk->p_menuwin, p_wk->is_select);
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  選択中を取得
 *
 *	@param	const DECIDE *cp_wk   ワーク
 *
 *	@return TRUE選択中  FALSE選択していない
 */
//-----------------------------------------------------------------------------
static BOOL DECIDE_IsSelect( const DECIDE *cp_wk )
{
  return cp_wk->is_select;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタン状態かウィンドウ状態か
 *
 *	@param	const DECIDE *cp_wk   ワーク
 *
 *	@return TRUE  ボタン状態  FALSEウィンドウ状態
 */
//-----------------------------------------------------------------------------
static BOOL DECIDE_IsButton( const DECIDE *cp_wk )
{
  return cp_wk->p_menuwin != NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタンを押したかどうか
 *
 *	@param	const DECIDE *cp_wk   ワーク
 *
 *	@return TRUE  ボタンを押した  FALSEボタンを押していない
 */
//-----------------------------------------------------------------------------
static BOOL DECIDE_IsTrg( const DECIDE *cp_wk )
{
  if( cp_wk->p_menuwin != NULL )
  {
    return APP_TASKMENU_WIN_IsDecide( cp_wk->p_menuwin );
  }
  return FALSE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ボタン演出が終了したか
 *
 *	@param	const DECIDE *cp_wk ワーク
 *
 *	@return TRUE終了した  FALSE終了していない
 */
//-----------------------------------------------------------------------------
static BOOL DECIDE_IsEnd( const DECIDE *cp_wk )
{
  if( cp_wk->p_menuwin != NULL )
  {
    return APP_TASKMENU_WIN_IsFinish( cp_wk->p_menuwin );
  }
  return FALSE;
}
