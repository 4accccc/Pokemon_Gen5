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
///	一回分の管理データ
//==============================================================
typedef struct {
  GFL_CLWK* clwk[2];
} PMS_DRAW_UNIT;

//--------------------------------------------------------------
///	メインワーク
//==============================================================
struct _PMS_DRAW_WORK {
  // [IN]
  GFL_CLUNIT* clwk_unit;
  PRINT_QUE*  print_que;
  // [PRIVATE]
  u8 unit_num;
  u8 padding[3];
  PMS_DRAW_UNIT*  unit;
};

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static void _unit_init( PMS_DRAW_UNIT* unit );
static void _unit_exit( PMS_DRAW_UNIT* unit );

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
PMS_DRAW_WORK* PMS_DRAW_Init( GFL_CLUNIT* clunit, PRINT_QUE* que, u8 obj_pltt_ofs, u8 id_max, HEAPID heap_id )
{
  int i;
  PMS_DRAW_WORK* wk;

  // ヒープ生成
  wk = GFL_HEAP_AllocClearMemory( heap_id, sizeof(PMS_DRAW_WORK) );
  GFL_STD_MemClear( wk, sizeof(PMS_DRAW_WORK) );

  wk->clwk_unit   = clunit;
  wk->print_que   = que;
  wk->unit_num    = id_max;

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
//inline BOOL PRINT_UTIL_Trans( PRINT_UTIL* wk, PRINT_QUE* que )

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

  // OBJの強制終了に注意

  for( i=0; i<wk->unit_num; i++ )
  {
    _unit_exit( &wk->unit[i] );
    GFL_HEAP_FreeMemory( wk->unit );
  }

  // ヒープ開放
  GFL_HEAP_FreeMemory( wk );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  指定IDに簡易会話を表示
 *
 *	@param	PMS_DRAW_WORK* wk
 *	@param	win
 *	@param	data
 *	@param	id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMS_DRAW_Print( PMS_DRAW_WORK* wk, GFL_BMPWIN* win, PMS_DRAW_DATA* data, u8 id )
{ 
  // 先勝ちで動作させる
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

}

//-----------------------------------------------------------------------------
/**
 *	@brief  指定IDの表示クリア
 *
 *	@param	PMS_DRAW_WORK* wk
 *	@param	win
 *	@param	id 
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void PMS_DRAW_Clear( PMS_DRAW_WORK* wk, GFL_BMPWIN* win, u8 id )
{ 
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
  // @TODO アクター解放
  GFL_HEAP_FreeMemory( unit );
}


