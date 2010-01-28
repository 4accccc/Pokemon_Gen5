/////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  プレイヤーボリュームのフェード管理
 * @file   player_volume_fader.c
 * @author obata
 * @date   2010.01.27
 */
///////////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "player_volume_fader.h"


//===================================================================================
// ■定数
//===================================================================================
#define PRINT_TARGET (1)    // デバッグ表示先
#define MAX_VOLUME   (127)  // 最大ボリューム


//===================================================================================
// ■システムワーク
//===================================================================================
struct _PLAYER_VOLUME_FADER
{
  u8  playerNo;     // プレイヤー番号
  u8  startVolume;  // フェード初期ボリューム
  u8  endVolume;    // フェード最終ボリューム
  u16 fadeFrame;    // フェード フレーム数
  u16 fadeCount;    // フェード フレームカウンタ
};


//===================================================================================
// ■非公開関数
//===================================================================================
// システム制御
static void InitFader      ( PLAYER_VOLUME_FADER* fader );
static void SetFadeParam   ( PLAYER_VOLUME_FADER* fader, u8 endVolume, u16 fadeFrame );
static void SetPlayerVolume( PLAYER_VOLUME_FADER* fader, u8 volume );
static void FaderMain      ( PLAYER_VOLUME_FADER* fader ); 
// ボリューム制御
static int  CalcNowVolume     ( const PLAYER_VOLUME_FADER* fader );
static void UpdatePlayerVolume( const PLAYER_VOLUME_FADER* fader );


//===================================================================================
// ■外部公開関数
//===================================================================================

//-----------------------------------------------------------------------------------
/**
 * @brief システムを生成する
 *
 * @param heapID   使用するヒープID
 * @param playerNo システムが操作するプレイヤー番号
 */
//-----------------------------------------------------------------------------------
PLAYER_VOLUME_FADER* PLAYER_VOLUME_FADER_Create( HEAPID heapID, u8 playerNo )
{
  PLAYER_VOLUME_FADER* fader;

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PLAYER-VOLUME-FADER: create system\n" );

  // 生成
  fader = GFL_HEAP_AllocMemory( heapID, sizeof(PLAYER_VOLUME_FADER) );

  // 初期化
  InitFader( fader );
  fader->playerNo = playerNo;

  return fader;
}

//-----------------------------------------------------------------------------------
/**
 * @brief システムを破棄する
 *
 * @param fader
 */
//-----------------------------------------------------------------------------------
void PLAYER_VOLUME_FADER_Delete( PLAYER_VOLUME_FADER* fader )
{
  GFL_HEAP_FreeMemory( fader );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PLAYER-VOLUME-FADER: delete system\n" );
}

//-----------------------------------------------------------------------------------
/**
 * @brief システム動作
 *
 * @parma fader
 */
//-----------------------------------------------------------------------------------
void PLAYER_VOLUME_FADER_Main( PLAYER_VOLUME_FADER* fader )
{
  FaderMain( fader );
}

//-----------------------------------------------------------------------------------
/**
 * @brief ボリュームを変更する
 *
 * @param fader
 * @param volume 設定するボリューム [0, 127]
 * @param frames フェードフレーム数
 */
//-----------------------------------------------------------------------------------
void PLAYER_VOLUME_FADER_SetVolume( PLAYER_VOLUME_FADER* fader, u8 volume, u16 frames )
{
  // 即時設定
  if( frames == 0)
  { 
    SetPlayerVolume( fader, volume ); 
    return;
  }

  // フェード設定
  SetFadeParam( fader, volume, frames );
}


//==================================================================================
// ■システム制御
//==================================================================================

//----------------------------------------------------------------------------------
/**
 * @brief システムを初期化する
 *
 * @param fader
 */
//----------------------------------------------------------------------------------
static void InitFader( PLAYER_VOLUME_FADER* fader )
{
  fader->playerNo    = 0;
  fader->startVolume = MAX_VOLUME;
  fader->endVolume   = MAX_VOLUME;
  fader->fadeFrame   = 0;
  fader->fadeCount   = 0;

  UpdatePlayerVolume( fader ); 
}

//----------------------------------------------------------------------------------
/**
 * @brief フェード パラメータを設定する
 *
 * @param fader
 * @param endVolume フェード最終値
 * @param fadeFrame フェード フレーム数
 */
//----------------------------------------------------------------------------------
static void SetFadeParam( PLAYER_VOLUME_FADER* fader, u8 endVolume, u16 fadeFrame )
{
  fader->startVolume = CalcNowVolume( fader );
  fader->endVolume   = endVolume;
  fader->fadeFrame   = fadeFrame;
  fader->fadeCount   = 0; 
}

//----------------------------------------------------------------------------------
/**
 * @brief プレイヤーボリュームを即時設定する
 *
 * @param fader
 * @parma volume 設定するボリューム
 */
//----------------------------------------------------------------------------------
static void SetPlayerVolume( PLAYER_VOLUME_FADER* fader, u8 volume )
{
  fader->startVolume = volume;
  fader->endVolume   = volume;
  fader->fadeFrame   = 0;
  fader->fadeCount   = 0;

  UpdatePlayerVolume( fader );
}

//----------------------------------------------------------------------------------
/**
 * @brief システム メイン動作
 *
 * @param fader
 */
//----------------------------------------------------------------------------------
static void FaderMain( PLAYER_VOLUME_FADER* fader )
{
  if( fader->fadeCount < fader->fadeFrame )
  {
    fader->fadeCount++;
    UpdatePlayerVolume( fader );
  } 
}

//==================================================================================
// ■ボリューム制御
//==================================================================================

//----------------------------------------------------------------------------------
/**
 * @brief 現在のボリュームを計算する
 *
 * @param fader
 */
//---------------------------------------------------------------------------------- 
static int CalcNowVolume( const PLAYER_VOLUME_FADER* fader )
{
  int volume;
  int startVolume, endVolume;
  int fadeFrame, fadeCount;

  startVolume = fader->startVolume;
  endVolume   = fader->endVolume;
  fadeFrame   = fader->fadeFrame;
  fadeCount   = fader->fadeCount;

  if( fadeCount == 0 )
  {
    volume = startVolume;
  }
  else
  { 
    volume = (endVolume - startVolume) * fadeCount / fadeFrame + startVolume;
  }

  GF_ASSERT( 0 <= volume );
  GF_ASSERT( volume <= MAX_VOLUME ); 
  return volume;
}

//----------------------------------------------------------------------------------
/**
 * @brief プレイヤーのボリュームを更新する
 * 
 * @param fader
 */
//----------------------------------------------------------------------------------
static void UpdatePlayerVolume( const PLAYER_VOLUME_FADER* fader )
{
  int volume;

  volume = CalcNowVolume( fader );
  NNS_SndPlayerSetPlayerVolume( fader->playerNo, volume );

  // DEBUG:
  OS_TFPrintf( PRINT_TARGET, "PLAYER-VOLUME-FADER: update player volume ==> %d\n", volume );
}
