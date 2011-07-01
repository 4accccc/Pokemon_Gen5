//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_wfbc.h
 *	@brief  White Forest  Black City データ
 *	@author	tomoya takahashi
 *	@date		2009.11.09
 *
 *	モジュール名：FIELD_WFBC
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>

#include "map/dp3format.h"

#include "field_g3d_mapper.h"
#include "field_g3d_map.h"

#include "field/field_wfbc_data.h"
#include "field/eventdata_system.h"

#include "debug/debugwin_sys.h"

#include "field_status_local.h"
#include "field_buildmodel.h"

#include "fldmmdl.h"
#include "field_camera.h"

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
//-------------------------------------
///	WFBCワーク
//=====================================
typedef struct _FIELD_WFBC FIELD_WFBC;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	生成・破棄
//=====================================
extern FIELD_WFBC* FIELD_WFBC_Create( HEAPID heapID );
extern void FIELD_WFBC_Delete( FIELD_WFBC* p_wk );

//-------------------------------------
///	全体情報の取得
//=====================================
extern void FIELD_WFBC_ReadCore( FIELD_WFBC* p_wk );
extern void FIELD_WFBC_GetCore( const FIELD_WFBC* cp_wk, FIELD_WFBC_CORE* p_buff );

// 街タイプ
extern FIELD_WFBC_CORE_TYPE FIELD_WFBC_GetType( const FIELD_WFBC* cp_wk );
// 動作タイプ
extern MAPMODE FIELD_WFBC_GetMapMode( const FIELD_WFBC* cp_wk );

//-------------------------------------
///	全体情報の設定
//=====================================
extern void FIELD_WFBC_SetUp( FIELD_WFBC* p_wk, FIELD_WFBC_CORE* p_core, MAPMODE mapmode, HEAPID heapID );
extern void FIELD_WFBC_Clear( FIELD_WFBC* p_wk );



//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//  ↓このヘッダで宣言しているが実態はfield_g3d_mapper.cにある
//------------------------------------------------------------------
extern FIELD_WFBC* FLDMAPPER_GetWfbcWork( const FLDMAPPER* g3Dmapper);

//-------------------------------------
///	マッパーのWFBCワークに街情報を設定する
//=====================================
extern void FLDMAPPER_SetWfbcData( FLDMAPPER* g3Dmapper, FIELD_WFBC_CORE* p_core, MAPMODE mapmode );


#ifdef PM_DEBUG
extern void FIELD_FUNC_RANDOM_GENERATE_InitDebug( HEAPID heapId, void* p_gdata );
extern void FIELD_FUNC_RANDOM_GENERATE_TermDebug();
#endif


#ifdef _cplusplus
}	// extern "C"{
#endif



