//======================================================================
/**
 * @file	field_sound.c
 * @brief	フィールドのサウンド関連まとめ
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "sound/pm_sndsys.h"

#include "fieldmap.h"
#include "field/zonedata.h"

#include "field_sound.h"

//======================================================================
//  define
//======================================================================
#define TRACKBIT_ALL (0xffff) ///<全track ON
//#define TRACKBIT_ACTION ((1<<8)|(1<<9)) ///<アクション用BGM Track 9,10
//#define TRACKBIT_STILL (TRACKBIT_ALL^TRACKBIT_ACTION) ///<Action Track OFF


// FIELD_SOUNDフェード監視状態
typedef enum{
  FS_STATE_PLAY,      // 再生中
  FS_STATE_STOP,      // 停止中
  FS_STATE_FADE_IN,   // フェードイン中
  FS_STATE_FADE_OUT,  // フェードアウト中
} FS_STATE;


//======================================================================
//  struct  
//======================================================================
//--------------------------------------------------------------
/// FIELD_SOUND
//--------------------------------------------------------------
struct _TAG_FIELD_SOUND
{
  u16 play_event_bgm; // イベントBGM再生中
  s16 push_count; // BGM Pushカウント

  FS_STATE   state;  // 現在の状態
  u32      nextBGM;  // 次に再生するBGM
  u16  fadeInFrame;  // フェードインフレーム数 
  u16 fadeOutFrame;  // フェードアウトフレーム数
  BOOL   reqFadeIn;  // リクエスト(フェードイン) 
  BOOL  reqFadeOut;  // リクエスト(フェードアウト) 
  BOOL      reqPop;  // リクエスト(ポップ)
  BOOL     reqPush;  // リクエスト(プッシュ)
};

//======================================================================
//  proto
//======================================================================
static u32 fsnd_GetMapBGM( GAMEDATA *gdata, u32 zone_id );
static void fsnd_PushBGM( FIELD_SOUND *fsnd, int max );
static void fsnd_PopBGM( FIELD_SOUND *fsnd );
static void fsnd_UpdateBGM_PLAY( FIELD_SOUND* fsnd );
static void fsnd_UpdateBGM_STOP( FIELD_SOUND* fsnd );
static void fsnd_UpdateBGM_FADE_IN( FIELD_SOUND* fsnd );
static void fsnd_UpdateBGM_FADE_OUT( FIELD_SOUND* fsnd );

//======================================================================
//  FILED_SOUND
//======================================================================
//--------------------------------------------------------------
/**
 * FIELD_SOUND FIELD_SOUND作成
 * @param heapID HEAPID
 * @retval FIELD_SOUND*
 */
//--------------------------------------------------------------
FIELD_SOUND * FIELD_SOUND_Create( HEAPID heapID )
{
  FIELD_SOUND *fsnd;
  fsnd = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_SOUND) );
  fsnd->state      = FS_STATE_STOP;
  fsnd->nextBGM    = 0;
  fsnd->reqFadeIn  = FALSE;
  fsnd->reqFadeOut = FALSE;
  fsnd->reqPop     = FALSE;
  fsnd->reqPush    = FALSE;
  return( fsnd );
}

//--------------------------------------------------------------
/**
 * FIELD_SOUND FIELDSOUND削除
 * @param heapID HEAPID
 * @retval FIELD_SOUND*
 */
//--------------------------------------------------------------
void FIELD_SOUND_Delete( FIELD_SOUND *fsnd )
{
  GFL_HEAP_FreeMemory( fsnd );
}

//======================================================================
//  フィールド BGM 再生
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドBGM  BGM再生
 * @param bgmNo 再生するBGMナンバー
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PlayBGM( u32 bgmNo )
{ 
  u32 now;
  
  OS_Printf( "FIELD PLAY BGMNO %d\n", bgmNo );
  
  if( bgmNo == 0 ){
    OS_Printf( "WARNING: FIELD PLAY BGMNO ZERO\n" );
    return;
  }
  
  now = PMSND_GetNextBGMsoundNo();
  
  if( now != bgmNo ){
    PMSND_PlayBGM_EX( bgmNo, TRACKBIT_ALL );
  }
}

//--------------------------------------------------------------
/**
 * フィールドBGM フェードアウト → BGM再生
 * @param fsnd         フィールドサウンド
 * @param bgmNo        再生するBGMナンバー
 * @param fadeOutFrame フェードアウトフレーム数
 * @param fadeInFrame  フェードインフレーム数
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PlayNextBGM_Ex( FIELD_SOUND* fsnd, u32 bgmNo, 
                                 u16 fadeOutFrame, u16 fadeInFrame )
{ 
  // 何らかのリクエストがある場合, BGM変更リクエストは無効とする
  if( fsnd->reqFadeIn || fsnd->reqFadeOut || fsnd->reqPop || fsnd->reqPush )
  {
    return;
  } 
  // リクエストを更新
  fsnd->nextBGM      = bgmNo;
  fsnd->fadeInFrame  = fadeInFrame; 
  fsnd->fadeOutFrame = fadeOutFrame; 
}

//--------------------------------------------------------------
/**
 * ゾーン切り替え時のBGM変更
 * @param fsnd FIELD_SOUND
 * @param gdata GAMEDATA
 * @param form PLAYER_MOVE_FORM
 * @param zone_id 
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_ChangePlayZoneBGM( FIELD_SOUND* fsnd, GAMEDATA* gdata, 
                                    PLAYER_MOVE_FORM form, u32 zone_id )
{
  // 自機フォームなどを考慮し, 指定ゾーンで再生すべきBGMを決定
  u32 no = FIELD_SOUND_GetFieldBGMNo( gdata, form, zone_id );
  FIELD_SOUND_PlayNextBGM( fsnd, no );
}

//--------------------------------------------------------------
/**
 * 現在のBGMを退避し、イベント用のBGMを再生
 * @param fsnd FIELD_SOUND
 * @param bgmno BGM番号
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PushPlayEventBGM( FIELD_SOUND *fsnd, u32 bgmno )
{
  if( fsnd->push_count <= FSND_PUSHCOUNT_BASEBGM ){ //EVENT BGMに移行済みか
    fsnd_PushBGM( fsnd, FSND_PUSHCOUNT_BASEBGM );
  }
  FIELD_SOUND_PlayBGM( bgmno );
}

//--------------------------------------------------------------
/**
 * 現在のBGMを退避し、ME用のBGMを再生
 * @param fsnd FIELD_SOUND
 * @param bgmno BGM番号
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PushPlayJingleBGM( FIELD_SOUND *fsnd, u32 bgmno )
{
  fsnd_PushBGM( fsnd, FSND_PUSHCOUNT_EVENTBGM );
  FIELD_SOUND_PlayBGM( bgmno );
}

//======================================================================
//  フィールドBGM 退避、復帰
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドBGM 退避
 * @param FIELD_SOUND *fsnd
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PushBGM( FIELD_SOUND *fsnd )
{
  fsnd_PushBGM( fsnd, FSND_PUSHCOUNT_BASEBGM );
}

//--------------------------------------------------------------
/**
 * フィールドBGM 復帰
 * @param fsnd FIELD_SOUND
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PopBGM( FIELD_SOUND *fsnd )
{
  fsnd_PopBGM( fsnd );
}

//--------------------------------------------------------------
/**
 * フィールドBGM 退避されていれば強制開放。
 * @param fsnd FIELD_SOUND
 * @retval nothing
 * @note エラー回避用なので基本は使用禁止。
 */
//--------------------------------------------------------------
void FIELD_SOUND_ForcePopBGM( FIELD_SOUND *fsnd )
{
  if( fsnd->push_count ){
    GF_ASSERT( 0 && "ERROR: FSOUND FORGET POP\n" );
    
    while( fsnd->push_count ){
      fsnd_PopBGM( fsnd );
      fsnd->push_count--;
    }
  }
}

//event_debug_menu.c
//event_ircbattle.c
//event_colosseum_battle.c
//scrcmd_trainer.c
//scrcmd_musical.c


//======================================================================
// フィールドBGM フェード管理
//======================================================================

//----------------------------------------------------------------------
/**
 * @brief BGMのフェード状態を管理し, BGMの変更を行う
 *
 * @param fsnd FIELD_SOUND
 */
//----------------------------------------------------------------------
void FIELD_SOUND_UpdateBGM( FIELD_SOUND* fsnd )
{
  switch( fsnd->state )
  {
  case FS_STATE_PLAY:     fsnd_UpdateBGM_PLAY( fsnd );     break;
  case FS_STATE_FADE_OUT: fsnd_UpdateBGM_FADE_OUT( fsnd ); break;
  case FS_STATE_STOP:     fsnd_UpdateBGM_STOP( fsnd );     break;
  case FS_STATE_FADE_IN:  fsnd_UpdateBGM_FADE_IN( fsnd );  break;
  }
}

//----------------------------------------------------------------------
/**
 * @brief BGMフェードの管理(再生中)
 */
//----------------------------------------------------------------------
static void fsnd_UpdateBGM_PLAY( FIELD_SOUND* fsnd )
{
  u32 now_bgm = PMSND_GetNextBGMsoundNo();

  // BGM変更 or フェードアウト
  if( (now_bgm != fsnd->nextBGM) || ( fsnd->reqFadeOut ) )
  {
    PMSND_FadeOutBGM( fsnd->fadeOutFrame );
    fsnd->state = FS_STATE_FADE_OUT; 
    // DEBUG:
    OBATA_Printf( "FIELD_SOUND: fade out\n" );
  } 
}
//----------------------------------------------------------------------
/**
 * @brief BGMフェードの管理(フェードアウト中)
 */
//----------------------------------------------------------------------
static void fsnd_UpdateBGM_FADE_OUT( FIELD_SOUND* fsnd )
{
  if( PMSND_CheckFadeOnBGM() != TRUE )  // if(フェード終了)
  { 
    if( fsnd->reqPush )  // if(プッシュリクエスト)
    { 
      fsnd_PushBGM( fsnd, FSND_PUSHCOUNT_BASEBGM );
    } 
    fsnd->reqFadeOut = FALSE;
    fsnd->state      = FS_STATE_STOP;
    // DEBUG:
    OBATA_Printf( "FIELD_SOUND: stop\n" );
  }
}
//----------------------------------------------------------------------
/**
 * @brief BGMフェードの管理(停止中)
 */
//----------------------------------------------------------------------
static void fsnd_UpdateBGM_STOP( FIELD_SOUND* fsnd )
{
  BOOL fade_start = FALSE;
  u32  now_bgm    = PMSND_GetNextBGMsoundNo();

  // Popリクエスト有 ==> フェードイン開始
  if( fsnd->reqPop )
  { // BGM復帰
    fsnd_PopBGM( fsnd );
    PMSND_FadeInBGM( fsnd->fadeInFrame );
    fade_start = TRUE;
  }
  // BGM変更リクエスト有
  else if( fsnd->nextBGM && fsnd->nextBGM != now_bgm )
  { 
    PMSND_PlayNextBGM( fsnd->nextBGM, 0, fsnd->fadeInFrame );
    fade_start = TRUE;
  }
  // フェードインリクエスト有 ==> フェードイン開始
  else if( fsnd->reqFadeIn )
  { 
    PMSND_FadeInBGM( fsnd->fadeInFrame );
    fade_start = TRUE;
  }

  // フェードイン開始
  if( fade_start )
  {
    fsnd->state = FS_STATE_FADE_IN;
    // DEBUG:
    OBATA_Printf( "FIELD_SOUND: fade in\n" );
  }
}
//----------------------------------------------------------------------
/**
 * @brief BGMフェードの管理(フェードイン中)
 */
//----------------------------------------------------------------------
static void fsnd_UpdateBGM_FADE_IN( FIELD_SOUND* fsnd )
{
  // フェード終了 ==> 再生中
  if( PMSND_CheckFadeOnBGM() != TRUE )
  {
    fsnd->reqFadeIn = FALSE;
    fsnd->state     = FS_STATE_PLAY;
    // DEBUG:
    OBATA_Printf( "FIELD_SOUND: play\n" );
  }
}


//======================================================================
//  フィールド BGM フェード
//======================================================================
//--------------------------------------------------------------
/**
 * @brief BGMフェードイン
 * @param fsnd   FIELD_SOUND
 * @param bgmNo  再生するBGM
 * @param frames フェードインに要するフレーム数
 */
//--------------------------------------------------------------
void FIELD_SOUND_FadeInBGM( FIELD_SOUND* fsnd, u32 bgmNo, u16 frames )
{
  fsnd->state       = FS_STATE_STOP;
  fsnd->nextBGM     = bgmNo;
  fsnd->fadeInFrame = frames;
  fsnd->reqFadeIn   = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief BGM Pop ==> フェードイン
 * @param fsnd   FIELD_SOUND
 * @param frames フェードインに要するフレーム数
 */
//--------------------------------------------------------------
void FIELD_SOUND_FadeInPopBGM( FIELD_SOUND* fsnd, u16 frames )
{
  FIELD_SOUND_FadeInBGM( fsnd, 0, frames );
  fsnd->reqPop = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief BGMフェードアウト
 * @param fsnd   FIELD_SOUND
 * @param frames フェードアウトに要するフレーム数
 */
//--------------------------------------------------------------
void FIELD_SOUND_FadeOutBGM( FIELD_SOUND* fsnd, u16 frames )
{
  fsnd->state        = FS_STATE_PLAY;
  fsnd->fadeOutFrame = frames;
  fsnd->reqFadeOut   = TRUE;
  //FIELD_SOUND_PlayNextBGM_Ex( fsnd, 0, frames, 0 );
}

//--------------------------------------------------------------
/**
 * @brief BGMフェードアウト ==> BGM Push
 * @param fsnd   FIELD_SOUND
 * @param frames フェードアウトに要するフレーム数
 */
//--------------------------------------------------------------
void FIELD_SOUND_FadeOutPushBGM( FIELD_SOUND* fsnd, u16 frames )
{
  FIELD_SOUND_FadeOutBGM( fsnd, frames );
  fsnd->reqPush = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief BGMフェード検出
 * @return BGMがフェード中かどうか(TRUE : フェード中)
 */
//--------------------------------------------------------------
BOOL FIELD_SOUND_IsBGMFade( FIELD_SOUND* fsnd )
{
  if( (fsnd->state == FS_STATE_FADE_IN) || 
      (fsnd->state == FS_STATE_FADE_OUT) )
  { 
    return TRUE;
  }
  return FALSE;
}

#if 0
//======================================================================
//  フィールド BGM トラック関連
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド BGM 自機移動時のトラックに変更
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_ChangeBGMTrackAction( void )
{
	u16 trackBit = TRACKBIT_ALL;
	PMSND_ChangeBGMtrack( trackBit );
}

//--------------------------------------------------------------
/**
 * フィールド BGM 自機停止時のトラックに変更
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_ChangeBGMTrackStill( void )
{
	u16 trackBit = TRACKBIT_STILL;
	PMSND_ChangeBGMtrack( trackBit );
}

//--------------------------------------------------------------
/**
 * フィールド BGM ボリューム変更
 * @param vol BGMボリューム 0-127
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_ChangeBGMVolume( int vol )
{
	PMSND_ChangeBGMVolume( TRACKBIT_ALL, vol );
}

//--------------------------------------------------------------
/**
 * フィールド BGM アクション用トラックボリューム変更
 * @param vol BGMボリューム 0-127
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_ChangeBGMActionVolume( int vol )
{
	PMSND_ChangeBGMVolume( TRACKBIT_ACTION, vol );
}
#endif


//======================================================================
//  フィールド BGM BGMナンバー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド BGM 再生するBGMナンバー取得
 * @param fieldMap FIELDMAP_WORK *
 * @param zone_id ゾーンID
 * @retval u16 フィールドBGMナンバー
 */
//--------------------------------------------------------------
u32 FIELD_SOUND_GetFieldBGMNo(
    GAMEDATA *gdata, PLAYER_MOVE_FORM form, u32 zone_id )
{
  if( form == PLAYER_MOVE_FORM_SWIM ){
    return( SEQ_BGM_NAMINORI );
  }
  
  if( form == PLAYER_MOVE_FORM_CYCLE ){
    return( SEQ_BGM_BICYCLE );
  }
  
  return( fsnd_GetMapBGM(gdata,zone_id) );
}

//======================================================================
//  トレーナー視線BGM
//======================================================================
//--------------------------------------------------------------
/// 視線BGM対応データ
//--------------------------------------------------------------
#include "tr_tool/trtype_def.h"
#include "../../../resource/trtype_bgm/trtype_bgm.cdat"

//--------------------------------------------------------------
/**
 * トレーナー視線曲を再生します。
 * @param trtype トレーナータイプ
 * @retval u32 BGM番号
 */
//--------------------------------------------------------------
u32 FIELD_SOUND_GetTrainerEyeBgmNo( u32 trtype )
{
  int i = 0;
  while( DATA_TrainerTypeToEyeBGMNo[i][0] != TRTYPE_MAX ){
    if( DATA_TrainerTypeToEyeBGMNo[i][0] == trtype ){
      return( DATA_TrainerTypeToEyeBGMNo[i][1]  );
    }
    i++;
  }
  
  OS_Printf( "ERROR:UNKNOW TRAINER TYPE BGM\n" );
  GF_ASSERT( 0 );
  return( SEQ_BGM_EYE_01 );
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * BGMプッシュ回数を取得
 * @param fsnd FIELD_SOUND
 * @retval FSND_PUSHCOUNT
 */
//--------------------------------------------------------------
FSND_PUSHCOUNT FIELD_SOUND_GetBGMPushCount( const FIELD_SOUND *fsnd )
{
  return( fsnd->push_count );
}

//--------------------------------------------------------------
/**
 * 現在のフィールドBGMインデックスを取得
 * @param fieldMap FIELDMAP_WORK
 * @retval u16 BGM Index
 */
//--------------------------------------------------------------
static u32 fsnd_GetMapBGM( GAMEDATA *gdata, u32 zone_id )
{
  u16 idx = ZONEDATA_GetBGMID( zone_id, GAMEDATA_GetSeasonID(gdata) );
  return( idx );
}

//--------------------------------------------------------------
/**
 * BGM退避
 * @param fsnd FIELD_SOUND
 * @param max 超えるとエラーとなる退避数最大
 * @retval nothing
 */
//--------------------------------------------------------------
static void fsnd_PushBGM( FIELD_SOUND *fsnd, int max )
{
  int count = fsnd->push_count + 1;
  
  if( count > max || count >= FSND_PUSHCOUNT_OVER ){
    GF_ASSERT( 0 && "ERROR:FSOUND PUSH COUNT OVER" );
    return;
  }
  
  if( fsnd->state == FS_STATE_FADE_OUT )
  {
    PMSND_PauseBGM( FALSE );
  }
  else
  {
    PMSND_PauseBGM( TRUE );
  } 
  PMSND_PushBGM();
  fsnd->push_count++;
  // 監視状態を停止
  fsnd->state      = FS_STATE_STOP;
  fsnd->nextBGM    = 0;
  fsnd->reqFadeIn  = FALSE;
  fsnd->reqFadeOut = FALSE;
  fsnd->reqPush    = FALSE;
  fsnd->reqPop     = FALSE;

  // DEBUG:
  OBATA_Printf( "FIELD_SOUND: push BGM\n" );
}

//--------------------------------------------------------------
/**
 * BGM復帰
 * @param fsnd FIELD_SOUND
 * @retval nothing
 */
//--------------------------------------------------------------
static void fsnd_PopBGM( FIELD_SOUND *fsnd )
{
  int count = fsnd->push_count - 1;
  
  if( count < 0 ){
    GF_ASSERT( 0 && "ERROR:FSOUND POP COUNT ERROR" );
    return;
  }
  
  PMSND_PopBGM();
  PMSND_PauseBGM( FALSE );
  fsnd->push_count--;
  // 監視状態を復帰
  fsnd->state      = FS_STATE_PLAY;
  fsnd->nextBGM    = PMSND_GetBGMsoundNo();
  fsnd->reqFadeIn  = FALSE;
  fsnd->reqFadeOut = FALSE;
  fsnd->reqPush    = FALSE;
  fsnd->reqPop     = FALSE;

  // DEBUG:
  OBATA_Printf( "FIELD_SOUND: pop BGM\n" );
}
