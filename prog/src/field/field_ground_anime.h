//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_ground_anime.h
 *	@brief	フィールド地面アニメーション管理システム
 *	@author	tomoya takahshi
 *	@date		2009.04.30
 *
 *	モジュール名：FIELD_GRANM
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif

#include <gflib.h>

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
///	フィールド地面アニメーション管理システム
//=====================================
typedef struct _FIELD_GRANM FIELD_GRANM;

//-------------------------------------
///	フィールド地面アニメーションワーク
//=====================================
typedef struct _FIELD_GRANM_WORK FIELD_GRANM_WORK;

//-------------------------------------
///	初期化データ
//=====================================
typedef struct {
	u16 ita_use:8;
	u16 itp_use:8;
	u16 block_num;
	u16 ita_arcID;			// itaアニメーション
	u16 ita_dataID;
	u16 itp_arcID;			// itpアニメーション　テーブルデータ
	u16 itp_tblID;
	u16 itp_itparcID;		// itpアニメーション　アニメデータ
	u16 itp_texarcID;		// itpアニメーション　テクスチャデータ
} FIELD_GRANM_SETUP;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	フィールド地面アニメーション管理システム
//	操作関数
//=====================================
extern FIELD_GRANM* FIELD_GRANM_Create( const FIELD_GRANM_SETUP* cp_setup, const GFL_G3D_RES* cp_tex, HEAPID heapID );
extern void FIELD_GRANM_Delete( FIELD_GRANM* p_sys );
extern void FIELD_GRANM_Main( FIELD_GRANM* p_sys );
extern FIELD_GRANM_WORK* FIELD_GRANM_GetWork( const FIELD_GRANM* cp_sys, u32 idx );
extern void FIELD_GRANM_SetAnimeSpeed( FIELD_GRANM* p_sys, fx32 speed );
extern fx32 FIELD_GRANM_GetAnimeSpeed( const FIELD_GRANM* cp_sys );
extern void FIELD_GRANM_SetAutoAnime( FIELD_GRANM* p_sys, BOOL flag );
extern BOOL FIELD_GRANM_GetAutoAnime( const FIELD_GRANM* cp_sys );


//-------------------------------------
///	フィールド地面アニメーションワーク
//	操作関数
//=====================================
extern void FIELD_GRANM_WORK_Bind( FIELD_GRANM_WORK* p_wk, const GFL_G3D_RES* cp_mdlres, const GFL_G3D_RES* cp_texres, NNSG3dRenderObj* p_rendobj );
extern void FIELD_GRANM_WORK_Release( FIELD_GRANM_WORK* p_wk );



#ifdef _cplusplus
}	// extern "C"{
#endif



