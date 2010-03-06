//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		livebattlematch_flow.c
 *	@brief  ライブ大会フロー
 *	@author	Toru=Nagihashi
 *	@date		2010.03.06
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>

//システム
#include "system/gfl_use.h"
#include "system/main.h"  //HEAPID
#include "poke_tool/pokeparty.h"
#include "gamesystem/btl_setup.h"
#include "print/str_tool.h"
#include "poke_tool/poke_regulation.h"
#include "sound/pm_sndsys.h"
#include "system/net_err.h"

//プロセス


//アーカイブ
#include "msg/msg_battle_championship.h"

//ネット
#include "net/network_define.h"
#include "net/dreamworld_netdata.h"

//WIFIBATTLEMATCHのモジュール
#include "net_app/wifi_match/wifibattlematch_util.h"
#include "net_app/wifi_match/wifibattlematch_graphic.h"
#include "net_app/wifi_match/wifibattlematch_view.h"

//外部公開
#include "livebattlematch_flow.h"

//-------------------------------------
///	DEBUG
//=====================================
#ifdef PM_DEBUG
#endif //PM_DEBUG

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
//-------------------------------------
///	ライブ大会ワーク
//=====================================
struct _LIVEBATTLEMATCH_FLOW_WORK
{ 
  //シーケンス制御モジュール
  WBM_SEQ_WORK                  *p_seq;

  //選択肢表示モジュール
  WBM_LIST_WORK                 *p_list;
  u32                           list_type;

  //戻り値
  LIVEBATTLEMATCH_FLOW_RET      retcode;

  //引数
  LIVEBATTLEMATCH_FLOW_PARAM    param;

  //ヒープID
  HEAPID                        heapID;

};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	シーケンス関数
//=====================================
static void SEQFUNC_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );
static void SEQFUNC_End( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs );

//-------------------------------------
///	便利関数
//=====================================
//リスト
typedef enum
{ 
  LVM_MENU_TYPE_YESNO,
}LVM_MENU_TYPE;
static void UTIL_LIST_Create( LIVEBATTLEMATCH_FLOW_WORK *p_wk, LVM_MENU_TYPE type );
static void UTIL_LIST_Delete( LIVEBATTLEMATCH_FLOW_WORK *p_wk );
static u32 UTIL_LIST_Main( LIVEBATTLEMATCH_FLOW_WORK *p_wk );

//テキスト
static void UTIL_TEXT_Print( LIVEBATTLEMATCH_FLOW_WORK *p_wk, u32 strID );
static BOOL UTIL_TEXT_IsEnd( LIVEBATTLEMATCH_FLOW_WORK *p_wk );

//フロー終了
static void UTIL_FLOW_End( LIVEBATTLEMATCH_FLOW_WORK *p_wk, LIVEBATTLEMATCH_FLOW_RET ret );


//=============================================================================
/**
 *					データ
*/
//=============================================================================
//=============================================================================
/**
 *					外部公開
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  ライブ大会フロー  初期
 *
 *	@param	const LIVEBATTLEMATCH_FLOW_PARAM *cp_param  引数
 *	@param	heapID  ヒープID 
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
LIVEBATTLEMATCH_FLOW_WORK *LIVEBATTLEMATCH_FLOW_Init( const LIVEBATTLEMATCH_FLOW_PARAM *cp_param, HEAPID heapID )
{ 
  LIVEBATTLEMATCH_FLOW_WORK *p_wk;

  //ワーク作成
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(LIVEBATTLEMATCH_FLOW_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(LIVEBATTLEMATCH_FLOW_WORK) );
  p_wk->param   = *cp_param;
  p_wk->heapID  = heapID;

  //モジュール作成
  p_wk->p_seq = WBM_SEQ_Init( p_wk, SEQFUNC_Start, heapID );

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  ライブ大会フロー  破棄
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LIVEBATTLEMATCH_FLOW_Exit( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{ 
  //モジュール破棄
  WBM_SEQ_Exit( p_wk->p_seq);

  //ワーク破棄
  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  ライブ大会フロー  メイン処理
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void LIVEBATTLEMATCH_FLOW_Main( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{ 
  WBM_SEQ_Main( p_wk->p_seq );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ライブ大会フロー  終了検知
 *
 *	@param	const LIVEBATTLEMATCH_FLOW_WORK *cp_wk  ワーク
 *
 *	@return 終了コード（詳細は列挙型を参照）
 */
//-----------------------------------------------------------------------------
LIVEBATTLEMATCH_FLOW_RET LIVEBATTLEMATCH_FLOW_IsEnd( const LIVEBATTLEMATCH_FLOW_WORK *cp_wk )
{ 
  if( WBM_SEQ_IsEnd( cp_wk->p_seq )  )
  { 
    return cp_wk->retcode;
  }
  return LIVEBATTLEMATCH_FLOW_RET_CONTINUE;
}

//=============================================================================
/**
 *  シーケンス関数
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	開始
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_wk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_Start( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  LIVEBATTLEMATCH_FLOW_WORK *p_wk = p_wk_adrs;
  
}
//----------------------------------------------------------------------------
/**
 *	@brief	終了
 *
 *	@param	WBM_SEQ_WORK *p_seqwk	シーケンスワーク
 *	@param	*p_seq					シーケンス
 *	@param	*p_seqwk_adrs				ワーク
 */
//-----------------------------------------------------------------------------
static void SEQFUNC_End( WBM_SEQ_WORK *p_seqwk, int *p_seq, void *p_wk_adrs )
{ 
  //終了
  WBM_SEQ_End( p_seqwk );
}

//=============================================================================
/**
 *  便利関数
 */
//=============================================================================
//-------------------------------------
///	リスト
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  リストを表示
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *	@param	type                                表示リストタイプ
 */
//-----------------------------------------------------------------------------
static void UTIL_LIST_Create( LIVEBATTLEMATCH_FLOW_WORK *p_wk, LVM_MENU_TYPE type )
{ 
  enum
  { 
    POS_CENTER,
    POS_RIGHT_DOWN,
  } pos;

  u8 x,y,w,h;

  WBM_LIST_SETUP  setup;
  GFL_STD_MemClear( &setup, sizeof(WBM_LIST_SETUP) );
  setup.p_msg   = p_wk->param.p_msg;
  setup.p_font  = p_wk->param.p_font;
  setup.p_que   = p_wk->param.p_que;


  switch( type )
  { 
  case LVM_MENU_TYPE_YESNO:
    setup.strID[0]= BC_SELECT_07;
    setup.strID[1]= BC_SELECT_08;
    setup.list_max= 2;
    pos = POS_RIGHT_DOWN;
    break;
  }
  setup.frm     = BG_FRAME_M_TEXT;
  setup.font_plt= PLT_FONT_M;
  setup.frm_plt = PLT_LIST_M;
  setup.frm_chr = CGR_OFS_M_LIST;


  switch( pos )
  { 
  case POS_CENTER:
    w  = 28;
    h  = setup.list_max * 2;
    x  = 32 / 2  - w /2; 
    y  = (24-6) /2  - h /2;
    break;
  case POS_RIGHT_DOWN:
    w  = 12;
    h  = setup.list_max * 2;
    x  = 32 - w - 1; //1はフレーム分
    y  = 24 - h - 1 - 6; //１は自分のフレーム分と6はテキスト分
    break;
  }

  p_wk->p_list  = WBM_LIST_InitEx( &setup, x, y, w, h, p_wk->heapID );
  p_wk->list_type = type;
}
//----------------------------------------------------------------------------
/**
 *	@brief  リストを削除
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void UTIL_LIST_Delete( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{ 
  WBM_LIST_Exit( p_wk->p_list );
}
//----------------------------------------------------------------------------
/**
 *	@brief  リスト処理
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *
 *	@retval 選択肢インデックス
 */
//-----------------------------------------------------------------------------
static u32 UTIL_LIST_Main( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{ 
  u32 ret = WBM_LIST_Main( p_wk->p_list );

  if( ret == WBM_LIST_SELECT_CALNCEL )
  { 
    if( p_wk->list_type == LVM_MENU_TYPE_YESNO )
    { 
      ret = 1;  //NOにする
    }
    else
    { 
      //キャンセルきかない
      ret = WBM_LIST_SELECT_NULL;
    }
  }

  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  テキスト文字表示
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *	@param	strID                               文字
 */
//-----------------------------------------------------------------------------
static void UTIL_TEXT_Print( LIVEBATTLEMATCH_FLOW_WORK *p_wk, u32 strID )
{ 
  WBM_TEXT_Print( p_wk->param.p_text, p_wk->param.p_msg, strID, WBM_TEXT_TYPE_STREAM );
}
//----------------------------------------------------------------------------
/**
 *	@brief  テキスト表示待ち
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *
 *	@return TRUE文字描画完了  FALSE文字描画中
 */
//-----------------------------------------------------------------------------
static BOOL UTIL_TEXT_IsEnd( LIVEBATTLEMATCH_FLOW_WORK *p_wk )
{ 
  return WBM_TEXT_IsEnd( p_wk->param.p_text );
}
//----------------------------------------------------------------------------
/**
 *	@brief  フロー終了
 *
 *	@param	LIVEBATTLEMATCH_FLOW_WORK *p_wk ワーク
 *	@param	ret                                 戻り値
 */
//-----------------------------------------------------------------------------
static void UTIL_FLOW_End( LIVEBATTLEMATCH_FLOW_WORK *p_wk, LIVEBATTLEMATCH_FLOW_RET ret )
{ 
  p_wk->retcode = ret;
  WBM_SEQ_SetNext( p_wk->p_seq, SEQFUNC_End );
}
