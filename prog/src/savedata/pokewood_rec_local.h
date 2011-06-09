//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		pokewood_rec_local.h
 *	@brief  ポケウッド用戦闘録画セーブデータ直接アクセス用
 *	@author	Toru=Nagihashi
 *	@date		2011.06.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "battle/btl_common.h"
#include "savedata/battle_rec.h"
#include "battle/battle.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/poke_tool_def.h"
#include "battle_rec_local.h"
#include "savedata/pokewood_rec.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

///録画データの存在有無
#define POKEWOOD_REC_MAGIC_KEY   (0xF910)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ポケウッド操作内容保存バッファ
//=====================================
struct _POKEWOOD_REC_OPERATION_BUFFER
{
  u32  size;
  u8   buffer[ POKEWOOD_REC_OPERATION_BUFFER_SIZE ];
};

//-------------------------------------
///	ポケウッド操作内容保存バッファ
//=====================================
typedef struct
{
  int   senario_no;
  BOOL  success;
  u8    choiceTbl[ BTL_POKEWOOD_CHOICE_MAX ];
}POKEWOOD_REC_SENARIO;

//-------------------------------------
///	ポケウッド操作内容保存バッファ
//=====================================
typedef struct
{
  /// 保持できるポケモン数の最大
  u16 PokeCountMax;
  /// 現在保持しているポケモン数
  u16 PokeCount;
  /// ポケモンデータ
  REC_POKEPARA member[POKEWOOD_REC_TEMOTI_MAX];
}POKEWOOD_REC_POKEPARTY;

//-------------------------------------
///	ポケウッド戦闘録画本体
//=====================================
struct _POKEWOOD_REC_WORK
{
  BTLREC_SETUP_SUBSET       setupSubset;    ///< バトル画面セットアップパラメータのサブセット
  POKEWOOD_REC_SENARIO      senario;        ///< ポケウッドシナリオデータ
  POKEWOOD_REC_OPERATION_BUFFER   opBuffer; ///< クライアント操作内容の保存バッファ

  POKEWOOD_REC_POKEPARTY  rec_party[ POKEWOOD_REC_CLIENT_MAX ];
  BTLREC_CLIENT_STATUS  clientStatus[ POKEWOOD_REC_CLIENT_MAX ];
};
