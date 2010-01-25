//============================================================================
/**
 *  @file   shinka_demo.h
 *  @brief  進化デモ
 *  @author Koji Kawada
 *  @data   2010.01.13
 *  @note   
 *
 *  モジュール名：SHINKADEMO
 */
//============================================================================
#pragma once

#include "poke_tool/pokeparty.h"
#include "gamesystem/gamedata_def.h"

// オーバーレイ
FS_EXTERN_OVERLAY(shinka_demo);


//=============================================================================
/**
 *  PROC
 */
//=============================================================================
extern  const GFL_PROC_DATA   ShinkaDemoProcData;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROC パラメータ
//=====================================
typedef struct _SHINKA_DEMO_PARAM
{
  GAMEDATA*         gamedata;           ///< [in,out]  GAMEDATA
  const POKEPARTY*  ppt;                ///< [in]      POKEPARTY構造体
  u16               after_mons_no;      ///< [in]      進化後のポケモンナンバー
  u8                shinka_pos;         ///< [in]      進化するポケモンのPOKEPARTY内のインデックス
  u8                shinka_cond;        ///< [in]      進化条件（ヌケニンチェックに使用）
  BOOL              b_field;            ///< [in]      フィールドから呼び出すときはTRUE、バトル後に呼び出すときはFALSE
  BOOL              b_enable_cancel;    ///< [in]      進化キャンセルできるときはTRUE、できないときはFALSE
}
SHINKA_DEMO_PARAM;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// 進化デモ用パラメータ構造体生成
//=====================================
extern  SHINKA_DEMO_PARAM*  SHINKADEMO_AllocParam(
                                HEAPID heapID, GAMEDATA* gamedata, const POKEPARTY* ppt,
                                u16 after_mons_no, u8 pos, u8 cond, BOOL b_field );

//-------------------------------------
/// 進化デモ用パラメータワーク解放
//=====================================
extern  void                SHINKADEMO_FreeParam( SHINKA_DEMO_PARAM* sdp );

//-------------------------------------
/// 進化デモ用パラメータを設定する
//=====================================
extern  void                SHINKADEMO_InitParam(
                                SHINKA_DEMO_PARAM* param,
                                GAMEDATA* gamedata, const POKEPARTY* ppt,
                                u16 after_mons_no, u8 pos, u8 cond, BOOL b_field, BOOL b_enable_cancel );
