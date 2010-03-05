///////////////////////////////////////////////////////////////////////////////
/**
 * @brief  パレットアニメーション
 * @file   palette_anime.c
 * @author obata
 * @date   2010.03.04
 */
///////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "palette_anime.h"
#include "system/palanm.h"


//=============================================================================
// ■定数
//============================================================================= 
// カーソルONの明滅
#define CURSOR_ON_ANIME_ONE_CYCLE_FRAMES (120) // 明滅1回のフレーム数
#define CURSOR_ON_ANIME_MIN_EVY            (0) // 最小フェード係数１
#define CURSOR_ON_ANIME_MAX_EVY            (4) // 最大フェード係数

// 選択時の明滅
#define SELECT_ANIME_FRAMES      (20) // アニメーションフレーム数
#define SELECT_ANIME_BLINK_FRAMES (4) // 点滅待ちフレーム数
#define SELECT_ANIME_MIN_EVY      (0) // 最小フェード係数１
#define SELECT_ANIME_MAX_EVY     (10) // 最大フェード係数



//=============================================================================
// ■パレットアニメーション実行ワーク
//=============================================================================
struct _PALETTE_ANIME
{
  HEAPID heapID;

  BOOL setupFlag;     // セットアップされているかどうか
  u16* transDest;     // 転送先アドレス
  u8   colorNum;      // アニメーションするカラー数 ( パレット1本 = 16 )
  u16* originalColor; // アニメーション元データ
  u16  fadeColor;     // フェードカラー

  BOOL       animeFlag;  // アニメーション中かどうか
  ANIME_TYPE animeType;  // アニメーションタイプ
  u32        frameCount; // フレーム数カウンタ
};




//=============================================================================
// ■動作・制御
//=============================================================================
static void StartAnime( PALETTE_ANIME* work, ANIME_TYPE type, u16 color ); // アニメーションを開始する
static void StopAnime( PALETTE_ANIME* work ); // アニメーションを停止する
static void UpdateAnime( PALETTE_ANIME* work ); // アニメーションを更新する
static void UpdateAnime_CURSOR_ON( PALETTE_ANIME* work ); // アニメーションを更新する ( カーソルONの明滅 )
static void UpdateAnime_SELECT( PALETTE_ANIME* work ); // アニメーションを更新する ( 選択時の明滅 )
static void ResetPalette( PALETTE_ANIME* work ); // パレットをリセットする
//=============================================================================
// ■生成・破棄
//=============================================================================
static PALETTE_ANIME* CreateWork( HEAPID heapID ); // ワークを生成する
static void DeleteWork( PALETTE_ANIME* work ); // ワークを破棄する
static void InitWork( PALETTE_ANIME* work ); // ワークを初期化する
static void SetupWork( PALETTE_ANIME* work, u16* dest, u16* src, u8 colorNum ); // ワークをセットアップする
static void CleanUpWork( PALETTE_ANIME* work ); // ワークをクリーンアップする




//=============================================================================
// ■生成・破棄
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを生成する
 *
 * @param heapID 使用するヒープID
 * @return 生成したアニメーションワーク
 */
//-----------------------------------------------------------------------------
PALETTE_ANIME* PALETTE_ANIME_Create( HEAPID heapID )
{
  PALETTE_ANIME* work;

  work = CreateWork( heapID ); // 生成
  InitWork( work );            // 初期化

  return work;
}

//-----------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを破棄する
 *
 * @param work 破棄するワーク
 */
//-----------------------------------------------------------------------------
void PALETTE_ANIME_Delete( PALETTE_ANIME* work )
{
  CleanUpWork( work ); // クリーンアップ
  DeleteWork( work );  // 破棄
}

//-----------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをセットアップする
 *
 * @param work
 * @param dest     パレットデータの転送先アドレス
 * @param src      アニメーションの元データのアドレス
 * @param colorNum アニメーションする色の数 ( パレット1本 = 16 )
 */
//-----------------------------------------------------------------------------
void PALETTE_ANIME_Setup( PALETTE_ANIME* work, u16* dest, u16* src, u8 colorNum )
{
  SetupWork( work, dest, src, colorNum );
}

//=============================================================================
// ■制御
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void PALETTE_ANIME_Update( PALETTE_ANIME* work )
{
  UpdateAnime( work );
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを開始する
 *
 * @param work
 * @param type 開始するアニメーションを指定
 * @param color フェードカラー
 */
//-----------------------------------------------------------------------------
void PALETTE_ANIME_Start( PALETTE_ANIME* work, ANIME_TYPE type, u16 color )
{
  StartAnime( work, type, color );
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを停止する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void PALETTE_ANIME_Stop( PALETTE_ANIME* work )
{
  StopAnime( work );
}

//-----------------------------------------------------------------------------
/**
 * @brief 操作していたパレットをリセットする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
void PALETTE_ANIME_Reset( PALETTE_ANIME* work )
{
  ResetPalette( work );
}



//=============================================================================
// ■動作・制御
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを開始する
 *
 * @param work
 * @param type  開始するアニメーションを指定
 * @param color フェードカラー
 */
//-----------------------------------------------------------------------------
static void StartAnime( PALETTE_ANIME* work, ANIME_TYPE type, u16 color )
{
  GF_ASSERT( work->setupFlag ); // セットアップされていない

  work->animeFlag  = TRUE;
  work->animeType  = type;
  work->frameCount = 0;
  work->fadeColor  = color;
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを停止する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void StopAnime( PALETTE_ANIME* work )
{
  GF_ASSERT( work->setupFlag ); // セットアップされていない

  work->animeFlag = FALSE;
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを更新する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateAnime( PALETTE_ANIME* work )
{
  if( work->setupFlag == FALSE ) { return; } // 未セットアップ
  if( work->animeFlag == FALSE ) { return; } // アニメーション停止中

  // アニメタイプに応じた更新処理
  switch( work->animeType ) {
  case ANIME_TYPE_CURSOR_ON: UpdateAnime_CURSOR_ON( work ); break;
  case ANIME_TYPE_SELECT:    UpdateAnime_SELECT( work );    break;
  default: GF_ASSERT(0);
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを更新する ( カーソルONの明滅 )
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateAnime_CURSOR_ON( PALETTE_ANIME* work )
{
  u8 evy;

  GF_ASSERT( work->animeFlag ); // アニメーション停止中
  GF_ASSERT( work->setupFlag ); // セットアップされていない

  // フェード係数を決定
  {
    int idx;
    float sinVal;
    idx = (work->frameCount * 65536 / CURSOR_ON_ANIME_ONE_CYCLE_FRAMES) % 65536;
    sinVal = FX_FX16_TO_F32( FX_SinIdx( idx ) );
    evy = (int)( sinVal * 16 + 16 );
    evy = CURSOR_ON_ANIME_MIN_EVY + (CURSOR_ON_ANIME_MAX_EVY - CURSOR_ON_ANIME_MIN_EVY) * evy / 32;
  }

  // パレット書き換え
  SoftFade( work->originalColor, work->transDest, work->colorNum, evy, work->fadeColor ); 

  // アニメーションフレーム数をインクリメント
  work->frameCount++;
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを更新する ( 選択時の明滅 )
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateAnime_SELECT( PALETTE_ANIME* work )
{
  u8 evy;

  GF_ASSERT( work->animeFlag ); // アニメーション停止中
  GF_ASSERT( work->setupFlag ); // セットアップされていない

  // フェード係数を決定
  if( ((work->frameCount / SELECT_ANIME_BLINK_FRAMES) % 2) == 0 ) {
    evy = SELECT_ANIME_MIN_EVY;
  }
  else {
    evy = SELECT_ANIME_MAX_EVY;
  }

  // パレット書き換え
  SoftFade( work->originalColor, work->transDest, work->colorNum, evy, work->fadeColor ); 

  // アニメーションフレーム数をインクリメント
  work->frameCount++;

  // アニメーション終了
  if( SELECT_ANIME_FRAMES < work->frameCount )
  {
    work->animeFlag = FALSE;
    ResetPalette( work );
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief パレットをリセットする
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void ResetPalette( PALETTE_ANIME* work )
{
  GF_ASSERT( work->setupFlag ); // セットアップされていない

  MI_CpuCopy8( work->originalColor, work->transDest, sizeof(u16) * work->colorNum );
}


//=============================================================================
// ■生成・破棄
//=============================================================================

//-----------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを生成する
 *
 * @param heapID 使用するヒープID
 * @return 生成したワーク
 */
//-----------------------------------------------------------------------------
static PALETTE_ANIME* CreateWork( HEAPID heapID )
{
  PALETTE_ANIME* work;

  work = GFL_HEAP_AllocMemory( heapID, sizeof(PALETTE_ANIME) );
  work->heapID = heapID;
  return work;
}

//-----------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを破棄する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void DeleteWork( PALETTE_ANIME* work )
{
  // クリーンアップがされていない
  GF_ASSERT( work->setupFlag == FALSE );

  GFL_HEAP_FreeMemory( work );
}

//-----------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークを初期化する
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void InitWork( PALETTE_ANIME* work )
{
  HEAPID heapID;

  // ヒープIDを退避
  heapID = work->heapID;

  // 0クリア
  GFL_STD_MemClear( work, sizeof(PALETTE_ANIME) );

  // ヒープIDを復帰
  work->heapID = heapID;
}

//-----------------------------------------------------------------------------
/**
 * @brief パレットアニメーションワークをセットアップする
 *
 * @param work
 * @param dest     パレットデータの転送先アドレス
 * @param src      アニメーションの元データのアドレス
 * @param colorNum アニメーションする色の数 ( パレット1本 = 16 )
 */
//-----------------------------------------------------------------------------
static void SetupWork( PALETTE_ANIME* work, u16* dest, u16* src, u8 colorNum )
{
  int i;

  // すでにセットアップされている
  GF_ASSERT( work->setupFlag == FALSE ); 

  work->setupFlag = TRUE;
  work->transDest = dest;
  work->colorNum  = colorNum;

  // 元データのワークを生成
  work->originalColor = GFL_HEAP_AllocMemory( work->heapID, sizeof(u16) * colorNum );

  // 元データをコピー
  for( i=0; i < colorNum; i++ )
  {
    work->originalColor[i] = src[i];
  }
}

//-----------------------------------------------------------------------------
 /**
  * @brief パレットアニメーションワークをクリーンアップする
  *
  * @param work
  */
//-----------------------------------------------------------------------------
static void CleanUpWork( PALETTE_ANIME* work )
{
  // セットアップされていない
  GF_ASSERT( work->setupFlag );

  // 元データのワークを破棄
  GFL_HEAP_FreeMemory( work->originalColor );

  // ワークを初期化
  InitWork( work );
}
