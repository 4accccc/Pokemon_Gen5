//======================================================================
/**
 * @file	field_status.c
 * @brief フィールドマップ管理情報
 * @author  tamada GAMEFREAK inc.
 * @date    2009.10.03
 */
//======================================================================

#include <gflib.h>
#include "field/field_status.h"


#include "field_status_local.h"   //MAPMODE

#include "script.h" //SCRID_NULL

//--------------------------------------------------------------
//--------------------------------------------------------------

//======================================================================
//======================================================================
//--------------------------------------------------------------
//--------------------------------------------------------------
struct _FIELD_STATUS{
  MAPMODE map_mode;
  u8 isFieldInitFlag;
  u8 isContinueFlag;

  // フィールド技関連
  u8  fs_flash;      // フラッシュON
  u16 fs_mapeffect;  // 見た目の状態

  // 季節表示
  BOOL seasonDispFlag;  // 表示フラグ
  u8   seasonDispLast;  // 最後に表示した季節 PMSEASON_xxxx

  u8 proc_action;       // フィールド中か、戦闘中か、、等 PROC_ACTION_xxxx
  
  ///予約スクリプトID
  u16 reserved_script_id;
};

//======================================================================
//======================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
FIELD_STATUS * FIELD_STATUS_Create(HEAPID heapID)
{
  FIELD_STATUS * fldstatus;
  fldstatus = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_STATUS) );
  fldstatus->reserved_script_id = SCRID_NULL;
  return fldstatus;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_STATUS_Delete(FIELD_STATUS * fldstatus)
{
  GFL_HEAP_FreeMemory( fldstatus );
}


//======================================================================
//======================================================================
//------------------------------------------------------------------
/**
 * マップモード取得
 *
 * @param   gamedata		GAMEDATAへのポインタ
 *
 * @retval  MAPMODE		マップモード
 */
//------------------------------------------------------------------
MAPMODE FIELD_STATUS_GetMapMode(const FIELD_STATUS * fldstatus)
{
  return fldstatus->map_mode;
}

//------------------------------------------------------------------
/**
 * マップモード設定
 *
 * @param   gamedata		GAMEDATAへのポインタ
 * @param   map_mode		マップモード
 */
//------------------------------------------------------------------
void FIELD_STATUS_SetMapMode(FIELD_STATUS * fldstatus, MAPMODE map_mode)
{
  fldstatus->map_mode = map_mode;
}

//======================================================================
//======================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_STATUS_SetFieldInitFlag( FIELD_STATUS * fldstatus, BOOL flag )
{
  fldstatus->isFieldInitFlag = flag;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL FIELD_STATUS_GetFieldInitFlag( const FIELD_STATUS * fldstatus )
{
  return fldstatus->isFieldInitFlag;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void FIELD_STATUS_SetContinueFlag( FIELD_STATUS * fldstatus, BOOL flag )
{
  fldstatus->isContinueFlag = flag;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
BOOL FIELD_STATUS_GetContinueFlag( const FIELD_STATUS * fldstatus )
{
  return fldstatus->isContinueFlag;
}


//------------------------------------------------------------------
// フィールド技　フラッシュのON・OFF
//------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  フィールド技　フラッシュ　ON・OFF
 *
 *	@param	fldstatus   ワーク
 *	@param	flag        TRUE:ON   FALSE：OFF
 */
//-----------------------------------------------------------------------------
void FIELD_STATUS_SetFieldSkillFlash( FIELD_STATUS * fldstatus, BOOL flag )
{
  GF_ASSERT( fldstatus );
  fldstatus->fs_flash = flag;
}


//----------------------------------------------------------------------------
/**
 *	@brief  フィールド技　フラッシュ　チェック
 *
 *	@param	fldstatus   ワーク
 *
 *	@retval TRUE    フラッシュ状態
 *	@retval FALSE   フラッシュしてない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_STATUS_IsFieldSkillFlash( const FIELD_STATUS * fldstatus )
{
  GF_ASSERT( fldstatus );
  return fldstatus->fs_flash;
}

//------------------------------------------------------------------
// フィールド技　マップ効果状態
//------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  フィールド技　マップ効果状態マスク　を設定
 *
 *	@param	fldstatus   ワーク
 *	@param	msk         状態マスク
 */
//-----------------------------------------------------------------------------
void FIELD_STATUS_SetFieldSkillMapEffectMsk( FIELD_STATUS * fldstatus, u32 msk )  
{
  GF_ASSERT( fldstatus );
  fldstatus->fs_mapeffect = msk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールド技　マップ効果状態マスクを　取得
 *
 *	@param	fldstatus   ワーク
 *
 *	@return マスク
 */
//-----------------------------------------------------------------------------
u32 FIELD_STATUS_GetFieldSkillMapEffectMsk( const FIELD_STATUS * fldstatus )
{
  GF_ASSERT( fldstatus );
  return fldstatus->fs_mapeffect;
}

//------------------------------------------------------------------
/**
 * @brief 予約スクリプトIDの登録
 * @param fldstatus   ワーク
 * @param scr_id    登録するスクリプトのID
 */
//------------------------------------------------------------------
void FIELD_STATUS_SetReserveScript( FIELD_STATUS * fldstatus, u16 scr_id )
{
  fldstatus->reserved_script_id = scr_id;
}

//------------------------------------------------------------------
/**
 * @brief 予約スクリプトIDの取得
 * @param fldstatus
 * @retval  u16   登録したスクリプトのID
 */
//------------------------------------------------------------------
u16 FIELD_STATUS_GetReserveScript( const FIELD_STATUS * fldstatus )
{
  return fldstatus->reserved_script_id;
}

//==================================================================
/**
 * PROCアクションをセット
 *
 * @param   fldstatus		
 * @param   action		    PROC_ACTION_xxxx
 *
 * @retval  PROC_ACTION		セット前のPROCアクション
 */
//==================================================================
PROC_ACTION FIELD_STATUS_SetProcAction( FIELD_STATUS * fldstatus, PROC_ACTION action)
{
  PROC_ACTION old_action = fldstatus->proc_action;
  fldstatus->proc_action = action;
  return old_action;
}

//==================================================================
/**
 * PROCアクションを取得
 *
 * @param   fldstatus		
 *
 * @retval  PROC_ACTION		PROC_ACTION_xxxx
 */
//==================================================================
PROC_ACTION FIELD_STATUS_GetProcAction( const FIELD_STATUS * fldstatus )
{
  return fldstatus->proc_action;
}

