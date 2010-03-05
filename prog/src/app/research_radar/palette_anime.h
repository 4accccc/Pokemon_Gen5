///////////////////////////////////////////////////////////////////////////////
/**
 * @brief  パレットアニメーション
 * @file   palette_anime.h
 * @author obata
 * @date   2010.03.04
 */
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>


//=============================================================================
// ■アニメーション実行ワークの不完全型
//=============================================================================
typedef struct _PALETTE_ANIME PALETTE_ANIME;


//=============================================================================
// ■定数
//============================================================================= 
// アニメーションタイプ
typedef enum {
  ANIME_TYPE_CURSOR_ON, // カーソルが乗っている状態の明滅
  ANIME_TYPE_SELECT,    // 選択時の明滅
} ANIME_TYPE;


//=============================================================================
// ■生成・破棄
//=============================================================================
// 生成
extern PALETTE_ANIME* PALETTE_ANIME_Create( HEAPID heapID );
// 破棄
extern void PALETTE_ANIME_Delete( PALETTE_ANIME* anime );
// セットアップ
extern void PALETTE_ANIME_Setup( PALETTE_ANIME* anime, u16* dest, u16* src, u8 colorNum );

//=============================================================================
// ■制御
//=============================================================================
// アニメーション更新
extern void PALETTE_ANIME_Update( PALETTE_ANIME* anime );
// アニメーション開始
extern void PALETTE_ANIME_Start( PALETTE_ANIME* anime, ANIME_TYPE type, u16 color );
// アニメーション停止
extern void PALETTE_ANIME_Stop( PALETTE_ANIME* anime );
// パレットリセット
extern void PALETTE_ANIME_Reset( PALETTE_ANIME* anime );
