//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_data.h
 *	@brief  データバッファ
 *	@author	Toru=Nagihashi
 *	@date		2009.12.01
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "poke_tool/pokeparty.h"
#include "buflen.h"
#include "system/pms_data.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ワーク
//=====================================
typedef struct _WIFIBATTLEMATCH_DATA_WORK WIFIBATTLEMATCH_DATA_WORK;

//-------------------------------------
/// 保存データ
//=====================================

//対戦者情報
typedef struct 
{
  STRCODE   name[PERSON_NAME_SIZE+EOM_SIZE];
  u8        sex;
  u8        trainer_view;
  u8        nation;
  u8        area;
  u32       rate;
  PMS_DATA  pms;
  u32       pokeparty[0]; //後尾にポケパーティのデータがつく
} WIFIBATTLEMATCH_ENEMYDATA;
//対戦者情報のサイズ
#define WIFIBATTLEMATCH_DATA_ENEMYDATA_SIZE (sizeof(WIFIBATTLEMATCH_ENEMYDATA) + PokeParty_GetWorkSize())

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//ワーク作成
extern WIFIBATTLEMATCH_DATA_WORK *WIFIBATTLEMATCH_DATA_Init( HEAPID heapID );
extern void WIFIBATTLEMATCH_DATA_Exit( WIFIBATTLEMATCH_DATA_WORK *p_wk );

//データ取得、設定
extern WIFIBATTLEMATCH_ENEMYDATA * WIFIBATTLEMATCH_DATA_GetPlayerDataPtr( WIFIBATTLEMATCH_DATA_WORK *p_wk );
extern WIFIBATTLEMATCH_ENEMYDATA * WIFIBATTLEMATCH_DATA_GetEnemyDataPtr( WIFIBATTLEMATCH_DATA_WORK *p_wk );
