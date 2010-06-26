//============================================================================================
/**
 * @file  ringtone_sys.c
 * @date  2010.03.11
 * @author  tamada GAME FREAK inc.
 * @brief   TVトランシーバー着信音＆DS開閉でのボリューム制御
 *
 * 着信音システムはフィールドサウンドシステム、プレイヤーボリューム制御システムに
 * 依存しており、直接呼び出されることはない。
 *
 *
 * 2010.3.18  着信音がなっていない場合はマスターボリュームを制御するように修正
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
 * @brief システムの状態定義
 */
//------------------------------------------------------------------
typedef enum {
  STAT_OPEN_OFF = 0, ///<蓋開いてる：着信OFF
  STAT_OPEN_ON,      ///<蓋開いてる：着信ON
  STAT_CLOSE_OFF,    ///<蓋閉じてる：着信OFF
  STAT_CLOSE_ON,     ///<蓋閉じてる：着信ON

  STAT_MAX,
}RINGTONE_STAT;

//------------------------------------------------------------------
/**
 * @brief リクエストの定義
 */
//------------------------------------------------------------------
typedef enum {
  REQ_STOP = 0,   ///<リクエスト：着信OFF
  REQ_PLAY,       ///<リクエスト：着信ON
  REQ_OPEN,       ///<リクエスト：ふた開けた
  REQ_CLOSE,      ///<リクエスト：ふた閉じた

  REQ_MAX,
}RINGTONE_REQ;

//------------------------------------------------------------------
/**
 * @brief 着信音制御システムワーク定義
 */
//------------------------------------------------------------------
struct _RINGTONE_SYS{

  RINGTONE_STAT state;  ///<制御システムの状態
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

static void changeStatus( RINGTONE_SYS * rtone, RINGTONE_REQ req );
static void updateStatus( RINGTONE_SYS * rtone );


//============================================================================================
//
//    外部公開関数
//
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
  rtone->ringWait = 0;
  rtone->fader = fader;

  ringToneSysPtr = rtone;

  //着信音制御対応のDS開閉時サウンドコールバックを登録
  RINGTONE_SYS_SetSleepSoundFunc();

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
  updateStatus( rtone );
}

//------------------------------------------------------------------
/**
 * @brief 着信音の開始
 * @param RINGTONE_SYS  着信音制御システムのポインタ
 */
//------------------------------------------------------------------
void RINGTONE_SYS_Play( RINGTONE_SYS * rtone )
{
  changeStatus( rtone, REQ_PLAY );
}

//------------------------------------------------------------------
/**
 * @brief 着信音の停止
 * @param RINGTONE_SYS  着信音制御システムのポインタ
 */
//------------------------------------------------------------------
void RINGTONE_SYS_Stop( RINGTONE_SYS * rtone )
{
  changeStatus( rtone, REQ_STOP );
}

//------------------------------------------------------------------
/**
 * @brief 着信音制御対応のDS開閉時サウンドコールバックを登録
 */
//------------------------------------------------------------------
void RINGTONE_SYS_SetSleepSoundFunc( void )
{
  if ( ringToneSysPtr )
  {
    GFL_UI_SleepSoundGoSetFunc( ringTone_sleepSoundGoFunc, ringToneSysPtr );
    GFL_UI_SleepSoundReleaseSetFunc( ringTone_sleepSoundReleaseFunc, ringToneSysPtr );
  } else {
    //着信音システムがないのに呼び出された場合
    GF_ASSERT( 0 );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  RINGTONEのスリープサウンド操作を使用しているか？
 *
 *	@retval TRUE    使用している
 *	@retval FALSE   使用していない
 */
//-----------------------------------------------------------------------------
BOOL RINGTONE_SYS_IsSleepSoundFunc( void )
{
  if( GFL_UI_SleepSoundGoGetFunc() == ringTone_sleepSoundGoFunc ){
    GF_ASSERT( GFL_UI_SleepSoundReleaseGetFunc() == ringTone_sleepSoundReleaseFunc );
    return TRUE;
  }
  return FALSE;
}

//============================================================================================
//
//    内部使用関数
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief   着信音再生制御
 * @param   rtone
 * @return  BOOL  TRUEのとき、なり続けている
 */
//------------------------------------------------------------------
static BOOL ringTone_PlayMain( RINGTONE_SYS * rtone )
{
  if ( rtone->ringWait )
  {
    rtone->ringWait --;
  }
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
  changeStatus( rtone, REQ_CLOSE );
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
  changeStatus( rtone, REQ_OPEN );
}

//------------------------------------------------------------------
/**
 * @brief 着信音開始
 */
//------------------------------------------------------------------
static void startSE( RINGTONE_SYS * rtone )
{
  if (PMSND_CheckPlaySE_byPlayerID( RINGTONE_SEPLAYER ) == FALSE )
  {
    PMSND_PlaySE_byPlayerID( RINGTONE_SE_NO, RINGTONE_SEPLAYER );
  }
  rtone->ringWait = RINGTONE_LENGTH;
}
//------------------------------------------------------------------
/**
 * @brief 着信音中止
 */
//------------------------------------------------------------------
static void stopSE( RINGTONE_SYS * rtone )
{
  PMSND_StopSE_byPlayerID( RINGTONE_SEPLAYER );
  rtone->ringWait = 0;
}
//------------------------------------------------------------------
/**
 * @brief 他のプレイヤーのミュート操作
 */
//------------------------------------------------------------------
static void setMuteOther( RINGTONE_SYS * rtone, BOOL flag )
{
  if ( flag == TRUE ) {
    NNS_SndPlayerSetPlayerVolume( PLAYER_SE_SYS, MIN_VOLUME );
    NNS_SndPlayerSetPlayerVolume( PLAYER_SE_1, MIN_VOLUME );
    NNS_SndPlayerSetPlayerVolume( PLAYER_SE_2, MIN_VOLUME );
    NNS_SndPlayerSetPlayerVolume( PLAYER_SE_PSG, MIN_VOLUME );
  } else {
    NNS_SndPlayerSetPlayerVolume( PLAYER_SE_SYS, MAX_VOLUME );
    NNS_SndPlayerSetPlayerVolume( PLAYER_SE_1, MAX_VOLUME );
    NNS_SndPlayerSetPlayerVolume( PLAYER_SE_2, MAX_VOLUME );
    NNS_SndPlayerSetPlayerVolume( PLAYER_SE_PSG, MAX_VOLUME );
  }
  PLAYER_VOLUME_FADER_SetMute( rtone->fader, flag );
}

//------------------------------------------------------------------
/**
 * @brief システムの状態変更処理
 * @param   rtone
 * @param   req     状態変更リクエスト
 */
//------------------------------------------------------------------
static void changeStatus( RINGTONE_SYS * rtone, RINGTONE_REQ req )
{
  static const u8 seqTbl[STAT_MAX][REQ_MAX] = {
    // STOP             PLAY            OPEN          CLOSE
    { STAT_OPEN_OFF,  STAT_OPEN_ON,   STAT_OPEN_OFF,  STAT_CLOSE_OFF }, //OPEN  & OFF
    { STAT_OPEN_OFF,  STAT_OPEN_ON,   STAT_OPEN_ON,   STAT_CLOSE_ON },  //OPEN  & ON
    { STAT_CLOSE_OFF, STAT_CLOSE_ON,  STAT_OPEN_OFF,  STAT_CLOSE_OFF }, //CLOSE & OFF
    { STAT_CLOSE_OFF, STAT_CLOSE_ON,  STAT_OPEN_ON,   STAT_CLOSE_ON },  //CLOSE & ON
  };

  RINGTONE_STAT new_state = seqTbl[rtone->state][req];

  //OS_TPrintf(" RINGTONE REQ(%d) STAT(%d --> %d)\n", req, rtone->state, new_state );

  switch ( new_state )
  {
  case STAT_OPEN_OFF:
    NNS_SndSetMasterVolume( MAX_VOLUME ); //MasterVol  == 有効
    setMuteOther( rtone, FALSE );         //他のPlayer == 有効
    stopSE( rtone );
    break;

  case STAT_OPEN_ON:
    NNS_SndSetMasterVolume( MAX_VOLUME ); //MasterVol  == 有効
    setMuteOther( rtone, TRUE );          //他のPlayer == 無効
    startSE( rtone );
    break;

  case STAT_CLOSE_OFF:
    NNS_SndSetMasterVolume( MIN_VOLUME ); //MasterVol  == 無効
    stopSE( rtone );
    break;

  case STAT_CLOSE_ON:
    NNS_SndSetMasterVolume( MAX_VOLUME ); //MasterVol  == 有効
    setMuteOther( rtone, TRUE );          //他のPlayer == 無効
    startSE( rtone );
    break;
  }

  rtone->state = new_state;
}

//------------------------------------------------------------------
/**
 * @brief 通常更新処理
 * @param rtone
 */
//------------------------------------------------------------------
static void updateStatus( RINGTONE_SYS * rtone )
{
  BOOL flag;
  flag = ringTone_PlayMain( rtone );
  switch ( rtone->state )
  {
  case STAT_OPEN_OFF:
    break;
  case STAT_OPEN_ON:
    if ( flag == FALSE )
    {
      changeStatus( rtone, STAT_OPEN_OFF );
    }
    break;

  case STAT_CLOSE_OFF:
    break;
  case STAT_CLOSE_ON:
    if ( flag == FALSE )
    {
      changeStatus( rtone, STAT_CLOSE_OFF );
    }
    break;
  }
}






