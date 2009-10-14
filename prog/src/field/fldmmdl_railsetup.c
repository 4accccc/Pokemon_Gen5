//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fldmmdl_railsetup.c
 *	@brief  動作モデル　レールセットアップ処理
 *	@author	tomoya takahshi
 *	@date		2009.10.12
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "fldmmdl.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
#define RAIL_SETUP_DATA_MAGIC_NUMBER  ( 0xefefefef )
//-------------------------------------
///	レール初期化構造体
//  
//  動作バッファを使用します。
//=====================================
typedef struct 
{
  u32 indata;       ///<データがあることを知らせるパラメータ
  RAIL_LOCATION location;             ///<レールロケーション 8bytie
} MV_RAIL_SETUP_WORK;
#define MV_RAIL_SETUP_WORK_SIZE (sizeof(MV_RAIL_SETUP_WORK))		///<MV_RAIL_SETUPサイズ

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static void MMdl_SetSetUpLocation( MMDL* mmdl, const RAIL_LOCATION* location );
  

//----------------------------------------------------------------------------
/**
 *	@brief  レール用　ヘッダー反映処理    登録処理　前処理
 *
 *	@param	mmdl
 *	@param	head 
 */
//-----------------------------------------------------------------------------
void MMDL_SetRailHeaderBefore( MMDL* mmdl, const MMDL_HEADER* head )
{
  GF_ASSERT( head->pos_type == MMDL_HEADER_POSTYPE_RAIL );

 MMDL_OnStatusBit( mmdl, MMDL_STABIT_RAIL_MOVE );
}

//----------------------------------------------------------------------------
/**
 *	@brief  レール用　ヘッダー反映処理    登録処理　後処理
 *
 *	@param	mmdl
 *	@param	head 
 */
//-----------------------------------------------------------------------------
void MMDL_SetRailHeaderAfter( MMDL* mmdl, const MMDL_HEADER* head )
{
  const MMDL_HEADER_RAILPOS* cp_railpos;
  MV_RAIL_SETUP_WORK* p_work;
  RAIL_LOCATION location;
  
  GF_ASSERT( head->pos_type == MMDL_HEADER_POSTYPE_RAIL );
  GF_ASSERT( MMDL_CheckStatusBit( mmdl, MMDL_STABIT_RAIL_MOVE ) );


  cp_railpos = (const MMDL_HEADER_RAILPOS*)head->pos_buf;

  // 座標の反映
  location.rail_index   = cp_railpos->rail_index;
  location.type         = FIELD_RAIL_TYPE_LINE;
  location.key          = FIELD_RAIL_TOOL_ConvertDirToRailKey(head->dir);
  location.line_grid    = cp_railpos->front_grid;
  location.width_grid   = cp_railpos->side_grid;


  // MAP CHANGEでAddMMdlされた場合、
  // MOVE_PROCの初期化が動作しないので、
  // rail_wkがない場合がある。
  //
  // この場合　画面に描画される前にReturnが呼ばれる
  if( MMDLSYS_CheckStatusBit( MMDL_GetMMdlSys(mmdl), MMDLSYS_STABIT_MOVE_INIT_COMP ) )
  {
    MMDL_SetRailLocation( mmdl, &location );
  }
  else
  {
	  p_work = MMDL_InitMoveProcWork( mmdl, MV_RAIL_SETUP_WORK_SIZE );
    MMdl_SetSetUpLocation( mmdl, &location );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  ロケーションセーブデータの有無をチェック
 *
 *	@param	mmdl  モデルデータ
 *
 *	@retval TRUE  ある
 *	@retval FALSE ない
 */
//-----------------------------------------------------------------------------
BOOL MMdl_CheckSetUpLocation( const MMDL* mmdl )
{
  const MV_RAIL_SETUP_WORK* cp_work = MMDL_GetMoveProcWork( (MMDL*)mmdl );
  if( cp_work->indata == RAIL_SETUP_DATA_MAGIC_NUMBER )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ロケーション情報の保存
 *
 *	@param	mmdl      モデルデータ
 *	@param	location  ロケーション
 */
//-----------------------------------------------------------------------------
static void MMdl_SetSetUpLocation( MMDL* mmdl, const RAIL_LOCATION* location )
{
  MV_RAIL_SETUP_WORK* p_work = MMDL_GetMoveProcWork( mmdl );

  p_work->location = *location;
  p_work->indata = RAIL_SETUP_DATA_MAGIC_NUMBER;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ロケーション情報の取得
 *
 *	@param	mmdl
 *	@param	location 
 *
 *	@retval TRUE    データがあった
 *	@retval FALSE  データがなかった
 */
//-----------------------------------------------------------------------------
BOOL MMdl_GetSetUpLocation( const MMDL* mmdl, RAIL_LOCATION* location )
{
  const MV_RAIL_SETUP_WORK* cp_work = MMDL_GetMoveProcWork( (MMDL*)mmdl );
  if( cp_work->indata == RAIL_SETUP_DATA_MAGIC_NUMBER )
  {
    *location = cp_work->location;
    return TRUE;
  }
  return FALSE;
}




