//============================================================================
/**
 *
 *	@file		demo3d_data.h
 *	@brief  コンバートデータのアクセサ
 *	@author	hosaka genya
 *	@data		2009.11.27
 *
 */
//============================================================================
#pragma once

#include <gflib.h>

#include "demo/demo3d.h" // for DEMO3D_ID
#include "demo3d_cmd_def.h" // for DEMO3D_CMD_DATA

#include "system/ica_anime.h" // for ICA_ANIME

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータからユニット数を取得
 *
 *	@param	DEMO3D_ID id  デモID
 *
 *	@retval　ユニット数
 */
//-----------------------------------------------------------------------------
extern u8 Demo3D_DATA_GetUnitMax( DEMO3D_ID id );

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータからセットアップデータを取得
 *
 *	@param	DEMO3D_ID id  デモID
 *	@param	setup_idx     セットアップデータのインデックス
 *
 *	@retval GFL_G3D_UTIL_SETUP*
 */
//-----------------------------------------------------------------------------
extern const GFL_G3D_UTIL_SETUP* Demo3D_DATA_GetG3DUtilSetup( DEMO3D_ID id, u8 setup_idx );

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータからICAカメラを生成
 *
 *	@param	DEMO3D_ID id  デモID
 *	@param	heapID  ヒープID
 *	@param	buf_interval バッファリングする感覚
 *
 *	@retval ICA_ANIME*
 */
//-----------------------------------------------------------------------------
extern ICA_ANIME* Demo3D_DATA_CreateICACamera( DEMO3D_ID id, HEAPID heapID, int buf_interval );

//-----------------------------------------------------------------------------
/**
 *	@brief  コンバートデータからコマンドデータを取得
 *
 *	@param	DEMO3D_ID id デモID
 *
 *	@retval DEMO3D_CMD_DATA* コマンドデータ配列へのポインタ
 */
//-----------------------------------------------------------------------------
extern const DEMO3D_CMD_DATA* Demo3D_DATA_GetCmdData( DEMO3D_ID id );

