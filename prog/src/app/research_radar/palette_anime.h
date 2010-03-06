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
  ANIME_TYPE_SIN_FADE,      // sin フェード
  ANIME_TYPE_SIN_FADE_FAST, // sin フェード ( 早 )
  ANIME_TYPE_SIN_FADE_SLOW, // sin フェード ( 遅 )
  ANIME_TYPE_BLINK,         // 点滅
  ANIME_TYPE_BLINK_SHORT,   // 点滅 ( 短 )
  ANIME_TYPE_BLINK_LONG,    // 点滅 ( 長 )
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
// アニメーションを更新する
extern void PALETTE_ANIME_Update( PALETTE_ANIME* anime );
// アニメーションを開始する
extern void PALETTE_ANIME_Start( PALETTE_ANIME* anime, ANIME_TYPE type, u16 color );
// アニメーションを停止する
extern void PALETTE_ANIME_Stop( PALETTE_ANIME* anime );
// 操作したパレットをアニメーション開始時点の状態にリセットする
extern void PALETTE_ANIME_Reset( PALETTE_ANIME* anime );
