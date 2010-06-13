///////////////////////////////////////////////////////////////////////////////////
/**
 * @brief フィールドのサウンド制御 ( WB特化機能 )
 * @file   field_sound.c
 * @author obata
 * @date   2010.01.15
 */
///////////////////////////////////////////////////////////////////////////////////
#include "field_sound_system.h"
#include "field_sound.h"
#include "sound/wb_sound_data.sadl"
#include "gamesystem/playerwork.h"
#include "gamesystem/game_data.h"
#include "field/zonedata.h"
#include "tr_tool/trtype_def.h"
#include "../../../resource/trtype_bgm/trtype_bgm.cdat" 
#include "../../../resource/fldmapdata/special_bgm/special_bgm.cdat"


//================================================================================= 
// ■定数
//================================================================================= 
#define SPECIAL_BGM_NONE (0xffffffff)  // 特殊BGMなし


//================================================================================= 
// ■非公開関数
//================================================================================= 
// 自機フォームの取得
static PLAYER_MOVE_FORM GetPlayerMoveForm( GAMEDATA* gameData );
// BGM 番号取得
static u32 GetZoneBGM( GAMEDATA* gameData, u32 zoneID, u8 seasonID );
static u32 GetSpecialBGM( GAMEDATA* gameData, u32 zoneID );
static u32 GetFieldBGM( GAMEDATA* gameData, u32 zoneID, u8 seasonID );


//=================================================================================
// ■BGM 操作
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief イベントワーク
 */
//---------------------------------------------------------------------------------
typedef struct
{
  GAMESYS_WORK* gameSystem;
  FIELD_SOUND*  fieldSound;
  u32             soundIdx;  // 再生するBGM
  u16          fadeInFrame;  // 再生するBGMのフェードイン フレーム数
  u16         fadeOutFrame;  // 再生中のBGMのフェードアウト フレーム数

} FSND_EVWORK;


//---------------------------------------------------------------------------------
/**
 * @brief BGM 即時再生イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT ForcePlayBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_FORCE_PLAY( fieldSound, work->soundIdx );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH; 
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM 即時再生イベント 取得
 *
 * @param gameSystem
 * @param soundIdx 再生する BGM
 * 
 * @return BGM 即時再生イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_ForcePlayBGM( GAMESYS_WORK* gameSystem, u16 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, ForcePlayBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM フェードイン イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT FadeInBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_FADE_IN( fieldSound, work->fadeInFrame );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM フェードイン イベント 取得
 *
 * @param gameSystem
 * @param fadeInFrame フェードイン時間
 * 
 * @return BGM フェードイン イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_FadeInBGM( GAMESYS_WORK* gameSystem, u16 fadeInFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, FadeInBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->fadeInFrame = fadeInFrame;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM フェードアウト処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT FadeOutBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_FADE_OUT( fieldSound, work->fadeOutFrame );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM フェードアウト イベント 取得
 *
 * @param gameSystem
 * @param fadeOutFrame フェードアウト時間
 * 
 * @return BGM フェードアウト イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_FadeOutBGM( GAMESYS_WORK* gameSystem, u16 fadeOutFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, FadeOutBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->fadeOutFrame = fadeOutFrame;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM 退避イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PushBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_PUSH( fieldSound, work->fadeOutFrame );
    (*seq)++;
    break;
  case 1:  // 退避完了待ち
    if( FIELD_SOUND_HaveRequest(fieldSound) == FALSE ){ (*seq)++; }
    break;
  case 2:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM 退避イベント 取得
 *
 * @param gameSystem
 * @param fadeOutFrame フェードアウト時間
 * 
 * @return BGM 退避イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_PushBGM( GAMESYS_WORK* gameSystem, u16 fadeOutFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PushBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->fadeOutFrame = fadeOutFrame;
  return event;
} 

//---------------------------------------------------------------------------------
/**
 * @brief BGM復帰イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PopBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_POP( fieldSound, work->fadeOutFrame, work->fadeInFrame );
    (*seq)++;
    break;
  case 1:  // BGM復帰完了待ち
    if( FIELD_SOUND_HaveRequest(fieldSound) == FALSE ){ (*seq)++; }
    break;
  case 2: 
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM 復帰イベント 取得
 *
 * @param gameSystem
 * @param fadeOutFrame 再生中のBGMのフェードアウト フレーム数
 * @param fadeInFrame  復帰したBGMのフェードイン フレーム数
 * 
 * @return BGM 復帰イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_PopBGM( GAMESYS_WORK* gameSystem, u16 fadeOutFrame, u16 fadeInFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PopBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->fadeInFrame = fadeInFrame;
  work->fadeOutFrame = fadeOutFrame;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM 復帰イベント ( 復帰待ちなし )
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PopPlayBGMNoWaitEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq ) {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_POP( fieldSound, work->fadeOutFrame, work->fadeInFrame );
    (*seq)++;
    break;
  case 1: 
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 

//---------------------------------------------------------------------------------
/**
 * @brief BGM 復帰イベント ( 復帰待ちなし ) 取得
 *
 * @param gameSystem
 * @param fadeOutFrame 再生中のBGMのフェードアウト フレーム数
 * @param fadeInFrame  復帰したBGMのフェードイン フレーム数
 * 
 * @return BGM 復帰イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_PopBGMNoWait( GAMESYS_WORK* gameSystem, u16 fadeOutFrame, u16 fadeInFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PopPlayBGMNoWaitEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->fadeInFrame = fadeInFrame;
  work->fadeOutFrame = fadeOutFrame;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM 全復帰イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT AllPopBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work = (FSND_EVWORK*)wk;
  FIELD_SOUND* fieldSound = work->fieldSound;
  FSND_PUSHCOUNT pushCount;
  int i; 

  switch( *seq )
  {
  // 全ポップ
  case 0:
    // ポップされる予定のないBGMの数を取得
    pushCount = FIELD_SOUND_GetBGMPushCount_atAllRequestFinished( fieldSound );

    // 全BGMをポップ
    if( FSND_PUSHCOUNT_NONE < pushCount )
    {
      for( i=0; i<pushCount-1; i++ )
      { // フェードなしでポップ
        FIELD_SOUND_RegisterRequest_POP( fieldSound, FSND_FADE_NONE, FSND_FADE_NONE );
      }
      // フェードありでポップ
      FIELD_SOUND_RegisterRequest_POP( fieldSound, FSND_FADE_NONE, work->fadeInFrame );
    }
    (*seq)++;
    break;

  // イベント終了
  case 1:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE; 
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM 全復帰イベント 取得
 *
 * @param gameSystem
 * @param fadeInFrame フェードイン時間
 * 
 * @return BGM 全復帰イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_AllPopBGM( GAMESYS_WORK* gameSystem, u16 fadeInFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, AllPopBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->fadeInFrame = fadeInFrame;
  return event;
} 

//---------------------------------------------------------------------------------
/**
 * @brief BGM 退避・再生イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PushPlayNextBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_PUSH( fieldSound, work->fadeOutFrame );
    FIELD_SOUND_RegisterRequest_CHANGE( fieldSound, work->soundIdx, 
                                        FSND_FADE_NONE, work->fadeInFrame );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}
//---------------------------------------------------------------------------------
/**
 * @brief BGM退避・再生イベント取得
 *
 * @param gameSystem
 * @param soundIdx     再生するBGM
 * @param fadeOutFrame 再生中のBGMのフェードアウト フレーム数
 * @param fadeInFrame  復帰したBGMのフェードイン フレーム数
 * 
 * @return 退避・再生イベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_PushPlayNextBGM( GAMESYS_WORK* gameSystem, u32 soundIdx,
                                     u16 fadeOutFrame, u16 fadeInFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PushPlayNextBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;
  work->fadeInFrame = fadeInFrame;
  work->fadeOutFrame = fadeOutFrame;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief ME 再生イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PushPlayJingleBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_PUSH( fieldSound, FSND_FADE_FAST );
    FIELD_SOUND_RegisterRequest_FORCE_PLAY( fieldSound, work->soundIdx );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief ME 再生イベント 取得
 *
 * @param gameSystem
 * @param soundIdx 再生する ME
 * 
 * @return ME 再生イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_PushPlayJingleBGM( GAMESYS_WORK* gameSystem, u32 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PushPlayJingleBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;
  work->fadeOutFrame = FSND_FADE_NONE;
  return event;
} 

//---------------------------------------------------------------------------------
/**
 * @brief BGM 変更イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT ChangeBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq )
  {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_CHANGE( fieldSound, work->soundIdx, 
                                        work->fadeOutFrame, work->fadeInFrame );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}
//---------------------------------------------------------------------------------
/**
 * @brief BGM 変更イベント取得
 *
 * @param gameSystem
 * @param soundIdx 再生する ME
 * @param fadeOutFrame 再生中のBGMのフェードアウト フレーム数
 * @param fadeInFrame  復帰したBGMのフェードイン フレーム数
 * 
 * @return 退避・再生イベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_ChangeBGM( GAMESYS_WORK* gameSystem, u32 soundIdx,
                               u16 fadeOutFrame, u16 fadeInFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, ChangeBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;
  work->fadeInFrame = fadeInFrame;
  work->fadeOutFrame = fadeOutFrame;
  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief BGM リセット イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT ResetBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work       = (FSND_EVWORK*)wk;
  GAMEDATA*    gameData   = GAMESYSTEM_GetGameData( work->gameSystem );
  FIELD_SOUND* fieldSound = work->fieldSound;

  switch( *seq ) {
  // フェードアウトリクエスト発行
  case 0:
    FIELD_SOUND_RegisterRequest_FADE_OUT( fieldSound, work->fadeOutFrame );
    (*seq)++;
    break;

  // 全リクエストの消化待ち
  case 1:
    if( FIELD_SOUND_HaveRequest( fieldSound ) == FALSE ) { (*seq)++; }
    break;

  // FIELD_SOUNDをリセット
  case 2:
    FIELD_SOUND_Reset( fieldSound, gameData );
    (*seq)++;
    break;

  // イベント終了
  case 3:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}

//---------------------------------------------------------------------------------
/**
 * @brief イベントBGM再生イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PlayEventBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq ) {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_FADE_OUT( fieldSound, FSND_FADE_FAST );
    FIELD_SOUND_RegisterRequest_FORCE_PLAY( fieldSound, work->soundIdx );
    (*seq)++;
    break;
  case 1:
    // 即時再生の完了待ち
    if( FIELD_SOUND_HaveRequest( fieldSound ) == FALSE ) {
      return GMEVENT_RES_FINISH;
    }
  } 
  return GMEVENT_RES_CONTINUE;
} 
//--------------------------------------------------------------------------------- 
/**
 * @brief イベントBGMの再生イベントを生成する
 *
 * @param gameSystem
 * @param soundIdx   再生するBGM
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_PlayEventBGM( GAMESYS_WORK* gameSystem, u32 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  // イベントを生成
  event = GMEVENT_Create( gameSystem, NULL, PlayEventBGMEvent, sizeof(FSND_EVWORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;

  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief イベントBGM再生イベント処理関数 ( FOフレーム数指定ver. )
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PlayEventBGMExEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq ) {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_FADE_OUT( fieldSound, work->fadeOutFrame );
    FIELD_SOUND_RegisterRequest_FORCE_PLAY( fieldSound, work->soundIdx );
    (*seq)++;
    break;
  case 1:
    // 即時再生の完了待ち
    if( FIELD_SOUND_HaveRequest( fieldSound ) == FALSE ) {
      return GMEVENT_RES_FINISH;
    }
  } 
  return GMEVENT_RES_CONTINUE;
} 
//--------------------------------------------------------------------------------- 
/**
 * @brief イベントBGMの再生イベントを生成する ( FOフレーム数指定ver. )
 *
 * @param gameSystem
 * @param soundIdx     再生するBGM
 * @param fadeOutFrame フェードアウトフレーム数
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_PlayEventBGMEx( GAMESYS_WORK* gameSystem, u32 soundIdx, u32 fadeOutFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  // イベントを生成
  event = GMEVENT_Create( gameSystem, NULL, PlayEventBGMExEvent, sizeof(FSND_EVWORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->fieldSound   = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx     = soundIdx;
	work->fadeOutFrame = fadeOutFrame;

  return event;
} 
//---------------------------------------------------------------------------------
/**
 * @brief 無音 BGM 再生イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PlayEventBGMSilentEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq ) {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_FADE_OUT( fieldSound, work->fadeOutFrame );
    FIELD_SOUND_RegisterRequest_FORCE_PLAY( fieldSound, SEQ_BGM_SILENCE_FIELD );
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 
//--------------------------------------------------------------------------------- 
/**
 * @brief 無音 BGM の再生イベントを生成する
 *
 * @param gameSystem
 * @param fadeOutFrame フェードアウトフレーム数
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_PlayEventBGMSilent( GAMESYS_WORK* gameSystem, u32 fadeOutFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  // イベントを生成
  event = GMEVENT_Create( gameSystem, NULL, PlayEventBGMSilentEvent, sizeof(FSND_EVWORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->fieldSound   = GAMEDATA_GetFieldSound( gdata );
	work->fadeOutFrame = fadeOutFrame;

  return event;
}
//---------------------------------------------------------------------------------
/**
 * @brief イベントBGM再生イベント処理関数 ( フィールド BGM 退避 ver. )
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PushPlayEventBGM( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq ) {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_PUSH( fieldSound, FSND_FADE_FAST );
    FIELD_SOUND_RegisterRequest_FORCE_PLAY( fieldSound, work->soundIdx );
    (*seq)++;
    break;
  case 1: // リクエスト処理待ち
    if( FIELD_SOUND_HaveRequest( fieldSound ) == FALSE ) {
      return GMEVENT_RES_FINISH;
    }
  } 
  return GMEVENT_RES_CONTINUE;
} 
//--------------------------------------------------------------------------------- 
/**
 * @brief イベントBGMの再生イベントを生成する ( フィールド BGM 退避 ver. )
 *
 * @param gameSystem
 * @param soundIdx   再生するBGM
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_PushPlayEventBGM( GAMESYS_WORK* gameSystem, u32 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  // イベントを生成
  event = GMEVENT_Create( gameSystem, NULL, PushPlayEventBGM, sizeof(FSND_EVWORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;

  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief 視線曲再生イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PlayTrainerEyeBGM( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq ) {
  case 0:  // リクエスト発行
    FIELD_SOUND_RegisterRequest_FADE_OUT( fieldSound, FSND_FADE_FAST );
    FIELD_SOUND_RegisterRequest_FORCE_PLAY( fieldSound, work->soundIdx );
    (*seq)++;
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
} 
//--------------------------------------------------------------------------------- 
/**
 * @brief 視線曲再生イベントを生成する
 *
 * @param gameSystem
 * @param soundIdx   再生するBGM
 *
 * @return 生成したイベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_PlayTrainerEyeBGM( GAMESYS_WORK* gameSystem, u32 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  // イベントを生成
  event = GMEVENT_Create( gameSystem, NULL, PlayTrainerEyeBGM, sizeof(FSND_EVWORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;

  return event;
}

//--------------------------------------------------------------------------------- 
/**
 * @brief BGMを停止し, 退避中の全てのBGMを破棄する
 *
 * @param gameSystem
 * @param fadeOutFrame 再生中のBGMのフェードアウト フレーム数
 *
 * @return BGMリセットイベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_ResetBGM( GAMESYS_WORK* gameSystem, u16 fadeOutFrame )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  // イベントを生成
  event = GMEVENT_Create( gameSystem, NULL, ResetBGMEvent, sizeof(FSND_EVWORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->gameSystem   = gameSystem;
  work->fieldSound   = GAMEDATA_GetFieldSound( gdata );
  work->fadeOutFrame = fadeOutFrame;

  return event;
}

//---------------------------------------------------------------------------------
/**
 * @brief ベースBGM 退避イベント処理関数
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PushPlayBattleBGMEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq ) {
  // リクエスト発行
  case 0:  
    if( FIELD_SOUND_GetBGMPushCount_atAllRequestFinished(fieldSound) == FSND_PUSHCOUNT_NONE ) { 
      // ベースBGM再生中
      FIELD_SOUND_RegisterRequest_PUSH( fieldSound, FSND_FADE_FAST );
    }
    else { 
      // 視線曲再生中 ( ベースBGMはすでに退避済み )
      FIELD_SOUND_RegisterRequest_FADE_OUT( fieldSound, FSND_FADE_FAST );
    }
    (*seq)++;
    break;

  // BGM退避 or フェードアウト完了待ち
  case 1:  
    if( FIELD_SOUND_HaveRequest(fieldSound) == FALSE ){ (*seq)++; }
    break;

  // バトルBGM再生
  case 2:  
    FIELD_SOUND_RegisterRequest_FORCE_PLAY( fieldSound, work->soundIdx );
    (*seq)++;
    break;

  // イベント終了
  case 3:
    return GMEVENT_RES_FINISH;
  }
  return GMEVENT_RES_CONTINUE;
} 
//---------------------------------------------------------------------------------
/**
 * @brief ベースBGMの退避, 戦闘BGM再生イベント取得
 *
 * @param gameSystem
 * @param soundIdx   再生する戦闘BGM
 * 
 * @return BGM 退避イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_PushPlayBattleBGM( GAMESYS_WORK* gameSystem, u32 soundIdx )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, PushPlayBattleBGMEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  work->soundIdx = soundIdx;
  return event;
} 
//---------------------------------------------------------------------------------
/**
 * @brief 戦闘曲からベースBGMに復帰するイベントを生成する
 *
 * @param gameSystem
 *
 * @return 戦闘曲からのBGM復帰イベント
 */
//---------------------------------------------------------------------------------
GMEVENT* EVENT_FSND_PopPlayBGM_fromBattle( GAMESYS_WORK* gameSystem )
{
  FSND_EVWORK* work;
  GAMEDATA*    gameData;
  GMEVENT*     event;

  gameData = GAMESYSTEM_GetGameData( gameSystem );

  // イベントを生成
  event = GMEVENT_Create( gameSystem, NULL, PopPlayBGMNoWaitEvent, sizeof(FSND_EVWORK) );

  // イベントワークを初期化
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gameData );
  work->fadeInFrame = FSND_FADE_NORMAL;
  work->fadeOutFrame = FSND_FADE_SHORT;
  return event;
} 
//---------------------------------------------------------------------------------
/**
 * @brief BGM フェード完了待ちイベント
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT WaitBGMFadeEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq ) {
  case 0:  // フェード完了待ち
    if( FIELD_SOUND_IsBGMFade(fieldSound) == FALSE ){ (*seq)++; }
    break;
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}
//--------------------------------------------------------------------------------- 
/**
 * @brief BGMフェード完了待ちイベント取得
 *
 * @param gameSystem
 *
 * @return BGMフェード完了待ちイベント
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_WaitBGMFade( GAMESYS_WORK* gameSystem )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, WaitBGMFadeEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  return event;
} 

//---------------------------------------------------------------------------------
/**
 * @brief BGM復帰待ちイベント
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT WaitBGMPopEvent( GMEVENT* event, int* seq, void* wk )
{
  FSND_EVWORK* work;
  FIELD_SOUND* fieldSound;

  work = (FSND_EVWORK*)wk;
  fieldSound = work->fieldSound;

  switch( *seq ) {
  // BGMのPOP完了待ち
  case 0:
    if( FIELD_SOUND_CheckBGMPopFinished(fieldSound) == TRUE ) { (*seq)++; }
    break;

  // イベント終了
  case 1:
    return GMEVENT_RES_FINISH;
  } 
  return GMEVENT_RES_CONTINUE;
}
//--------------------------------------------------------------------------------- 
/**
 * @brief BGM復帰待ちイベントを生成する
 *
 * @param gameSystem
 */
//--------------------------------------------------------------------------------- 
GMEVENT* EVENT_FSND_WaitBGMPop( GAMESYS_WORK* gameSystem )
{
  GMEVENT* event;
  FSND_EVWORK* work;
  GAMEDATA* gdata;

  gdata = GAMESYSTEM_GetGameData( gameSystem );

  event = GMEVENT_Create( gameSystem, NULL, WaitBGMPopEvent, sizeof(FSND_EVWORK) );
  work = GMEVENT_GetEventWork( event );
  work->fieldSound = GAMEDATA_GetFieldSound( gdata );
  return event;
}

//=================================================================================
// ■プレイヤーの操作により発生するBGMの変更
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief ゾーン チェンジによるBGMの変更
 *
 * @param fieldSound
 * @param gameData
 * @param nextZoneID チェンジ後のゾーンID
 */
//---------------------------------------------------------------------------------
void FSND_ChangeBGM_byZoneChange( FIELD_SOUND* fieldSound, GAMEDATA* gameData, u16 nextZoneID )
{
  u8 seasonID;
  u16 fadeOutFrame, fadeInFrame;
  u32 soundIdx;

  // ゾーンチェンジでは, 季節は変化しない
  seasonID = GAMEDATA_GetSeasonID( gameData );

  // チェンジ先のゾーンのBGMを取得
  soundIdx = GetSpecialBGM( gameData, nextZoneID );
  if( soundIdx == SPECIAL_BGM_NONE ) {
    soundIdx = GetZoneBGM( gameData, nextZoneID, seasonID ); 
  }

  // フェード フレーム数を決定
  if( GetPlayerMoveForm(gameData) == PLAYER_MOVE_FORM_CYCLE ) { // 自転車
    fadeOutFrame = FSND_FADE_NORMAL;
    fadeInFrame  = FSND_FADE_NORMAL;
  }
  else { // 歩き
    fadeOutFrame = FSND_FADE_NORMAL;
    fadeInFrame  = FSND_FADE_NONE;
  }

  // リクエスト登録
  FIELD_SOUND_RegisterRequest_CHANGE( fieldSound, soundIdx, fadeOutFrame, fadeInFrame );
}

//---------------------------------------------------------------------------------
/**
 * @brief マップ チェンジ時のBGM再生準備
 *
 * @param fieldSound
 * @param gameData
 * @param nextZoneID   遷移後のゾーンID
 * @param nextSeasonID 遷移後の季節ID
 */
//---------------------------------------------------------------------------------
void FSND_StandByNextMapBGM( 
    FIELD_SOUND* fieldSound, GAMEDATA* gameData, u16 nextZoneID, u8 nextSeasonID )
{
  u32 nextBGM;

  nextBGM = GetSpecialBGM( gameData, nextZoneID );
  if( nextBGM == SPECIAL_BGM_NONE ) { 
    nextBGM = GetZoneBGM( gameData, nextZoneID, nextSeasonID ); 
  }

  FIELD_SOUND_RegisterRequest_STAND_BY( fieldSound, nextBGM, FSND_FADE_NORMAL );
}

//---------------------------------------------------------------------------------
/**
 * @brief マップ チェンジ時のBGM再生開始
 *
 * @param fieldSound
 * @param gameData
 */
//---------------------------------------------------------------------------------
void FSND_PlayStartBGM( FIELD_SOUND* fieldSound, GAMEDATA* gameData )
{
  // リクエスト登録
  FIELD_SOUND_RegisterRequest_FADE_IN( fieldSound, FSND_FADE_NONE );
}

//---------------------------------------------------------------------------------
/**
 * @brief 自機移動フォームの変更によるBGM変更
 *
 * @param fieldSound
 * @param gameData
 * @param zoneID      現在のゾーンID
 */
//---------------------------------------------------------------------------------
void FSND_ChangeBGM_byPlayerFormChange( FIELD_SOUND* fieldSound, GAMEDATA* gameData, u16 zoneID )
{
  u8 seasonID;
  u32 soundIdx;

  soundIdx = GetFieldBGM( gameData, zoneID, GAMEDATA_GetSeasonID(gameData) );

  // リクエスト登録
  FIELD_SOUND_RegisterRequest_CHANGE( fieldSound, soundIdx, FSND_FADE_SHORT, FSND_FADE_NONE );
}


//=================================================================================
// ■BGMボリューム操作
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief フィールド → アプリ 遷移時の BGMボリューム抑制処理
 *
 * @param fieldSound
 * @parma iss
 */
//---------------------------------------------------------------------------------
void FSND_HoldBGMVolume_forApp( FIELD_SOUND* fieldSound, ISS_SYS* iss )
{
  ISS_3DS_SYS* iss3DSSystem;

  iss3DSSystem = ISS_SYS_GetIss3DSSystem( iss );

  // 橋ISSを無効化
  ISS_3DS_SYS_SetMasterVolume( iss3DSSystem, 0 );

  // プレイヤーボリュームをフェードアウト
  FIELD_SOUND_ChangePlayerVolume( fieldSound, APP_HOLD_VOLUME, APP_FADE_FRAME );  
}

//---------------------------------------------------------------------------------
/**
 * @brief フィールド ← アプリ 遷移時の BGMボリューム解放処理
 *
 * @param fieldSound
 * @parma iss
 */
//---------------------------------------------------------------------------------
void FSND_ReleaseBGMVolume_fromApp( FIELD_SOUND* fieldSound, ISS_SYS* iss )
{
  ISS_3DS_SYS* iss3DSSystem;

  iss3DSSystem = ISS_SYS_GetIss3DSSystem( iss );

  // 橋ISSを無効化
  ISS_3DS_SYS_SetMasterVolume( iss3DSSystem, MAX_VOLUME );

  // プレイヤーボリュームをフェードイン
  FIELD_SOUND_ChangePlayerVolume( fieldSound, MAX_VOLUME, APP_FADE_FRAME );
}


//=================================================================================
// ■BGM No.取得
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief 指定したゾーンでの再生すべきBGMを取得する
 *
 * @param gameData
 * @param zoneID   ゾーンID
 * @param seasonID 季節ID
 *
 * @return 指定したゾーンで再生すべきBGM No.
 */
//---------------------------------------------------------------------------------
u32 FSND_GetFieldBGM( GAMEDATA* gameData, u32 zoneID, u8 seasonID )
{
  return GetFieldBGM( gameData, zoneID, seasonID );
}

//---------------------------------------------------------------------------------
/**
 * @brief 最後のBGMリクエスト処理が完了した時点での BGM No.
 *
 * @param gameData
 */
//---------------------------------------------------------------------------------
u32 FSND_GetLastBGM( GAMEDATA* gameData )
{
  FIELD_SOUND* fieldSound = GAMEDATA_GetFieldSound( gameData );
  return FIELD_SOUND_GetPlayingBGM_atAllRequestFinished( fieldSound );
}

//---------------------------------------------------------------------------------
/**
 * @brief 特殊 BGM があるかどうか
 *
 * @param gameData
 * @param zoneID   ゾーンID
 *
 * @return 指定したゾーンに, 有効な特殊BGMがある場合 TRUE
 */
//---------------------------------------------------------------------------------
BOOL FSND_CheckValidSpecialBGM( GAMEDATA* gameData, u16 zoneID )
{
  if( GetSpecialBGM( gameData, zoneID ) == SPECIAL_BGM_NONE ) {
    return FALSE;
  }
  return TRUE;
}

//---------------------------------------------------------------------------------
/**
 * @brief トレーナー視線曲 取得
 *
 * @param trType トレーナータイプ
 *
 * @return 指定したトレーナータイプの視線曲
 */
//---------------------------------------------------------------------------------
u32 FSND_GetTrainerEyeBGM( u32 trType )
{
  int i = 0;
  while( DATA_TrainerTypeToEyeBGMNo[i][0] != TRTYPE_MAX )
  {
    if( DATA_TrainerTypeToEyeBGMNo[i][0] == trType )
    {
      return( DATA_TrainerTypeToEyeBGMNo[i][1]  );
    }
    i++;
  }
  
  GF_ASSERT(0 && "FIELD-SOUND: error unknown trainer type\n");
  return SEQ_BGM_EYE_01;
} 


//=================================================================================
// ■非公開関数
//=================================================================================

//---------------------------------------------------------------------------------
/**
 * @brief 自機のフォームを取得する
 *
 * @param gameData
 *
 * @return 自機のフォーム
 */
//---------------------------------------------------------------------------------
static PLAYER_MOVE_FORM GetPlayerMoveForm( GAMEDATA* gameData )
{
  PLAYER_WORK* player;
  PLAYER_MOVE_FORM playerForm;

  player = GAMEDATA_GetMyPlayerWork( gameData );
  playerForm = PLAYERWORK_GetMoveForm( player );

  return playerForm;
}

//---------------------------------------------------------------------------------
/**
 * @brief 指定ゾーンのBGMを取得
 *
 * @param gameData
 * @param zoneID   ゾーンID
 * @param seasonID 季節ID
 *
 * @return ゾーン×季節 で決定する フィールドの BGM
 */
//---------------------------------------------------------------------------------
static u32 GetZoneBGM( GAMEDATA* gameData, u32 zoneID, u8 seasonID )
{
  u32 soundIdx;

  soundIdx = ZONEDATA_GetBGMID( zoneID, seasonID );

  return soundIdx;
}

//---------------------------------------------------------------------------------
/**
 * @brief 特殊BGMを取得する
 *
 * @param gameData
 * @param zoneID   特殊BGMを検索するゾーン
 *
 * @return 条件が一致する特殊BGMが存在する場合, そのBGM No.
 *         特殊BGMが存在しない場合, SPECIAL_BGM_NONE
 */
//---------------------------------------------------------------------------------
static u32 GetSpecialBGM( GAMEDATA* gameData, u32 zoneID )
{
  int i;
  EVENTWORK* eventWork;

  eventWork = GAMEDATA_GetEventWork( gameData );

  // 検索
  for( i=0; i<NELEMS(specialBGMTable); i++)
  {
    // 発見
    if( (specialBGMTable[i].zoneID == zoneID) &&
        (EVENTWORK_CheckEventFlag(eventWork, specialBGMTable[i].flagNo) == TRUE) )
    {
      return specialBGMTable[i].soundIdx;
    }
  }

  // 一致なし
  return SPECIAL_BGM_NONE;
}

//---------------------------------------------------------------------------------
/**
 * @brief 指定ゾーンにおける, 再生すべきBGMを取得する
 *
 * @param gameData
 * @param zoneID
 * @param seasonID
 *
 * @return 指定したゾーンで再生すべきBGM
 */
//---------------------------------------------------------------------------------
static u32 GetFieldBGM( GAMEDATA* gameData, u32 zoneID, u8 seasonID )
{
  u32 soundIdx;
  PLAYER_MOVE_FORM playerForm;
  FIELD_SOUND* fieldSound;

  playerForm = GetPlayerMoveForm( gameData );
  fieldSound = GAMEDATA_GetFieldSound( gameData );

  // 特殊BGMを取得
  soundIdx = GetSpecialBGM( gameData, zoneID );

  // 特殊BGMが無い
  if( soundIdx == SPECIAL_BGM_NONE ) {
    // なみのり
    if( (playerForm == PLAYER_MOVE_FORM_SWIM) && ZONEDATA_BicycleBGMEnable( zoneID ) ) { 
      soundIdx = SEQ_BGM_NAMINORI;
    }      
    // 自転車
    else if( (playerForm == PLAYER_MOVE_FORM_CYCLE) && ZONEDATA_BicycleBGMEnable( zoneID ) ) { 
      soundIdx = SEQ_BGM_BICYCLE; 
    } 
    // 歩き
    else { 
      soundIdx = GetZoneBGM( gameData, zoneID, seasonID ); 
    }                             
  } 

  return soundIdx;
}
