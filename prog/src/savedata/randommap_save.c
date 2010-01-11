//============================================================================================
/**
 * @file	randommap_save.c
 * @brief	ランダム生成マップ用セーブデータ
 * @author	Nobuhiko Ariizumi
 * @date	2009/04/08
 *
 * モジュール名：RANDOMMAP_SAVE
 */
//============================================================================================
#include <gflib.h>

#include "savedata/randommap_save.h"
#include "savedata/save_tbl.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct


struct _RANDOMMAP_SAVE
{
  FIELD_WFBC_CORE mapdata;
  FIELD_WFBC_CORE_ITEM item;

  FIELD_WFBC_EVENT event;
};


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
int RANDOMMAP_SAVE_GetWorkSize(void)
{
	return sizeof( RANDOMMAP_SAVE );
}

void RANDOMMAP_SAVE_InitWork(RANDOMMAP_SAVE *randomMapSave)
{
  // クリア
  FIELD_WFBC_CORE_Clear( &randomMapSave->mapdata );
  WFBC_CORE_ITEM_ClaerAll( &randomMapSave->item );

}

//----------------------------------------------------------
//	データ操作のための関数
//----------------------------------------------------------
RANDOMMAP_SAVE* RANDOMMAP_SAVE_GetRandomMapSave( SAVE_CONTROL_WORK *sv )
{
	return (RANDOMMAP_SAVE*)SaveControl_DataPtrGet( sv , GMDATA_ID_RANDOMMAP );
}


//----------------------------------------------------------------------------
/**
 *	@brief  WFBC基本情報を設定
 *
 *	@param	sv        セーブワーク
 *	@param	cp_buff   基本情報
 */
//-----------------------------------------------------------------------------
void RANDOMMAP_SAVE_SetCoreWork( RANDOMMAP_SAVE* sv, const FIELD_WFBC_CORE* cp_buff )
{
  GF_ASSERT( sv );
  GF_ASSERT( cp_buff );
  GFL_STD_MemCopy( cp_buff, &sv->mapdata, sizeof(FIELD_WFBC_CORE) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  WFBC基本情報を取得する
 *
 *	@param	sv        セーブワーク
 *	@param	p_buff    バッファ
 */
//-----------------------------------------------------------------------------
void RANDOMMAP_SAVE_GetCoreWork( const RANDOMMAP_SAVE* sv, FIELD_WFBC_CORE* p_buff )
{
  GF_ASSERT( sv );
  GF_ASSERT( p_buff );
  
  GFL_STD_MemCopy( &sv->mapdata, p_buff, sizeof(FIELD_WFBC_CORE) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  アイテム配置情報を取得
 *
 *	@param	sv        ワーク
 *	@param	p_buff    格納バッファ
 */
//-----------------------------------------------------------------------------
FIELD_WFBC_CORE_ITEM* RANDOMMAP_SAVE_GetItemData( RANDOMMAP_SAVE* sv )
{
  GF_ASSERT( sv );
  return &sv->item;
}


//----------------------------------------------------------------------------
/**
 *	@brief  WFBCイベントセーブデータを取得
 *
 *	@param	sv  セーブワーク
 *
 *	@return セーブデータ
 */
//-----------------------------------------------------------------------------
FIELD_WFBC_EVENT* RANDOMMAP_SAVE_GetEventData( RANDOMMAP_SAVE* sv )
{
  GF_ASSERT( sv );
  return &sv->event;
}



