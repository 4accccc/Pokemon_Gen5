//============================================================================================
/**
 * @file	poke_regulation_ex.h
 * @brief	レギュレーション検査用プログラム、拡張
 * @author	tamada
 * @date	2011.07.28
 */
//============================================================================================
#pragma once

#include <gflib.h>

//------------------------------------------------------------------
/// レギュレーション拡張用指定
//------------------------------------------------------------------
typedef struct {
  u8 poketype;      ///<ポケモンタイプの制限
  u8 isPkwd;    ///<ポケウッド専用わざチェックのON/OFF
  u16 waza_no;      ///<使用禁止技ナンバー
}REGULATIONEX;

//------------------------------------------------------------------
/**
 * @brief レギュレーション拡張オブジェクトの生成
 *
 * 常駐メモリをあけるためにインライン関数にしてREGULATIONEXを公開しているが、
 * REGULATIONEXオブジェクトを直接操作することは禁止する
 */
//------------------------------------------------------------------
static inline REGULATIONEX * REGULATIONEX_Create( HEAPID heapID, u8 poketype, u16 waza_no, BOOL isPkwd )
{
  REGULATIONEX * rex = GFL_HEAP_AllocClearMemory( heapID, sizeof(REGULATIONEX) );
  rex->poketype = poketype;
  rex->isPkwd = isPkwd;
  rex->waza_no = waza_no;
  return rex;
}
//------------------------------------------------------------------
/**
 * @brief レギュレーション拡張オブジェクトの初期化
 *
 * 常駐メモリをあけるためにインライン関数にしてREGULATIONEXを公開しているが、
 * REGULATIONEXオブジェクトを直接操作することは禁止する
 */
//------------------------------------------------------------------
static inline void REGULATIONEX_Init( REGULATIONEX * rex, u8 poketype, u16 waza_no, BOOL isPkwd )
{
  rex->poketype = poketype;
  rex->isPkwd = isPkwd;
  rex->waza_no = waza_no;
}
//------------------------------------------------------------------
/**
 * @brief レギュレーション拡張オブジェクトの削除
 *
 * 常駐メモリをあけるためにインライン関数にしてREGULATIONEXを公開しているが、
 * REGULATIONEXオブジェクトを直接操作することは禁止する
 */
//------------------------------------------------------------------
static inline void REGULATIONEX_Delete( REGULATIONEX * rex )
{
  GFL_HEAP_FreeMemory( rex );
}

