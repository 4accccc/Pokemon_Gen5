//============================================================================
/**
 *  @file   zukan_detail.h
 *  @brief  図鑑詳細画面共通
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *
 *  モジュール名：ZUKAN_DETAIL
 */
//============================================================================
#pragma once


// インクルード
#include <gflib.h>

#include "gamesystem/gamedata_def.h"


// オーバーレイ
FS_EXTERN_OVERLAY(zukan_detail);


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
typedef enum
{
  ZUKAN_DETAIL_TYPE_NONE,
  ZUKAN_DETAIL_TYPE_INFO,
  ZUKAN_DETAIL_TYPE_MAP,
  ZUKAN_DETAIL_TYPE_VOICE,
  ZUKAN_DETAIL_TYPE_FORM,
}
ZUKAN_DETAIL_TYPE;

typedef enum
{
  ZUKAN_DETAIL_RET_CLOSE,    // ×ボタン
  ZUKAN_DETAIL_RET_RETURN,   // ←┘ボタン
}
ZUKAN_DETAIL_RETURN;


//=============================================================================
/**
 *  PROC
 */
//=============================================================================
extern const GFL_PROC_DATA    ZUKAN_DETAIL_ProcData;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROC パラメータ
//=====================================
typedef struct _ZUKAN_DETAIL_PARAM
{
  GAMEDATA*            gamedata;           ///< [in,out]  GAMEDATA
  ZUKAN_DETAIL_TYPE    type;               ///< [in]      表示する詳細画面
  u16*                 list;  // ポケモンのmonsnoのリストかな？
  u16                  num;   // listの要素数
  u16                  no;    // listの何番目にあるポケモンを表示するか
  ZUKAN_DETAIL_RETURN  ret;                ///< [out]     次の指示
}
ZUKAN_DETAIL_PARAM;


//=============================================================================
/**
*  関数のプロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
/// パラメータ生成
//=====================================
extern ZUKAN_DETAIL_PARAM*  ZUKAN_DETAIL_AllocParam(
                                HEAPID               heap_id,
                                GAMEDATA*            gamedata,
                                ZUKAN_DETAIL_TYPE    type,
                                u16*                 list,
                                u16                  num,
                                u16                  no );

//-------------------------------------
/// パラメータ解放
//=====================================
extern void             ZUKAN_DETAIL_FreeParam(
                            ZUKAN_DETAIL_PARAM*  param );

//-------------------------------------
/// パラメータを設定する
//=====================================
extern void             ZUKAN_DETAIL_InitParam(
                            ZUKAN_DETAIL_PARAM*  param,
                            GAMEDATA*            gamedata,
                            ZUKAN_DETAIL_TYPE    type,
                            u16*                 list,
                            u16                  num,
                            u16                  no );

