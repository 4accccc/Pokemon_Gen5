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
// sin フェード
#define SIN_FADE_ONE_CYCLE_FRAMES (120) // 明滅1回のフレーム数
#define SIN_FADE_MIN_EVY            (0) // 最小フェード係数[0, 16]
#define SIN_FADE_MAX_EVY            (3) // 最大フェード係数[0, 16]
// sin フェード ( 早 )
#define SIN_FADE_FAST_ONE_CYCLE_FRAMES (100) // 明滅1回のフレーム数
#define SIN_FADE_FAST_MIN_EVY            (0) // 最小フェード係数[0, 16]
#define SIN_FADE_FAST_MAX_EVY            (3) // 最大フェード係数[0, 16]
// sin フェード ( 遅 )
#define SIN_FADE_SLOW_ONE_CYCLE_FRAMES (140) // 明滅1回のフレーム数
#define SIN_FADE_SLOW_MIN_EVY            (0) // 最小フェード係数[0, 16]
#define SIN_FADE_SLOW_MAX_EVY            (3) // 最大フェード係数[0, 16]
// 点滅
#define BLINK_FRAMES      (20) // アニメーションフレーム数
#define BLINK_WAIT_FRAMES  (4) // 点滅待ちフレーム数
#define BLINK_MIN_EVY      (0) // 最小フェード係数[0, 16]
#define BLINK_MAX_EVY      (6) // 最大フェード係数[0, 16]
// 点滅 ( 短 )
#define BLINK_SHORT_FRAMES      (10) // アニメーションフレーム数
#define BLINK_SHORT_WAIT_FRAMES  (4) // 点滅待ちフレーム数
#define BLINK_SHORT_MIN_EVY      (0) // 最小フェード係数[0, 16]
#define BLINK_SHORT_MAX_EVY      (6) // 最大フェード係数[0, 16]
// 点滅 ( 長 )
#define BLINK_LONG_FRAMES      (30) // アニメーションフレーム数
#define BLINK_LONG_WAIT_FRAMES  (4) // 点滅待ちフレーム数
#define BLINK_LONG_MIN_EVY      (0) // 最小フェード係数[0, 16]
#define BLINK_LONG_MAX_EVY      (6) // 最大フェード係数[0, 16]


//=============================================================================
// ■パレットアニメーション実行ワーク
//=============================================================================
struct _PALETTE_ANIME
{
  HEAPID heapID;

  BOOL setupFlag;     // セットアップされているかどうか
  u16* transDest;     // 転送先アドレス
  u16* originalColor; // アニメーション元データ
  u8   colorNum;      // アニメーションするカラー数 ( パレット1本 = 16 )
  u16  fadeColor;     // フェードカラー

  BOOL       animeFlag;  // アニメーション中かどうか
  ANIME_TYPE animeType;  // アニメーションタイプ
  u32        frameCount; // フレーム数カウンタ
};




//=============================================================================
// ■関数インデックス
//=============================================================================
//-----------------------------------------------------------------------------
// ◆LAYER 2 アニメーション
//-----------------------------------------------------------------------------
static void UpdateAnime( PALETTE_ANIME* work ); // アニメーションを更新する
static void UpdateAnime_SIN_FADE( PALETTE_ANIME* work ); // アニメーションを更新する ( sin フェード )
static void UpdateAnime_SIN_FADE_FAST( PALETTE_ANIME* work ); // アニメーションを更新する ( sin フェード・早 )
static void UpdateAnime_SIN_FADE_SLOW( PALETTE_ANIME* work ); // アニメーションを更新する ( sin フェード・遅 )
static void UpdateAnime_BLINK( PALETTE_ANIME* work ); // アニメーションを更新する ( 点滅 )
static void UpdateAnime_BLINK_SHORT( PALETTE_ANIME* work ); // アニメーションを更新する ( 点滅・短 )
static void UpdateAnime_BLINK_LONG( PALETTE_ANIME* work ); // アニメーションを更新する ( 点滅・長 )
//-----------------------------------------------------------------------------
// ◆LAYER 1 制御
//-----------------------------------------------------------------------------
static void StartAnime( PALETTE_ANIME* work, ANIME_TYPE type, u16 color ); // アニメーションを開始する
static void StopAnime( PALETTE_ANIME* work ); // アニメーションを停止する
static void ResetPalette( PALETTE_ANIME* work ); // パレットをリセットする
//-----------------------------------------------------------------------------
// ◆LAYER 0 生成・破棄
//-----------------------------------------------------------------------------
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
  case ANIME_TYPE_SIN_FADE:      UpdateAnime_SIN_FADE( work );      break;
  case ANIME_TYPE_SIN_FADE_FAST: UpdateAnime_SIN_FADE_FAST( work ); break;
  case ANIME_TYPE_SIN_FADE_SLOW: UpdateAnime_SIN_FADE_SLOW( work ); break;
  case ANIME_TYPE_BLINK:         UpdateAnime_BLINK( work );         break;
  case ANIME_TYPE_BLINK_SHORT:   UpdateAnime_BLINK_SHORT( work );   break;
  case ANIME_TYPE_BLINK_LONG:    UpdateAnime_BLINK_LONG( work );    break;
  default: GF_ASSERT(0);
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを更新する ( sin フェード )
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateAnime_SIN_FADE( PALETTE_ANIME* work )
{
  u8 evy;

  GF_ASSERT( work->animeFlag ); // アニメーション停止中
  GF_ASSERT( work->setupFlag ); // セットアップされていない

  // フェード係数を決定
  {
    int idx;
    float sinVal;
    idx = (work->frameCount * 65536 / SIN_FADE_ONE_CYCLE_FRAMES) % 65536;
    sinVal = FX_FX16_TO_F32( FX_SinIdx( idx ) );
    evy = (int)( sinVal * 16 + 16 );
    evy = SIN_FADE_MIN_EVY + (SIN_FADE_MAX_EVY - SIN_FADE_MIN_EVY) * evy / 32;
  }

  // パレット書き換え
  SoftFade( work->originalColor, work->transDest, work->colorNum, evy, work->fadeColor ); 

  // アニメーションフレーム数をインクリメント
  work->frameCount++;
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを更新する ( sin フェード・早 )
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateAnime_SIN_FADE_FAST( PALETTE_ANIME* work )
{
  u8 evy;

  GF_ASSERT( work->animeFlag ); // アニメーション停止中
  GF_ASSERT( work->setupFlag ); // セットアップされていない

  // フェード係数を決定
  {
    int idx;
    float sinVal;
    idx = (work->frameCount * 65536 / SIN_FADE_FAST_ONE_CYCLE_FRAMES) % 65536;
    sinVal = FX_FX16_TO_F32( FX_SinIdx( idx ) );
    evy = (int)( sinVal * 16 + 16 );
    evy = SIN_FADE_FAST_MIN_EVY + (SIN_FADE_FAST_MAX_EVY - SIN_FADE_FAST_MIN_EVY) * evy / 32;
  }

  // パレット書き換え
  SoftFade( work->originalColor, work->transDest, work->colorNum, evy, work->fadeColor ); 

  // アニメーションフレーム数をインクリメント
  work->frameCount++;
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを更新する ( sin フェード・遅 )
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateAnime_SIN_FADE_SLOW( PALETTE_ANIME* work )
{
  u8 evy;

  GF_ASSERT( work->animeFlag ); // アニメーション停止中
  GF_ASSERT( work->setupFlag ); // セットアップされていない

  // フェード係数を決定
  {
    int idx;
    float sinVal;
    idx = (work->frameCount * 65536 / SIN_FADE_SLOW_ONE_CYCLE_FRAMES) % 65536;
    sinVal = FX_FX16_TO_F32( FX_SinIdx( idx ) );
    evy = (int)( sinVal * 16 + 16 );
    evy = SIN_FADE_SLOW_MIN_EVY + (SIN_FADE_SLOW_MAX_EVY - SIN_FADE_SLOW_MIN_EVY) * evy / 32;
  }

  // パレット書き換え
  SoftFade( work->originalColor, work->transDest, work->colorNum, evy, work->fadeColor ); 

  // アニメーションフレーム数をインクリメント
  work->frameCount++;
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを更新する ( 点滅 )
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateAnime_BLINK( PALETTE_ANIME* work )
{
  u8 evy;

  GF_ASSERT( work->animeFlag ); // アニメーション停止中
  GF_ASSERT( work->setupFlag ); // セットアップされていない
  GF_ASSERT( work->frameCount <= BLINK_FRAMES ); // 不正呼び出し

  // フェード係数を決定
  if( ((work->frameCount / BLINK_WAIT_FRAMES) % 2) == 0 ) {
    evy = BLINK_MIN_EVY;
  }
  else {
    evy = BLINK_MAX_EVY;
  }

  // パレット書き換え
  SoftFade( work->originalColor, work->transDest, work->colorNum, evy, work->fadeColor ); 

  // アニメーションフレーム数をインクリメント
  work->frameCount++;

  // アニメーション終了
  if( BLINK_FRAMES < work->frameCount ) {
    StopAnime( work );
    ResetPalette( work );
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを更新する ( 点滅・短 )
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateAnime_BLINK_SHORT( PALETTE_ANIME* work )
{
  u8 evy;

  GF_ASSERT( work->animeFlag ); // アニメーション停止中
  GF_ASSERT( work->setupFlag ); // セットアップされていない
  GF_ASSERT( work->frameCount <= BLINK_SHORT_FRAMES ); // 不正呼び出し

  // フェード係数を決定
  if( ((work->frameCount / BLINK_SHORT_WAIT_FRAMES) % 2) == 0 ) {
    evy = BLINK_SHORT_MIN_EVY;
  }
  else {
    evy = BLINK_SHORT_MAX_EVY;
  }

  // パレット書き換え
  SoftFade( work->originalColor, work->transDest, work->colorNum, evy, work->fadeColor ); 

  // アニメーションフレーム数をインクリメント
  work->frameCount++;

  // アニメーション終了
  if( BLINK_SHORT_FRAMES < work->frameCount ) {
    StopAnime( work );
    ResetPalette( work );
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief アニメーションを更新する ( 点滅・長 )
 *
 * @param work
 */
//-----------------------------------------------------------------------------
static void UpdateAnime_BLINK_LONG( PALETTE_ANIME* work )
{
  u8 evy;

  GF_ASSERT( work->animeFlag ); // アニメーション停止中
  GF_ASSERT( work->setupFlag ); // セットアップされていない
  GF_ASSERT( work->frameCount <= BLINK_LONG_FRAMES ); // 不正呼び出し

  // フェード係数を決定
  if( ((work->frameCount / BLINK_LONG_WAIT_FRAMES) % 2) == 0 ) {
    evy = BLINK_LONG_MIN_EVY;
  }
  else {
    evy = BLINK_LONG_MAX_EVY;
  }

  // パレット書き換え
  SoftFade( work->originalColor, work->transDest, work->colorNum, evy, work->fadeColor ); 

  // アニメーションフレーム数をインクリメント
  work->frameCount++;

  // アニメーション終了
  if( BLINK_LONG_FRAMES < work->frameCount ) {
    StopAnime( work );
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
  work->heapID = heapID; // 生成時のヒープIDを記憶
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
  GF_ASSERT( work->setupFlag == FALSE ); // クリーンアップがされていない

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

  // ワークを0クリア
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

  GF_ASSERT( work->setupFlag == FALSE ); // すでにセットアップされている

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
  GF_ASSERT( work->setupFlag ); // セットアップされていない

  // 元データのワークを破棄
  GFL_HEAP_FreeMemory( work->originalColor );

  // セットアップフラグを伏せる
  work->setupFlag = FALSE;
}
