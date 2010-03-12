//============================================================================================
/**
 * @file  ringtone_sys.c
 * @date  2010.03.11
 * @author  tamada GAME FREAK inc.
 * @brief   TVトランシーバー着信音制御
 *
 * 着信音システムはフィールドサウンドシステム、プレイヤーボリューム制御システムに
 * 依存しており、直接呼び出されることはない。
 */
//============================================================================================

#include <gflib.h>
#include "ringtone_sys.h"

#include "sound/pm_sndsys.h"
#include "player_volume_fader.h"


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 着信音制御システムワーク定義
 */
//------------------------------------------------------------------
struct _RINGTONE_SYS{
  BOOL ringFlag;       ///<着信音が鳴っているかどうか
  BOOL isFoldMode;     ///<DSを閉じているかどうか

  u16  ringWait;       ///<着信音持続時間用カウンタ

  PLAYER_VOLUME_FADER * fader;  ///<NNSのプレイヤーボリューム制御システムへのポインタ

};
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {

  ///着信音SEのナンバー
  RINGTONE_SE_NO = SEQ_SE_SYS_35, 

  ///着信音SEに使用するプレイヤーの指定ID
  RINGTONE_SEPLAYER = SEPLAYER_SE3,

  ///着信音が鳴り続ける長さ
  RINGTONE_LENGTH = 60 * 15,

  MIN_VOLUME = 0,
  MAX_VOLUME = 127,
};

//============================================================================================
//============================================================================================
static RINGTONE_SYS * ringToneSysPtr = NULL;

//============================================================================================
//============================================================================================

static void ringTone_sleepSoundGoFunc( void * work );
static void ringTone_sleepSoundReleaseFunc( void * work );
static BOOL ringTone_PlayMain( RINGTONE_SYS * rtone );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 着信音制御システムの生成
 * @param HEAPID  使用するヒープの指定ID
 * @param fader   PLAYER_VOLUME_FADERへのポインタ
 * @return  RINGTONE_SYS  生成した着信音制御システムのポインタ
 */
//------------------------------------------------------------------
RINGTONE_SYS * RINGTONE_SYS_Create( HEAPID heapID, PLAYER_VOLUME_FADER * fader )
{
  RINGTONE_SYS * rtone = GFL_HEAP_AllocClearMemory( heapID, sizeof(RINGTONE_SYS) );
  rtone->ringFlag = FALSE;
  rtone->isFoldMode = FALSE;
  rtone->ringWait = 0;
  rtone->fader = fader;

  //着信音制御対応のDS開閉時サウンドコールバックを登録
  GFL_UI_SleepSoundGoSetFunc( ringTone_sleepSoundGoFunc, rtone );
  GFL_UI_SleepSoundReleaseSetFunc( ringTone_sleepSoundReleaseFunc, rtone );

  ringToneSysPtr = rtone;
  return rtone;
}

//------------------------------------------------------------------
/**
 * @brief 着信音制御システムの削除
 * @param RINGTONE_SYS  着信音制御システムのポインタ
 */
//------------------------------------------------------------------
void RINGTONE_SYS_Delete( RINGTONE_SYS * rtone )
{
  GFL_HEAP_FreeMemory( rtone );
}

//------------------------------------------------------------------
/**
 * @brief 着信音制御システムのメイン処理
 * @param RINGTONE_SYS  着信音制御システムのポインタ
 */
//------------------------------------------------------------------
void RINGTONE_SYS_Main( RINGTONE_SYS * rtone )
{
  if ( rtone->ringFlag == TRUE )
  {
    BOOL flag = ringTone_PlayMain( rtone );
    if ( flag == FALSE )
    {
      rtone->ringFlag = FALSE;
      //蓋を閉じていない場合はPlayerVolumeの操作も行う
      if (rtone->isFoldMode == FALSE)
      {
        PLAYER_VOLUME_FADER_SetMasterVolume( rtone->fader, MAX_VOLUME );
      }
    }
  }
}

//------------------------------------------------------------------
/**
 * @brief 着信音の開始
 * @param RINGTONE_SYS  着信音制御システムのポインタ
 */
//------------------------------------------------------------------
void RINGTONE_SYS_Play( RINGTONE_SYS * rtone )
{
  if ( rtone->ringFlag == FALSE )
  {
    PMSND_PlaySE_byPlayerID( RINGTONE_SE_NO, RINGTONE_SEPLAYER );
    rtone->ringFlag = TRUE;
    rtone->ringWait = RINGTONE_LENGTH;
    //蓋を閉じていない場合はPlayerVolumeの操作も行う
    if (rtone->isFoldMode == FALSE)
    {
      PLAYER_VOLUME_FADER_SetMasterVolume( rtone->fader, MIN_VOLUME );
    }
  }
}

//------------------------------------------------------------------
/**
 * @brief 着信音の停止
 * @param RINGTONE_SYS  着信音制御システムのポインタ
 */
//------------------------------------------------------------------
void RINGTONE_SYS_Stop( RINGTONE_SYS * rtone )
{
  if ( rtone->ringFlag == TRUE )
  {
    PMSND_StopSE_byPlayerID( RINGTONE_SEPLAYER );
    rtone->ringFlag = FALSE;
    rtone->ringWait = 0;
    //蓋を閉じていない場合はPlayerVolumeの操作も行う
    if (rtone->isFoldMode == FALSE)
    {
      PLAYER_VOLUME_FADER_SetMasterVolume( rtone->fader, MAX_VOLUME );
    }
  }
}

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL ringTone_PlayMain( RINGTONE_SYS * rtone )
{
  rtone->ringWait --;
  if ( rtone->ringWait == 0 )
  {
    if (PMSND_CheckPlaySE_byPlayerID( RINGTONE_SEPLAYER ) == TRUE )
    {
      PMSND_StopSE_byPlayerID( RINGTONE_SEPLAYER );
    } 
    return FALSE;
  }
  else
  {
    if (PMSND_CheckPlaySE_byPlayerID( RINGTONE_SEPLAYER ) == FALSE )
    {
      PMSND_PlaySE_byPlayerID( RINGTONE_SE_NO, RINGTONE_SEPLAYER );
    }
    return TRUE;
  }
}

//------------------------------------------------------------------
/**
 * @brief  フォールド状態への移行処理
 */
//------------------------------------------------------------------
static void ringTone_sleepSoundGoFunc( void * work )
{
  RINGTONE_SYS * rtone = ringToneSysPtr;
  //RINGTONE_SYS * rtone = (RINGTONE_SYS *)work;
  //着信音以外のプレイヤーのボリュームを０にする
  PLAYER_VOLUME_FADER_SetMasterVolume( rtone->fader, MIN_VOLUME );
  rtone->isFoldMode = TRUE;
}

//------------------------------------------------------------------
/**
 * @brief フォールド状態からの復帰処理
 */
//------------------------------------------------------------------
static void ringTone_sleepSoundReleaseFunc( void * work )
{
  RINGTONE_SYS * rtone = ringToneSysPtr;
  //RINGTONE_SYS * rtone = (RINGTONE_SYS *)work;
  if ( rtone->ringFlag == FALSE )
  { //鳴っていない場合、Volumeをもとに戻す
    PLAYER_VOLUME_FADER_SetMasterVolume( rtone->fader, MAX_VOLUME );
  }
  rtone->isFoldMode = FALSE;
}

