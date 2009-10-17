//=============================================================================
/**
 *
 *	@file		pms_draw.c
 *	@brief  簡易会話表示システム（簡易会話+デコメ管理）
 *	@author	hosaka genya
 *	@data		2009.10.14
 *
 */
//=============================================================================
#include <gflib.h>
#include "print/printsys.h"

#include "system/pms_draw.h"

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	要素ごとの管理データ
//==============================================================
typedef struct {
  BOOL          b_useflag;
  PRINT_UTIL    print_util;
  GFL_BMPWIN*   win;
  GFL_CLWK*     clwk[2];
} PMS_DRAW_UNIT;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
struct _PMS_DRAW_WORK {
  // [IN]
  HEAPID      heap_id;
  GFL_CLUNIT* clwk_unit;
  GFL_FONT*   font;
  PRINT_QUE*  print_que;
  // [PRIVATE]
  u8 unit_num;
  u8 padding[3];
  BOOL            b_print_end;  ///< プリント終了フラグ
  PMS_DRAW_UNIT*  unit;
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static void _unit_init( PMS_DRAW_UNIT* unit );
static void _unit_exit( PMS_DRAW_UNIT* unit );
static BOOL _unit_proc( PMS_DRAW_UNIT* unit, PRINT_QUE* que );
static void _unit_print( PMS_DRAW_UNIT* unit, PRINT_QUE* print_que, GFL_FONT* font, GFL_BMPWIN* win, PMS_DATA* pms, HEAPID heap_id );
static void _unit_clear( PMS_DRAW_UNIT* unit );

//=============================================================================
/**
 *								外部公開関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示システム 初期化
 *
 *	@param	GFL_CLUNIT* clunit
 *	@param	que
 *	@param	obj_pltt_ofs
 *	@param	id_max  管理ID数
 *	@param	heap_id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
PMS_DRAW_WORK* PMS_DRAW_Init( GFL_CLUNIT* clunit, PRINT_QUE* que, GFL_FONT* font, u8 obj_pltt_ofs, u8 id_max, HEAPID heap_id )
{
  int i;
  PMS_DRAW_WORK* wk;

  // ヒープ生成
  wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof(PMS_DRAW_WORK) );
  GFL_STD_MemClear( wk, sizeof(PMS_DRAW_WORK) );

  wk->heap_id     = heap_id;
  wk->clwk_unit   = clunit;
  wk->print_que   = que;
  wk->font        = font;
  wk->unit_num    = id_max;

  // ユニット初期化
  wk->unit = GFL_HEAP_AllocClearMemory( heap_id, sizeof(PMS_DRAW_UNIT) *id_max );
  GFL_STD_MemClear( wk->unit, sizeof(PMS_DRAW_UNIT) * id_max );

  // OBJ リソース取得

  for( i=0; i<wk->unit_num; i++ )
  {
    _unit_init( &wk->unit[i] );
  }

  return wk;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示システム 主処理
 *
 *	@param	PMS_DRAW_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMS_DRAW_Proc( PMS_DRAW_WORK* wk )
{
  int i;

  // 何こともなければ転送終了
  wk->b_print_end = TRUE;
  
  for( i=0; i<wk->unit_num; i++ )
  {
    if( _unit_proc( &wk->unit[i], wk->print_que ) )
    {
      // 一個でも転送が終わってないものがあったら転送終了フラグをOFF
      wk->b_print_end = FALSE;
    }
  }

}

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話表示システム 開放処理
 *
 *	@param	PMS_DRAW_WORK* wk 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMS_DRAW_Exit( PMS_DRAW_WORK* wk )
{
  int i;

  // 表示ユニットワーク終了処理
  for( i=0; i<wk->unit_num; i++ )
  {
    _unit_exit( &wk->unit[i] );
  }
    
  // 表示ユニットワークを一括フリー
  GFL_HEAP_FreeMemory( wk->unit );

  // ヒープ開放
  GFL_HEAP_FreeMemory( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  指定IDに簡易会話を表示
 *
 *	@param	PMS_DRAW_WORK* wk
 *	@param	win
 *	@param	pms
 *	@param	id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
// @TODO 先勝ちで動作させる
void PMS_DRAW_Print( PMS_DRAW_WORK* wk, GFL_BMPWIN* win, PMS_DATA* pms, u8 id )
{ 
  PMS_DRAW_UNIT* unit;

  GF_ASSERT( wk && win && pms );
  GF_ASSERT( id < wk->unit_num );

  _unit_print( &wk->unit[id], wk->print_que, wk->font, win, pms, wk->heap_id );
    
  // PROCを通るまでは転送されない
  wk->b_print_end = FALSE;
}


//-----------------------------------------------------------------------------
/**
 *	@brief  指定IDの表示終了チェック
 *
 *	@param	PMS_DRAW_WORK* wk
 *	@param	id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
BOOL PMS_DRAW_IsPrintEnd( PMS_DRAW_WORK* wk, u8 id )
{
  return wk->b_print_end;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  指定IDの表示クリア
 *
 *	@param	PMS_DRAW_WORK* wk
 *	@param	id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMS_DRAW_Clear( PMS_DRAW_WORK* wk, u8 id )
{ 
  PMS_DRAW_UNIT* unit;

  GF_ASSERT( wk );
  GF_ASSERT( id < wk->unit_num );
  
  _unit_clear( &wk->unit[id] );
}


//=============================================================================
/**
 *								static関数
 */
//=============================================================================

//-----------------------------------------------------------------------------
/**
 *	@brief  表示ユニット 初期化
 *
 *	@param	PMS_DRAW_UNIT* unit 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _unit_init( PMS_DRAW_UNIT* unit )
{
  unit->b_useflag = FALSE;
  // @TODO アクター初期化
}

//-----------------------------------------------------------------------------
/**
 *	@brief  表示ユニット 削除
 *
 *	@param	PMS_DRAW_UNIT* unit 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
static void _unit_exit( PMS_DRAW_UNIT* unit )
{
  GFL_BMPWIN* win;

  win = unit->print_util.win;

  GF_ASSERT( win );
  GFL_BMPWIN_Delete( win );

  // @TODO アクター解放
  
  // @TODO OBJの強制終了に注意

}

//-----------------------------------------------------------------------------
/**
 *	@brief  表示ユニット 主処理
 *
 *	@param	PMS_DRAW_UNIT* unit
 *	@param	que 
 *
 *  @retval BOOL	まだ転送が終わっていない場合はTRUE／それ以外FALSE
 */
//-----------------------------------------------------------------------------
static BOOL _unit_proc( PMS_DRAW_UNIT* unit, PRINT_QUE* que )
{
  BOOL doing;

  doing = PRINT_UTIL_Trans( &unit->print_util, que );
  
  if( doing == FALSE )
  {
    // @TODO 表示が終了していればアクター表示
  }

  return doing;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  表示ユニット プリント
 *
 *	@param	PMS_DRAW_UNIT* unit
 *	@param	print_que
 *	@param	font
 *	@param	win
 *	@param	pms
 *	@param	heap_id 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void _unit_print( PMS_DRAW_UNIT* unit, PRINT_QUE* print_que, GFL_FONT* font, GFL_BMPWIN* win, PMS_DATA* pms, HEAPID heap_id )
{
  STRBUF* buf;

  GF_ASSERT( unit->b_useflag == FALSE );

  // プリント設定初期化
  PRINT_UTIL_Setup( &unit->print_util, win );

  // プリントリクエスト
  buf = PMSDAT_ToString( pms, heap_id );
  PRINT_UTIL_Print( &unit->print_util, print_que, 0, 0, buf, font );
  GFL_STR_DeleteBuffer( buf );
 
  unit->b_useflag = TRUE;
      
  // 転送
	GFL_BMPWIN_MakeScreen( unit->print_util.win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame( unit->print_util.win) );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  表示ユニット クリア
 *
 *	@param	PMS_DRAW_UNIT* unit 
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void _unit_clear( PMS_DRAW_UNIT* unit )
{
  GF_ASSERT( unit->b_useflag == FALSE );

  // スクリーンをクリアして転送
	GFL_BMPWIN_ClearScreen( unit->print_util.win );
	GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame( unit->print_util.win) );

  // @TODO 画像非表示

  unit->b_useflag = FALSE;

}
