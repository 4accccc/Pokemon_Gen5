//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_wfbc_data_local.c
 *	@brief  White Forest  Black City 基本データ 非常駐 [OVERLAY FIELD_INIT]
 *	@author	tomoya takahashi
 *	@date		2010.01.12
 *
 *	モジュール名：FIELD_WFBC_CORE
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "gflib.h"


#include "system/main.h"

#include "arc/arc_def.h"

#include "system/gfl_use.h"

#include "field/field_wfbc_data.h"
#include "field/zonedata.h"

#include "field_status_local.h"

#include "eventwork_def.h"

#include "field_comm/intrude_work.h"
#include "savedata/outside_permanent.h"

#include "net/dwc_tool.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 day*					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	FIELD_WFBC_CORE用関数
//=====================================

//----------------------------------------------------------------------------
/**
 *	@brief  街の情報を設定する
 *
 *	@param	p_wk          ワーク
 *	@param  cp_mystatus   まいステータス
 *	@param  heapID        テンポラリバッファ用ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_CORE_SetUp( FIELD_WFBC_CORE* p_wk, const MYSTATUS* cp_mystatus, SAVE_CONTROL_WORK *p_sv, HEAPID tmp_heapID )
{
  OUTSIDE_PERMANENT *p_per =  SaveControl_GetOutsidePermanent( p_sv );
  KEY_SYSTEM_SAVE *p_key = OUTSIDE_PERMANENT_GetKeySystem( p_per );
  KEY_SYSTEM_WFBC wfbc_type = KEY_SYSTEM_SAVE_GetWfbc( p_key );

  GF_ASSERT( p_wk );

#ifdef PM_DEBUG
  OS_SetPrintOutput_Arm9( 2 );
#endif
  
  TOMOYA_Printf( "WFBC_SETUP\n" );
  
  p_wk->data_in = TRUE;
#if PM_VERSION == VERSION_B2
  if( wfbc_type == KEY_SYSTEM_WFBC_SELF )
  {
    p_wk->type    = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
  }
  else
  {
    p_wk->type    = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
  }
#else   // PM_VERSION == VERSION_B2
  if( wfbc_type == KEY_SYSTEM_WFBC_SELF )
  {
    p_wk->type    = FIELD_WFBC_CORE_TYPE_WHITE_FOREST; 
  }
  else
  {
    p_wk->type    = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
  }
#endif  // PM_VERSION == VERSION_B2

#ifdef PM_DEBUG
  OS_SetPrintOutput_Arm9( 0 );
#endif
}


//----------------------------------------------------------------------------
/**
 *	@brief  WFBCデータの整合性チェック
 *
 *	@param	cp_wk         ワーク
 *
 *	@retval TRUE    正常
 *	@retval FALSE   不正
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_CORE_IsConfomity( const FIELD_WFBC_CORE* cp_wk )
{
  GF_ASSERT( cp_wk );
  
  if( (cp_wk->data_in == TRUE) && (cp_wk->type >= FIELD_WFBC_CORE_TYPE_MAX) )
  {
    return FALSE;
  }

  // OK
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  データ調整  不正データの場合は、正常な情報に書き換えます。
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_WFBC_CORE_Management( FIELD_WFBC_CORE* p_wk, const MYSTATUS* cp_mystatus )
{
  int i;

  GF_ASSERT( p_wk );

  // TRUE以上の値がはいっているばあい。
  if( p_wk->data_in > TRUE ){
    p_wk->data_in = TRUE;
  }
  
  // 一応データがない状態も管理しておく
  if( (p_wk->data_in == FALSE) && (p_wk->type != FIELD_WFBC_CORE_TYPE_MAX) )
  {
    p_wk->type = FIELD_WFBC_CORE_TYPE_MAX;
    TOMOYA_Printf( "OverWrite max\n" );
  }

  // 街タイプの調整
  if( (p_wk->data_in == TRUE) && (p_wk->type >= FIELD_WFBC_CORE_TYPE_MAX) )
  {
    u32 pm_version, trainerID;

    pm_version  = MyStatus_GetRomCode( cp_mystatus );
    trainerID   = MyStatus_GetID( cp_mystatus );
    
    // 不正データ 自分のROMコードに合わせる
    // 下記関数はVERSION_B2, W2は返さないVERSION_BLACKorVERSION_WHITEのみ
    if( Intrude_GetIntrudeRomVersion( pm_version, trainerID ) == VERSION_BLACK ){
      p_wk->type = FIELD_WFBC_CORE_TYPE_BLACK_CITY;
    }else{
      p_wk->type = FIELD_WFBC_CORE_TYPE_WHITE_FOREST;
    }

    TOMOYA_Printf( "OverWrite city\n" );
  }
}

// データの有無   不正データの場合、FALSEを返します。
//----------------------------------------------------------------------------
/**
 *	@brief  データの有無チェック  不正データの場合、FALSEを返します。
 *
 *	@param	cp_wk   ワーク
 *
 *	@retval TRUE  データあり
 *	@retval FALSE データなし
 */
//-----------------------------------------------------------------------------
BOOL FIELD_WFBC_CORE_IsInData( const FIELD_WFBC_CORE* cp_wk )
{
  GF_ASSERT( cp_wk );
  
  if( !cp_wk->data_in )
  {
    return FALSE;
  }

  if( FIELD_WFBC_CORE_IsConfomity( cp_wk ) == FALSE )
  {
    return FALSE;
  }

  return TRUE;
}
