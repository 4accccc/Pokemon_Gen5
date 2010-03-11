//============================================================================================
/**
 * @file  ringtone_sys.c
 * @date  2010.03.11
 * @author  tamada GAME FREAK inc.
 * @brief   TVトランシーバー着信音制御
 */
//============================================================================================

#include <gflib.h>
#include "ringtone_sys.h"

#include "sound/pm_sndsys.h"
#include "player_volume_fader.h"

#define USE_LOOP_SE

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief 着信音制御システムワーク定義
 */
//------------------------------------------------------------------
struct _RINGTONE_SYS{
  BOOL ring_flag;     ///<着信音が鳴っているかどうか
  BOOL isFoldMode;    ///<DSを閉じているかどうか

  u16  ringtone_status;
  u16  call_count;

  PLAYER_VOLUME_FADER * fader;  ///<NNSのプレイヤーボリューム制御システムへのポインタ

};
//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  //RINGTONE_SE_NO = SEQ_SE_OPEN1,
  RINGTONE_SE_NO = SEQ_SE_SYS_35, 
  //RINGTONE_SE_NO = SEQ_SE_SYS_85, 
  RINGTONE_SEPLAYER = SEPLAYER_SE3,

  RINGTONE_LENGTH = 60 * 15,
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
//------------------------------------------------------------------
RINGTONE_SYS * RINGTONE_SYS_Create( HEAPID heapID, PLAYER_VOLUME_FADER * fader )
{
  RINGTONE_SYS * rtone = GFL_HEAP_AllocClearMemory( heapID, sizeof(RINGTONE_SYS) );
  rtone->ring_flag = FALSE;
  rtone->isFoldMode = FALSE;
  rtone->ringtone_status = 0;
  rtone->call_count = 0;
  rtone->fader = fader;

  //着信音制御対応のDS開閉時サウンドコールバックを登録
  GFL_UI_SleepSoundGoSetFunc( ringTone_sleepSoundGoFunc, rtone );
  GFL_UI_SleepSoundReleaseSetFunc( ringTone_sleepSoundReleaseFunc, rtone );

  ringToneSysPtr = rtone;
  return rtone;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void RINGTONE_SYS_Delete( RINGTONE_SYS * rtone )
{
  GFL_HEAP_FreeMemory( rtone );
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void RINGTONE_SYS_Main( RINGTONE_SYS * rtone )
{
  if ( rtone->ring_flag == TRUE )
  {
    BOOL flag = ringTone_PlayMain( rtone );
    if ( flag == FALSE )
    {
      rtone->ring_flag = FALSE;
      //蓋を閉じていない場合はPlayerVolumeの操作も行う
      if (rtone->isFoldMode == FALSE)
      {
        PLAYER_VOLUME_FADER_SetMasterVolume( rtone->fader, 127 );
      }
    }
  }
}

//------------------------------------------------------------------
//------------------------------------------------------------------
void RINGTONE_SYS_RequestSE( RINGTONE_SYS * rtone )
{
  if ( rtone->ring_flag == FALSE )
  {
    PMSND_PlaySE_byPlayerID( RINGTONE_SE_NO, RINGTONE_SEPLAYER );
    rtone->ring_flag = TRUE;
#ifdef  USE_LOOP_SE
    rtone->ringtone_status = 0;
    rtone->call_count = RINGTONE_LENGTH;
#endif
    //蓋を閉じていない場合はPlayerVolumeの操作も行う
    if (rtone->isFoldMode == FALSE)
    {
      PLAYER_VOLUME_FADER_SetMasterVolume( rtone->fader, 0 );
    }
  }
}


//------------------------------------------------------------------
//------------------------------------------------------------------
static BOOL ringTone_PlayMain( RINGTONE_SYS * rtone )
{
#ifdef  USE_LOOP_SE
  rtone->call_count --;
  if ( rtone->call_count == 0 )
  {
    if (PMSND_CheckPlaySE_byPlayerID( RINGTONE_SEPLAYER ) == TRUE )
    {
      PMSND_StopSE_byPlayerID( RINGTONE_SEPLAYER );
    } 
    return FALSE;
  } else {
    if (PMSND_CheckPlaySE_byPlayerID( RINGTONE_SEPLAYER ) == FALSE )
    {
      PMSND_PlaySE_byPlayerID( RINGTONE_SE_NO, RINGTONE_SEPLAYER );
      rtone->ringtone_status ++;
    }
    return TRUE;
  }
#else
  return PMSND_CheckPlaySE_byPlayerID( RINGTONE_SEPLAYER );
#endif
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void ringTone_sleepSoundGoFunc( void * work )
{
  RINGTONE_SYS * rtone = ringToneSysPtr;
  //RINGTONE_SYS * rtone = (RINGTONE_SYS *)work;
  PLAYER_VOLUME_FADER_SetMasterVolume( rtone->fader, 0 );
  rtone->isFoldMode = TRUE;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void ringTone_sleepSoundReleaseFunc( void * work )
{
  RINGTONE_SYS * rtone = ringToneSysPtr;
  //RINGTONE_SYS * rtone = (RINGTONE_SYS *)work;
  if ( rtone->ring_flag == FALSE )
  { //鳴っていない場合、Volumeをもとに戻す
    PLAYER_VOLUME_FADER_SetMasterVolume( rtone->fader, 127 );
  }
  rtone->isFoldMode = FALSE;
}

