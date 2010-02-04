//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		tr2dgra.h
 *	@brief	トレーナーOBJ,BGグラフィック
 *	@author	sogabe
 *	@date		2010.02.02
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "poke_tool/poke_tool.h"
#include "system/trgra.h"
#include "app/app_menu_common.h"
#include "poke_tool/monsno_def.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	キャラサイズ
//=====================================

//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//-------------------------------------
///	BG関係
//=====================================
//モンスター番号等から読む場合
extern void TR2DGRA_BG_TransResource( int trtype, u32 frm, u32 chr_offs, u32 plt_offs, HEAPID heapID );
extern GFL_ARCUTIL_TRANSINFO TR2DGRA_BG_TransResourceByAreaMan( int trtype, u32 frm, u32 plt_offs, HEAPID heapID );
//スクリーン貼り付け
extern void TR2DGRA_BG_WriteScreen( u32 frm, u32 chr_offs, u32 pal_offs, u16 x, u16 y );
//-------------------------------------
///	OBJ関係
//=====================================
//TRグラのアークハンドル
extern ARCHANDLE *TR2DGRA_OpenHandle( HEAPID heapID );
extern u32 TR2DGRA_OBJ_PLTT_Register( ARCHANDLE *p_handle, int trtype, CLSYS_DRAW_TYPE vramType, u16 byteOffs, HEAPID heapID );
extern u32 TR2DGRA_OBJ_CGR_Register( ARCHANDLE *p_handle, int trtype, CLSYS_DRAW_TYPE vramType, HEAPID heapID );
extern u32 TR2DGRA_OBJ_CELLANM_Register( int trtype, APP_COMMON_MAPPING mapping, CLSYS_DRAW_TYPE vramType, HEAPID heapID );
extern void TR2DGRA_OBJ_RES_Replace( ARCHANDLE *p_handle, int trtype, int cgr_idx, int pltt_idx, HEAPID heapID );

